#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <array>
#include <time.h>
#include "SDPolicies.h"
#include "SDCard.hpp"

//#undef _WIN32

#include "FatLib/source/ff.h"
#include "FatLib/source/diskio.h"

SPIDriver spiTester;
std::string SpiDriverPort;

DSTATUS stat = STA_NOINIT;

sd::SpiCard<SPIShim> sdcard;

int test_diskio (
        BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
        UINT ncyc,      /* Number of test cycles */
        DWORD* buff,    /* Pointer to the working buffer */
        UINT sz_buff    /* Size of the working buffer in unit of byte */
);

/* Pseudo random number generator */
/* 0:Initialize, !0:Read */
static DWORD pn ( DWORD pns);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: spicl <PORTNAME>\n");
        exit(1);
    }
    SpiDriverPort = std::string(argv[1]);

    int rc;
    DWORD buff[FF_MAX_SS];  /* Working buffer (4 sector in size) */

    /* Check function/compatibility of the physical drive #0 */
    rc = test_diskio(0, 3, buff, sizeof buff);

    if (rc) {
        printf("Sorry the function/compatibility test failed. (rc=%d)\nFatFs will not work with this disk driver.\n", rc);
    } else {
        printf("Congratulations! The disk driver works well.\n");
    }

    return 0;
}

DSTATUS disk_status ( BYTE pdrv ) {
    (void)pdrv;
    return stat;
}

DSTATUS disk_initialize ( BYTE pdrv ) {
    (void)pdrv;
    if(!sdcard.begin()) {
        stat = STA_NODISK;
    }
    else {
        stat = 0;
    }
    return stat;
}

DRESULT disk_read (
        BYTE pdrv,     /* [IN] Physical drive number */
        BYTE* buff,    /* [OUT] Pointer to the read data buffer */
        DWORD sector,  /* [IN] Start sector number */
        UINT count     /* [IN] Number of sectros to read */
)
{
    (void)pdrv;
    DRESULT retval = RES_OK;
    for(UINT i = 0; i < count; ++i) {
        if(1 != sdcard.readBlock(sector, buff) ){
            retval = RES_ERROR;
        }
        sector++;
        buff += 512;
    }
    return retval;
}

DRESULT disk_write (
        BYTE pdrv,        /* [IN] Physical drive number */
        const BYTE* buff, /* [IN] Pointer to the data to be written */
        DWORD sector,     /* [IN] Sector number to write from */
        UINT count        /* [IN] Number of sectors to write */
)
{
    (void)pdrv;
    DRESULT retval = RES_OK;
    for(UINT i = 0; i < count; ++i) {
        if(1 != sdcard.writeBlock(sector, buff) ){
            retval = RES_ERROR;
        }
        sector++;
        buff += 512;
    }
    return retval;
}

DRESULT disk_ioctl (
        BYTE pdrv,     /* [IN] Drive number */
        BYTE cmd,      /* [IN] Control command code */
        void* buff     /* [I/O] Parameter and data buffer */
)
{
    (void)pdrv;
    switch(cmd) {
        case CTRL_SYNC :
            break;
        case GET_SECTOR_COUNT :
            *((DWORD*)buff) = sdcard.cardCapacity();
            break;
        case GET_SECTOR_SIZE :
        case GET_BLOCK_SIZE :
            *((DWORD*)buff) = 512;
            break;
        case CTRL_TRIM :
            break;
        default: break;
    }
    return RES_OK;
}

DWORD get_fattime (void) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    DWORD fatTime = 0;
    fatTime |= ( (timeinfo->tm_year - 80)<<25 );
    fatTime |= ( (timeinfo->tm_mon + 1)<<21 );
    fatTime |= ( (timeinfo->tm_mday)<<16 );
    fatTime |= ( (timeinfo->tm_hour)<<11 );
    fatTime |= ( (timeinfo->tm_min)<<5 );
    fatTime |= ( (timeinfo->tm_sec / 2)<<0 );

    return fatTime;
}

/* Pseudo random number generator */
static DWORD pn (
        DWORD pns	/* 0:Initialize, !0:Read */
)
{
    static DWORD lfsr;
    UINT n;


    if (pns) {
        lfsr = pns;
        for (n = 0; n < 32; n++) pn(0);
    }
    if (lfsr & 1) {
        lfsr >>= 1;
        lfsr ^= 0x80200003;
    } else {
        lfsr >>= 1;
    }
    return lfsr;
}


int test_diskio (
        BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
        UINT ncyc,      /* Number of test cycles */
        DWORD* buff,    /* Pointer to the working buffer */
        UINT sz_buff    /* Size of the working buffer in unit of byte */
)
{
    UINT n, cc, ns;
    DWORD sz_drv, lba, lba2, sz_eblk, pns = 1;
    WORD sz_sect;
    BYTE *pbuff = (BYTE*)buff;
    DSTATUS ds;
    DRESULT dr;


    printf("test_diskio(%u, %u, 0x%08X, 0x%08X)\n", pdrv, ncyc, (uint64_t)buff, sz_buff);

    if (sz_buff < FF_MAX_SS + 4) {
        printf("Insufficient work area to run program.\n");
        return 1;
    }

    for (cc = 1; cc <= ncyc; cc++) {
        printf("**** Test cycle %u of %u start ****\n", cc, ncyc);

        printf(" disk_initalize(%u)", pdrv);
        ds = disk_initialize(pdrv);
        if (ds & STA_NOINIT) {
            printf(" - failed.\n");
            return 2;
        } else {
            printf(" - ok.\n");
        }

        printf("**** Get drive size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_COUNT, 0x%08X)", pdrv, (uint64_t)&sz_drv);
        sz_drv = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 3;
        }
        if (sz_drv < 128) {
            printf("Failed: Insufficient drive size to test.\n");
            return 4;
        }
        printf(" Number of sectors on the drive %u is %lu.\n", pdrv, sz_drv);

#if FF_MAX_SS != FF_MIN_SS
        printf("**** Get sector size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_SIZE, 0x%X)", pdrv, (uint64_t)&sz_sect);
        sz_sect = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_SIZE, &sz_sect);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 5;
        }
        printf(" Size of sector is %u bytes.\n", sz_sect);
#else
        sz_sect = FF_MAX_SS;
#endif

        printf("**** Get block size ****\n");
        printf(" disk_ioctl(%u, GET_BLOCK_SIZE, 0x%X)", pdrv, (uint64_t)&sz_eblk);
        sz_eblk = 0;
        dr = disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_eblk);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
        }
        if (dr == RES_OK || sz_eblk >= 2) {
            printf(" Size of the erase block is %lu sectors.\n", sz_eblk);
        } else {
            printf(" Size of the erase block is unknown.\n");
        }

        /* Single sector write test */
        printf("**** Single sector write test 1 ****\n");
        lba = 0;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)pbuff, lba);
        dr = disk_write(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 6;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 7;
        }
        memset(pbuff, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)pbuff, lba);
        dr = disk_read(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 8;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Data matched.\n");
        } else {
            printf("Failed: Read data differs from the data written.\n");
            return 10;
        }
        pns++;

        printf("**** Multiple sector write test ****\n");
        lba = 1; ns = sz_buff / sz_sect;
        if (ns > 4) ns = 4;
        for (n = 0, pn(pns); n < (uint64_t)(sz_sect * ns); n++) pbuff[n] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, %u)", pdrv, (uint64_t)pbuff, lba, ns);
        dr = disk_write(pdrv, pbuff, lba, ns);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 11;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 12;
        }
        memset(pbuff, 0, sz_sect * ns);
        printf(" disk_read(%u, 0x%X, %lu, %u)", pdrv, (uint64_t)pbuff, lba, ns);
        dr = disk_read(pdrv, pbuff, lba, ns);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 13;
        }
        for (n = 0, pn(pns); n < (uint64_t)(sz_sect * ns) && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == (uint64_t)(sz_sect * ns)) {
            printf(" Data matched.\n");
        } else {
            printf("Failed: Read data differs from the data written.\n");
            return 14;
        }
        pns++;

        printf("**** Single sector write test (misaligned address) ****\n");
        lba = 5;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n+3] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)(pbuff+3), lba);
        dr = disk_write(pdrv, pbuff+3, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 15;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 16;
        }
        memset(pbuff+5, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)(pbuff+5), lba);
        dr = disk_read(pdrv, pbuff+5, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            printf(" - failed.\n");
            return 17;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n+5] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Data matched.\n");
        } else {
            printf("Failed: Read data differs from the data written.\n");
            return 18;
        }
        pns++;

        printf("**** 4GB barrier test ****\n");
        if (sz_drv >= 128 + 0x80000000 / (sz_sect / 2)) {
            lba = 6; lba2 = lba + 0x80000000 / (sz_sect / 2);
            for (n = 0, pn(pns); n < (uint64_t)(sz_sect * 2); n++) pbuff[n] = (BYTE)pn(0);
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)pbuff, lba);
            dr = disk_write(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 19;
            }
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)(pbuff+sz_sect), lba2);
            dr = disk_write(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 20;
            }
            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 21;
            }
            memset(pbuff, 0, sz_sect * 2);
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)pbuff, lba);
            dr = disk_read(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 22;
            }
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uint64_t)(pbuff+sz_sect), lba2);
            dr = disk_read(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                printf(" - failed.\n");
                return 23;
            }
            for (n = 0, pn(pns); pbuff[n] == (BYTE)pn(0) && n < (uint64_t)(sz_sect * 2); n++) ;
            if (n == (uint64_t)(sz_sect * 2)) {
                printf(" Data matched.\n");
            } else {
                printf("Failed: Read data differs from the data written.\n");
                return 24;
            }
        } else {
            printf(" Test skipped.\n");
        }
        pns++;

        printf("**** Test cycle %u of %u completed ****\n\n", cc, ncyc);
    }

    return 0;
}