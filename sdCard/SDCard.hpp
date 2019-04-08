#ifndef SDCARD_SPI_H
#define SDCARD_SPI_H
#include <stddef.h>
#include "SDCard_info.h"

#include <cstdio>
#define SPISD_DEBUG(...)  do { fprintf(stderr, __VA_ARGS__); } while(0)

namespace sd {

    template<class SPIShim, class SDPolicy>
    class SpiCard : private SPIShim, SDPolicy {
    public:
        using SDPolicyTimeType = typename SDPolicy::timeType;

        /// constructor
        SpiCard() : m_errorCode(ErrorCode::INIT_NOT_CALLED), m_type(CardType::UNK) {}

//        Response1 lowLevelCommand(const SDCMD cmd, const uint32_t arg)
//        {
//            Response1 response;
//            uint8_t  cmdPacket[6];
//            uint32_t crc;
//
//            // Prepare the command packet
//            cmdPacket[0] = static_cast<uint8_t>(cmd);
//            cmdPacket[1] = (arg >> 24);
//            cmdPacket[2] = (arg >> 16);
//            cmdPacket[3] = (arg >> 8);
//            cmdPacket[4] = (arg >> 0);
//
//            if ( SDPolicy::useCRC7::value ) {
//                cmdPacket[5] = SDPolicy::getCRC7(cmdPacket, 5);
//            }
//            else {
//                // CMD0 is executed in SD mode, hence should have correct CRC
//                // CMD8 CRC verification is always enabled
//                switch (cmd) {
//                    case SDCMD::CMD0 :
//                        cmdPacket[5] = 0x95;
//                        break;
//                    case SDCMD::CMD8 :
//                        cmdPacket[5] = 0x87;
//                        break;
//                    default:
//                        cmdPacket[5] = 0xFF;    // Make sure bit 0-End bit is high
//                        break;
//                }
//            }
//
//            // send the command
//            SPIShim::select();
//            SPIShim::write(cmdPacket, 6);
//            SPIShim::deSelect();
//
//            // The received byte immediataly following CMD12 is a stuff byte,
//            // it should be discarded before receive the response of the CMD12.
//            if (SDCMD::CMD12 == cmd) {
//                SPIShim::read();
//            }
//
//            // Loop for response: Response is sent back within command response time (NCR), 0 to 8 bytes for SDC
//            for (int i = 0; i < 0x10; i++) {
//                response = SPIShim::read();
//                // Got the response
//                if (!response.noResponse()) {
//                    break;
//                }
//            }
//            return response;
//        }
//
//        Response1 command(const SDCMD cmd, const uint32_t arg, const bool isACMD = false)
//        {
//            // Select card and wait for card to be ready before sending next command
//            // Note: next command will fail if card is not ready
//            SPIShim::select();
//
//            // No need to wait for card to be ready when sending the stop command
//            if (SDCMD::CMD12 != cmd) {
//                if (!waitReady(SDPolicy::cmdTimeout::value)) {
//                    SPISD_DEBUG("TIMEOUT: Card not ready for command %d ()\n", (int)cmd);
//                }
//            }
//
//            // Re-try command 3 times
//            for (int i = 0; i < 3; i++) {
//                // Send CMD55 for APP command first
//                if (isACMD) {
//                    m_status = lowLevelCommand( SDCMD::CMD55, 0);
//                    // Wait for card to be ready after CMD55
//                    if (!waitReady(SDPolicy::cmdTimeout::value)) {
//                        SPISD_DEBUG("TIMEOUT: Card not ready aftern command 55 (APP command prep)\n");
//                    }
//                }
//
//                // Send command over SPI interface
//                m_status = lowLevelCommand(cmd, arg);
//                if (m_status.noResponse()) {
//                    SPISD_DEBUG("No response CMD: %d\n", (int)cmd);
//                    continue;
//                }
//                break;
//            }
//
//            if(m_status.noResponse()){
//                SPISD_DEBUG("CMD %d ERROR: No response (0x%02)\n", (int)cmd, m_status.rawStatus);
//                SPIShim::deSelect();
//                return m_status;
//            }
//            if(m_status.commandCRCError()) {
//                SPISD_DEBUG("CMD %d ERROR: CRC Error (0x%02)\n", (int)cmd, m_status.rawStatus);
//                SPIShim::deSelect();
//                return m_status;
//            }
//            if(m_status.illegalCommand()) {
//                SPISD_DEBUG("CMD %d ERROR: Illegal Command (0x%02)\n", (int)cmd, m_status.rawStatus);
//                SPIShim::deSelect();
//                return m_status;
//            }
//
//            SPISD_DEBUG("CMD: %d \t arg:0x%x \t Response:0x%x \n", (int)cmd, arg, m_status.rawStatus);
//            // Set status for other errors
//            if (m_status.EraseReset() || m_status.eraseSeqError()) {
//                SPISD_DEBUG("WARNING: Erase Error (0x%02)\n", m_status.rawStatus);
//            }
//            else if (m_status.addressError() || m_status.ParamError()) {
//                // Misaligned address / invalid address block length
//                SPISD_DEBUG("WARNING: Address Error (0x%02)\n", m_status.rawStatus);
//            }
//
//            // Get rest of the response part for other commands
//            uint32_t r;
//            switch (cmd) {
//                case SDCMD::CMD8 :             // Response R7
//                    SPISD_DEBUG("V2-Version Card\n");
//                    m_type = CardType::SD2;
//                    // Note: No break here, need to read rest of the response
//                case SDCMD::CMD58 :                // Response R3
//                    r  = (SPIShim::read() << 24);
//                    r |= (SPIShim::read() << 16);
//                    r |= (SPIShim::read() << 8);
//                    r |=  SPIShim::read();
//                    SPISD_DEBUG("R3/R7: 0x%x \n", r);
//                    break;
//
//                case SDCMD::CMD12 :       // Response R1b
//                case SDCMD::CMD38 :
//                    waitReady(SDPolicy::cmdTimeout::value);
//                    break;
//
//                case SDCMD::ACMD13 :             // Response R2
//                    r = SPIShim::read();
//                    SPISD_DEBUG("R2: 0x%x \n", r);
//                    break;
//
//                default:                            // Response R1
//                    break;
//            }
//
//            // Do not deselect card if read is in progress.
//            if (((SDCMD::CMD9 == cmd) || (SDCMD::ACMD22 == cmd) ||
//                 (SDCMD::CMD24 == cmd) || (SDCMD::CMD25 == cmd) ||
//                 (SDCMD::CMD17 == cmd) || (SDCMD::CMD18 == cmd)) && m_status)
//            {
//                return m_status;
//            }
//
//            // Deselect card
//            SPIShim::deSelect();
//            return m_status;
//        }

        /** Initialize the SD card.
         * @param spi [in] SPI instance
         * @param csPin [in] chip select for the SD card
         * \return true for success else false.
         */
        bool begin();

        /**
         * Read a card's CID register. The CID contains card identification
         * information such as Manufacturer ID, Product name, Product serial
         * number and Manufacturing date.
         *
         * @param cid [out] pointer to area for returned data.
         * @return True for success or False for failure.
         */
        CID readCID(); //{ return readRegister(SDCMD::CMD10, cid); }

        /**
         * Read a card's CSD register. The CSD contains Card-Specific Data that
         * provides information regarding access to the card's contents.
         *
         * @param[out] csd pointer to area for returned data.
         * @return true for success or false for failure.
         */
        bool readCSD(csd_t *csd); //{ return readRegister(CMD9, csd); }

        /**
         * Determine the size of an SD flash memory card.
         * @return The number of 512 byte sectors in the card or zero if an error occurs.
         */
        uint32_t cardCapacity();
//    {
//        csd_t csd;
//        return readCSD(&csd) ? sdCardCapacity(&csd) : 0;
//    }

        /// Get the card type
        CardType type() const { return m_type; }

        /** Erase a range of blocks.
         * @param firstBlock [in] The address of the first block in the range.
         * @param lastBlock [in] The address of the last block in the range.
         *
         * @note This function requests the SD card to do a flash erase for a
         * range of blocks.  The data on the card after an erase operation is
         * either 0 or 1, depends on the card vendor.  The card must support
         * single block erase.
         *
         * @return True is returned for success
         * @return False is returned for failure
         */
        bool erase(uint32_t firstBlock, uint32_t lastBlock);

        /** Determine if card supports single block erase.
         * @return true is returned if single block erase is supported.
         * @return false is returned if single block erase is not supported.
         */
        bool eraseSingleBlockEnable();
//    {
//        csd_t csd;
//        return readCSD(&csd) ? csd.v1.erase_blk_en : false;
//    }


        /**
         * Read multiple 512 byte blocks from an SD card.
         *
         * @param[in] lba Logical block to be read.
         * @param[in] nb Number of blocks to be read.
         * @param[out] dst Pointer to the location that will receive the data.
         * @return True is returned for success and False is returned for failure.
         */
        ssize_t read(const uint32_t LBA, uint8_t* buf, const size_t LEN)
        {
            // cmd18 for reading multiple blocks, otherwise single block read
            const SDCMD readCommand = (LEN > 1 ? SDCMD::CMD18 : SDCMD::CMD17);
            ssize_t readCount = 0;

            if(LEN > 0) {
                SPISD_DEBUG("Sending Read command...\n");
                spiWait(4);
                SPIShim::select();
                auto r1 = cardCommand(readCommand, LBA);
                SPIShim::deSelect();
                spiWait(4);

                if(!r1.ready()) {
                    SPISD_DEBUG("    Read command not ready 0x%02X\n", r1.rawStatus);
                    return -1;
                }

            }

            return readCount;
        }

        /**
         * Read a 512 byte block from an SD card.
         *
         * @param[in] lba Logical block to be read.
         * @param[out] dst Pointer to the location that will receive the data.
         * @return True is returned for success andFalse is returned for failure.
         */
        ssize_t readBlock(uint32_t LBA, uint8_t* buf)
        {
            SPISD_DEBUG("ReadBlock: LBA 0x%08X ...\n", LBA);
            if(m_type != CardType::SDHC) {
                LBA = LBA<<9;
                SPISD_DEBUG("    Non-SDHC card, shifting by 9 for block address: 0x%08X\n", LBA);
            }

            SPISD_DEBUG("CMD17: Sending single block read...\n");
            spiWait(4);
            SPIShim::select();
            auto r1 = cardCommand(SDCMD::CMD17, LBA);

            if(!r1.ready()) {
                SPIShim::deSelect();
                SPISD_DEBUG("    Read command not ready 0x%02X\n", r1.rawStatus);
                return -1;
            }

            const uint8_t dt = waitResponse();
            if(0xFE == dt) {
                SPIShim::read(buf, 512);

                const uint16_t crc = (SPIShim::read() << 8) | SPIShim::read();
                SPIShim::deSelect();

                if( SDPolicy::useCRC16 && (crc != SDPolicy::CRC_CCITT(buf, 512)) ) {
                    SPISD_DEBUG("    CRC check failed! (0x%04X)\n", crc);
                    return -2;
                }
            }
            else {
                if(dt & (1UL<<1)) { SPISD_DEBUG("    CC ERROR! (0x%02X)\n", dt); }
                else if(dt & (1UL<<2)) { SPISD_DEBUG("    CARD ECC FAILED! (0x%02X)\n", dt); }
                else if(dt & (1UL<<3)) { SPISD_DEBUG("    ADDRESS OUT OF RANGE! (0x%02X)\n", dt); }
                else if(dt & (1UL<<4)) { SPISD_DEBUG("    CARD LOCKED! (0x%02X)\n", dt); }
                return -1;
            }

            return 1;
        }


        /** Read one data block in a multiple block read sequence
         *
         * @param[out] dst Pointer to the location for the data to be read.
         * @return True is returned for success and False is returned for failure.
         */
        bool readData(uint8_t *dst);

        /** Read OCR register.
         *
         * @param[out] ocr Value of OCR register.
         * @return true for success else false.
         */
        bool readOCR(uint32_t *ocr);

        /** Return the 64 byte card status
         * @param[out] status location for 64 status bytes.
         * @return True is returned for success and False is returned for failure.
         */
        bool readStatus(uint8_t *status);

    private:
        Response1 cardAcmd(SDCMD cmd, uint32_t arg)
        {
            cardCommand(SDCMD::CMD55, 0);
            return cardCommand(cmd, arg);
        }

        Response1 cardCommand(SDCMD cmd, uint32_t arg = 0)
        {
            // wait if busy unless CMD0
            if (cmd != SDCMD::CMD0) {
                waitNotBusy(SDPolicy::cmdTimeout::value);
            }

            if(SDPolicy::useCRC7) {
                const uint8_t rawCmd = static_cast<uint8_t>(cmd);
                // form message
                uint8_t buf[6];
                buf[0] = (uint8_t)0x40U | rawCmd;
                buf[1] = (uint8_t)(arg >> 24U);
                buf[2] = (uint8_t)(arg >> 16U);
                buf[3] = (uint8_t)(arg >> 8U);
                buf[4] = (uint8_t)arg;

                // add CRC
                buf[5] = SDPolicy::getCRC7(buf, 5);

                // send message
                SPIShim::write(buf, 6);
            }
            else {
                // send command
                SPIShim::write( static_cast<uint8_t>(cmd) |  uint8_t(0x40) );

                // send argument
                uint8_t *pa = reinterpret_cast<uint8_t *>(&arg);
                for (int8_t i = 3; i >= 0; i--) {
                    SPIShim::write(pa[i]);
                }
                // send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
                const uint8_t writeVal = ( (cmd == SDCMD::CMD0) ? uint8_t(0x95) : uint8_t(0x87) );
                SPIShim::write(writeVal);
            }

            // there are 1-8 fill bytes before response.  fill bytes should be 0XFF.
            const Response1 r1( waitResponse() );
            return r1;
        }

        /**
         * Check for busy.  MISO low indicates the card is busy.
         * @return true once the card is not busy response is 0xFF). False if timeout occurs
         */
        bool waitNotBusy(uint16_t timeoutMS)
        {
            SDPolicyTimeType t0 = SDPolicy::getTime();
            // Check not busy first since yield is not called in isTimedOut.
            while (SPIShim::read() != 0XFF) {
                if (SDPolicy::isTimedOut(t0, timeoutMS)) {
                    return false;
                }
            }
            return true;
        }

        /// wait for count while outputting SD fill character
        void spiWait(const uint8_t count) {
            for (uint8_t i = 0; i < count; i++) {
                SPIShim::write(0xFF);
            }
        }

        /// wait till chip is ready with a millisecond timeout
        bool waitReady(const SDPolicyTimeType ms)
        {
            uint8_t response;
            const auto endTime = SDPolicy::getTime() + ms;
            do {
                response = SPIShim::write(0xFF);
            } while( (SDPolicy::getTime() < endTime) && (response != 0xFF) );
            return response == 0xFF;
        }

        /// SPI function to wait till chip is ready and sends start token. Times out after 300ms.
        bool waitToken(uint8_t token)
        {
            const auto endTime = SDPolicy::getTime() + SDPolicy::cmdTimeout::value;
            do {
                if (token == SPIShim::write(0xFF)) {
                    return true;
                }
            } while ( SDPolicy::getTime() < endTime );       // Wait for 300 msec for start token
            SPISD_DEBUG("Timeout: waiting for token 0x%02X\n", token);
            return false;
        }

        uint8_t waitResponse()
        {
            uint8_t response;
            const auto endTime = SDPolicy::getTime() + SDPolicy::cmdTimeout::value;
            do {
                response = SPIShim::write(0xFF);
            } while(response == 0xFF && SDPolicy::getTime() < endTime );
            return response;
        }

        ErrorCode       m_errorCode;
        CardType        m_type;
    };

template<class SPIShim, class SDPolicy>
bool SpiCard<SPIShim, SDPolicy>::begin()
{
    m_errorCode = ErrorCode::NONE;
    m_type      = CardType::UNK;
    Response1 r1;

    SPIShim::begin();

    // must supply min of 74 clock cycles with CS high.
    SPISD_DEBUG("Sending 80 clock cycles of 0xFF...\n");
    SPIShim::deSelect();
    for(int i = 0; i < 10; i++) {
        SPIShim::write(0xFF);
    }

    // Set Idle state: If MCU is reset but SD card is not, it may ignore the first CMD0
    // TODO: better loop (without break) and more checks from other drivers

    for (int i = 0; i < SDPolicy::cmd0_retry::value; i++) {
        SPISD_DEBUG("Sending CMD0: Set to idle state...\n");

        spiWait(4);
        SPIShim::select();
        r1 = cardCommand( SDCMD::CMD0 );
        SPIShim::deSelect();
        spiWait(4);

        if (r1.idle()) {
            SPISD_DEBUG("    CMD0 Success!\n");
            break;
        }

        spiWait(4);
        SPIShim::select();
        SPIShim::write(STOP_TRAN_TOKEN);
        SPIShim::deSelect();
        spiWait(4);
        for(int i = 0; i < 520; i++) {
            SPIShim::read();
        }
    }


    if(!r1.idle()) {
        SPISD_DEBUG("    CMD0 Failed!\n");
        return false;
    }

    // TODO : arduio uses the following code to (I assume) handle the case where the system resets during
    //          a write/read and the SD card is confused (in a loop with the CMD0). do this?

//                SPIShim::select();
//                // stop multi-block write
//                SPIShim::write(STOP_TRAN_TOKEN);
//
//                // finish block transfer
//                for (int j = 0; j < 520; j++) {
//                    SPIShim::write(0xFF);
//                }
//                SPIShim::deSelect();
//            }

    // TODO: should we explicetly deactivate CRC7? spec says it is off by default on SPI, but rumor has it that
    //       many cards do not follow that

    // check SD version
    SPISD_DEBUG("Sending CMD8: check SD version...\n");
    spiWait(4);
    SPIShim::select();
    r1 = cardCommand(SDCMD::CMD8, 0x1AA);
    if(r1.illegalCommand()) {
        SPISD_DEBUG("    CMD8 Invalid - SDv1\n");
        m_type = CardType::SD1;
    }
    else {
        uint8_t r7[4];
        SPIShim::read(r7, 4);

        if (r7[3] == 0XAA) {
            SPISD_DEBUG("    SD Card Type: SD2\n");
            m_type = CardType::SD2;
        }
        else {
            SPISD_DEBUG("    ERROR: COMMAND 8\n");
            m_errorCode = ErrorCode::CMD8;
            return false;
        }
    }
    SPIShim::deSelect();

    // give the card some clocks
    spiWait(4);

    // initialize card and send host supports SDHC if SD2
    // TODO: check CMD1 for old cards if ACMD41 returns an error or no response
    const uint32_t arg = (m_type == CardType::SD2 ? 0X40000000 : 0);
    for(int i = 0; i < 3 && !r1.ready(); ++i ) {
        SPISD_DEBUG("Sending ACMD41: activate card init %s...\n", (arg == 0 ? "" : "and asserting SDHC capabilty" ) );

        spiWait(2);
        SPIShim::select();
        r1 = cardAcmd(SDCMD::ACMD41, arg);
        SPIShim::deSelect();
        spiWait(2);
    }
    // abort if no response to ACMD41
    if(!r1 || !r1.ready()) {
        SPISD_DEBUG("    No valid response from ACMD41!\n");
        return false;
    }

    // if SD2 read OCR register to check for SDHC card
    if ( m_type == CardType::SD2) {
        // Get the OCR to check voltage levels
        SPIShim::select();
        SPISD_DEBUG("Sending CMD58: checking OCR to see if SDHC card...\n");
        r1 = cardCommand(SDCMD::CMD58, 0);
        if(r1) {
            Response3 r3;
            SPIShim::read(r3.rawValue, Response3::RAW_SIZE);
            if(r3.ccs() && r3.pwrUpStatus()) {
                m_type = CardType::SDHC;
            }
            SPISD_DEBUG("    OCR: 0x04X ... %s type card\n", (m_type == CardType::SDHC ? "SDHC" : "non-SDHC") );
        }
        else {
            SPISD_DEBUG("    No valid response from CMD58!\n");
            return false;
        }
        SPIShim::deSelect();
    }

    // set block size to 512 for older cards
    if(m_type != CardType::SDHC) {
        SPISD_DEBUG("Sending CMD16: Setting block size to 512...\n");
        spiWait(2);
        SPIShim::select();
        r1 = cardAcmd(SDCMD::CMD16, 512);
        SPIShim::deSelect();
        spiWait(2);
        if(!r1) {
            SPISD_DEBUG("    No valid response from CMD16!\n");
            return false;
        }
    }

    return true;
}

}       // sd namespace
#endif  // include gaurd