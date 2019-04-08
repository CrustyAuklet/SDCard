/**
 * Copyright (c) 2011-2018 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef SDCARD_INFO_H
#define SDCARD_INFO_H
#include <cstdint>


namespace sd {

// Based on the document:
//
// SD Specifications
// Part 1
// Physical Layer
// Simplified Specification
// Version 5.00
// Aug 10, 2016
//
// https://www.sdcard.org/downloads/pls/
//------------------------------------------------------------------------------
// SD card errors
// See the SD Specification for command info.
enum class ErrorCode : uint8_t {
    NONE = 0,

    // Basic commands and switch command.
    CMD0 = 0X20,
    CMD2,
    CMD3,
    CMD6,
    CMD7,
    CMD8,
    CMD9,
    CMD10,
    CMD12,
    CMD13,

    // Read, write, erase, and extension commands.
    CMD17 = 0X30,
    CMD18,
    CMD24,
    CMD25,
    CMD32,
    CMD33,
    CMD38,
    CMD58,
    CMD59,

    // Application specific commands.
    ACMD6 = 0X40,
    ACMD13,
    ACMD23,
    ACMD41,

    // Read/write errors
    READ = 0X50,
    READ_CRC,
    READ_FIFO,
    READ_REG,
    READ_START,
    READ_TIMEOUT,
    STOP_TRAN,
    WRITE,
    WRITE_FIFO,
    WRITE_START,
    FLASH_PROGRAMMING,
    WRITE_TIMEOUT,

    // Misc errors.
    DMA = 0X60,
    ERASE,
    ERASE_SINGLE_BLOCK,
    ERASE_TIMEOUT,
    INIT_NOT_CALLED,
    FUNCTION_NOT_SUPPORTED
};

enum class CardType : uint8_t {
    UNK  = 0,
    SD1  = 1,    //< Standard capacity V1 SD card
    SD2  = 2,    //< Standard capacity V2 SD card
    SDHC = 3     //< High Capacity SD card
};

// SD card commands
enum class SDCMD : uint8_t {
    CMD0   = 0x00,    //< GO_IDLE_STATE - init card in spi mode if CS low
    CMD2   = 0x02,    //< ALL_SEND_CID - Asks any card to send the CID.
    CMD3   = 0x03,    //< SEND_RELATIVE_ADDR - Ask the card to publish a new RCA.
    CMD6   = 0x06,    //< SWITCH_FUNC - Switch Function Command
    CMD7   = 0x07,    //< SELECT/DESELECT_CARD - toggles between the stand-by and transfer states.
    CMD8   = 0x08,    //< SEND_IF_COND - verify SD Memory Card interface operating condition.
    CMD9   = 0x09,    //< SEND_CSD - read the Card Specific Data (CSD register)
    CMD10  = 0x0A,    //< SEND_CID - read the card identification information (CID register)
    CMD12  = 0x0C,    //< STOP_TRANSMISSION - end multiple block read sequence
    CMD13  = 0x0D,    //< SEND_STATUS - read the card status register
    CMD16  = 0x10,    //< SET_BLOCK_SIZE - set the block size on non SDHC cards
    CMD17  = 0x11,    //< READ_SINGLE_BLOCK - read a single data block from the card
    CMD18  = 0x12,    //< READ_MULTIPLE_BLOCK - read a multiple data blocks from the card
    CMD24  = 0x18,    //< WRITE_BLOCK - write a single data block to the card
    CMD25  = 0x19,    //< WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION
    CMD32  = 0x20,    //< ERASE_WR_BLK_START - sets the address of the first block to be erased
    CMD33  = 0x21,    //< ERASE_WR_BLK_END - sets the address of the last block of the continuous range to be erased
    CMD38  = 0x26,    //< ERASE - erase all previously selected blocks
    CMD55  = 0x37,    //< APP_CMD - escape for application specific command
    CMD58  = 0x3A,    //< READ_OCR - read the OCR register of a card
    CMD59  = 0x3B,    //< CRC_ON_OFF - enable or disable CRC checking
    ACMD6  = 0x06,    //< SET_BUS_WIDTH - Defines the data bus width for data transfer.
    ACMD13 = 0x0D,    //< SD_STATUS - Send the SD Status.
    ACMD22 = 0x16,
    ACMD23 = 0x17,    //< SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be pre-erased before writing
    ACMD41 = 0x29,    //< SD_SEND_OP_COMD - Sends host capacity support information and activates the card's initialization process
};

// CARD_STATUS
struct CardStatus {
    explicit CardStatus(const uint8_t s) : rawStatus(s) {}
    explicit CardStatus() = default;
    void operator=(const uint32_t v) { rawStatus = v; }

    /// The command's argument was out of the allowed range for this card.
    constexpr bool outOfRange() const   { return rawStatus & (1UL<<31); }
    /// A misaligned address which did not match the block length.
    constexpr bool addressError() const { return rawStatus & (1UL<<30); }
    /// The transferred block length is not allowed for this card.
    constexpr bool blockLengthError() const { return rawStatus & (1UL<<29); }
    /// An error in the sequence of erase commands occurred.
    constexpr bool EraseSeqError() const { return rawStatus & (1UL<<28); }
    /// An invalid selection of write-blocks for erase occurred.
    constexpr bool EraseParam() const   { return rawStatus & (1UL<<27); }
    /// Set when the host attempts to write to a protected block.
    constexpr bool WPViolation() const  { return rawStatus & (1UL<<26); }
    /// When set, signals that the card is locked by the host.
    constexpr bool isLocked() const     { return rawStatus & (1UL<<25); }
    /// Set when a sequence or password error has been detected.
    constexpr bool unlockFailed() const { return rawStatus & (1UL<<24); }
    /// The CRC check of the previous command failed.
    constexpr bool comCRCError() const  { return rawStatus & (1UL<<23); }
    /// Command not legal for the card state.
    constexpr bool illegalCommand() const { return rawStatus & (1UL<<22); }
    /// Card internal ECC was applied but failed to correct the data.
    constexpr bool cardECCFailed() const { return rawStatus & (1UL<<21); }
    /// Internal card controller error
    constexpr bool CCError() const { return rawStatus & (1UL<<20); }
    /// A general or an unknown error occurred during the operation.
    constexpr bool error() const { return rawStatus & (1UL<<19); }
    /// Permanent WP set or attempt to change read only values of  CSD.
    constexpr bool CSDOverwrite() const { return rawStatus & (1UL<<16); }
    /// partial address space was erased due to write protect.
    constexpr bool WPEraseSkip() const { return rawStatus & (1UL<<15); }

    /// The command has been executed without using the internal ECC.
    constexpr bool ECCDisabled() const { return rawStatus & (1UL<<14); }
    /// An erase sequence was cleared before executing because an out of erase sequence command was received.
    constexpr bool WPEraseReset() const { return rawStatus & (1UL<<13); }

    /// card state enum. it is a 4-bit numebr between 0 and 15 (9-15 are reserved)
    enum class CardState : uint8_t { 
        idle  = 0, 
        ready = 1, 
        ident = 2, 
        stby  = 3, 
        tran  = 4, 
        data  = 5, 
        rcv   = 6, 
        prg   = 7, 
        dis   = 8 
    };

    /** The state of the card when receiving the command. If a command changes the state it will
     *  be visable in the response to the next command.
     */
    constexpr CardState cardState() const { return static_cast<CardState>( (rawStatus>>9)&0x0F ); }

    /// Corresponds to buffer empty signaling on the bus.
    constexpr bool readyForData() const { return rawStatus & (1UL<<8); }
    /// Extension Functions may set this bit to get host to deal with events.
    constexpr bool FXEvent() const { return rawStatus & (1UL<<6); }
    /// The card will expect ACMD, or the command has been interpreted as ACMD
    constexpr bool appCmd() const { return rawStatus & (1UL<<5); }
    /// Error in the sequence of the authentication process.
    constexpr bool AKESeqError() const { return rawStatus & (1UL<<3); }

    uint32_t rawStatus;     //< the raw status received
};

static_assert(sizeof(CardStatus) == 4);

struct Response1 {
    explicit Response1(const uint8_t s) : rawStatus(s) {}
    explicit Response1() : rawStatus(0x80) {}
    void operator=(const uint8_t v) { rawStatus = v; }

    constexpr bool ParamError() const       { return rawStatus & (1UL<<6); }
    constexpr bool addressError() const     { return rawStatus & (1UL<<5); }
    constexpr bool eraseSeqError() const    { return rawStatus & (1UL<<4); }
    constexpr bool commandCRCError() const  { return rawStatus & (1UL<<3); }
    constexpr bool illegalCommand() const   { return rawStatus & (1UL<<2); }
    constexpr bool EraseReset() const       { return rawStatus & (1UL<<1); }

    constexpr bool busy() const             { return rawStatus == 0x80; }
    constexpr bool idle() const             { return rawStatus == 0x01; }
    constexpr bool ready() const            { return rawStatus == 0x00; }
    constexpr bool noResponse() const       { return rawStatus == 0xFF;  }
    operator bool() const                   { return (rawStatus & 0xFC) == 0; }
    
    uint8_t rawStatus;  //< raw status from device
};

static_assert(sizeof(Response1) == 1);

struct Response3 {
    constexpr bool pwrUpStatus() const      { return rawValue[0] & (1UL<<7); }
    constexpr bool ccs() const              { return rawValue[0] & (1UL<<6); }
    constexpr bool uhs2() const             { return rawValue[0] & (1UL<<5); }
    constexpr bool canSwitch1v8() const     { return rawValue[0] & (1UL<<0); }
    constexpr uint16_t vRange() const       { return ( (((uint16_t)rawValue[1])<<1) | (rawValue[2]>>7) ); }

    static constexpr uint8_t RAW_SIZE = 4;
    uint8_t rawValue[RAW_SIZE];         //< raw OCR status
};

    static_assert(sizeof(Response3) == 4);

/** start data token for read or write single block*/
constexpr uint8_t DATA_START_BLOCK = 0XFE;
/** stop token for write multiple blocks*/
constexpr uint8_t STOP_TRAN_TOKEN = 0XFD;
/** start data token for write multiple blocks*/
constexpr uint8_t WRITE_MULTIPLE_TOKEN = 0XFC;
/** mask for data response tokens after a write block operation */
constexpr uint8_t DATA_RES_MASK = 0X1F;
/** write data accepted token */
constexpr uint8_t DATA_RES_ACCEPTED = 0X05;

/**
 * @brief Card IDentification (CID) register.
 */
struct CID {
    /// get the product revision as floating point number
    constexpr float ProductRevision() const { return ProductRev_Major() + (ProductRev_Minor()/10); }
    /// get product revision major number
    constexpr unsigned ProductRev_Major() const { return ((prv&0xF0)>>4); }
    /// get product revision minor number
    constexpr unsigned ProductRev_Minor() const { return (prv&0x0F); }
    /// Manufacture Month
    constexpr unsigned ManufMonth() const { return (manufDate & 0x0F00)>>8; }
    /// Manufacture Year
    constexpr unsigned ManufYear() const { return (manufDate & 0x00FF) + 2000; }
    /// Get the CRC value
    constexpr uint8_t getCRC7() const { return (crc7&0xFE)>>1; }
    /// check the CRC7 value against provided value
    constexpr uint8_t checkCRC7(const uint8_t refCrc7) const { return getCRC7() == refCrc7; }

    uint8_t  mid;       //< manufacturer ID
    uint8_t  oid[2];    //< OEM/Application ID
    uint8_t  name[5];   //< Product Name
    uint8_t  prv;       //< product revision
    uint32_t psn;       //< Product Serial Number
    uint16_t manufDate; //< manufacturing Date
    uint8_t  crc7;      //< CRC7 checksum
} __attribute__((packed)) ;

static_assert(sizeof(CID) == 16);

/**
 * \brief CSD register for version 1.00 cards .
 */
typedef struct CSDV1 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  unsigned char taac;
  // byte 2
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  unsigned char c_size_high : 2;
  unsigned char reserved2 : 2;
  unsigned char dsr_imp : 1;
  unsigned char read_blk_misalign : 1;
  unsigned char write_blk_misalign : 1;
  unsigned char read_bl_partial : 1;
  // byte 7
  unsigned char c_size_mid;
  // byte 8
  unsigned char vdd_r_curr_max : 3;
  unsigned char vdd_r_curr_min : 3;
  unsigned char c_size_low : 2;
  // byte 9
  unsigned char c_size_mult_high : 2;
  unsigned char vdd_w_cur_max : 3;
  unsigned char vdd_w_curr_min : 3;
  // byte 10
  unsigned char sector_size_high : 6;
  unsigned char erase_blk_en : 1;
  unsigned char c_size_mult_low : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  unsigned char sector_size_low : 1;
  // byte 12
  unsigned char write_bl_len_high : 2;
  unsigned char r2w_factor : 3;
  unsigned char reserved3 : 2;
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved4 : 5;
  unsigned char write_partial : 1;
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved5: 2;
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Indicates the file format on the card */
  unsigned char file_format_grp : 1;
  // byte 15
  unsigned char always1 : 1;
  unsigned char crc : 7;
} __attribute__((packed)) csd1_t;

//==============================================================================
/**
 * \class CSDV2
 * \brief CSD register for version 2.00 cards.
 */
typedef struct CSDV2 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  /** fixed to 0X0E */
  unsigned char taac;
  // byte 2
  /** fixed to 0 */
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  /** This field is fixed to 9h, which indicates READ_BL_LEN=512 Byte */
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  /** not used */
  unsigned char reserved2 : 4;
  unsigned char dsr_imp : 1;
  /** fixed to 0 */
  unsigned char read_blk_misalign : 1;
  /** fixed to 0 */
  unsigned char write_blk_misalign : 1;
  /** fixed to 0 - no partial read */
  unsigned char read_bl_partial : 1;
  // byte 7
  /** high part of card size */
  unsigned char c_size_high : 6;
  /** not used */
  unsigned char reserved3 : 2;
  // byte 8
  /** middle part of card size */
  unsigned char c_size_mid;
  // byte 9
  /** low part of card size */
  unsigned char c_size_low;
  // byte 10
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_high : 6;
  /** fixed to 1 - erase single is supported */
  unsigned char erase_blk_en : 1;
  /** not used */
  unsigned char reserved4 : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_low : 1;
  // byte 12
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_high : 2;
  /** fixed value of 2 */
  unsigned char r2w_factor : 3;
  /** not used */
  unsigned char reserved5 : 2;
  /** fixed value of 0 - no write protect groups */
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved6 : 5;
  /** always zero - no partial block read*/
  unsigned char write_partial : 1;
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved7: 2;
  /** Do not use always 0 */
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Do not use always 0 */
  unsigned char file_format_grp : 1;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** checksum */
  unsigned char crc : 7;
} __attribute__((packed)) csd2_t;

//==============================================================================
/**
 * \class csd_t
 * \brief Union of old and new style CSD register.
 */
union csd_t {
  csd1_t v1;
  csd2_t v2;
};

static_assert(sizeof(csd1_t) == 16);
static_assert(sizeof(csd2_t) == 16);
static_assert(sizeof(csd_t)  == 16);

//-----------------------------------------------------------------------------
inline uint32_t sdCardCapacity(csd_t* csd) {
  if (csd->v1.csd_ver == 0) {
    uint8_t read_bl_len = csd->v1.read_bl_len;
    uint16_t c_size = (csd->v1.c_size_high << 10)
                      | (csd->v1.c_size_mid << 2) | csd->v1.c_size_low;
    uint8_t c_size_mult = (csd->v1.c_size_mult_high << 1)
                          | csd->v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  } else if (csd->v2.csd_ver == 1) {
    uint32_t c_size = 0X10000L * csd->v2.c_size_high + 0X100L
                      * (uint32_t)csd->v2.c_size_mid + csd->v2.c_size_low;
    return (c_size + 1) << 10;
  } else {
    return 0;
  }
}

}   // namespace sd

#endif  // Header guard
