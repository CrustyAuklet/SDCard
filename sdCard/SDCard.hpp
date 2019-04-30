#ifndef SDCARD_SPI_H
#define SDCARD_SPI_H

#include <stddef.h>
#include <optional>
#include "SDCard_info.h"
#include "SDDefaultPolicies.h"

#include <cstdio>
#define SPISD_DEBUG(...)  do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)

namespace sd {

template<class SPIShim, class SDPolicy = sd::ShiftedCRC, class TimeoutPolicy = sd::CountBasedTimouts >
class SpiCard : private SPIShim, SDPolicy, TimeoutPolicy {
public:
    using SDPolicyTimeType = typename TimeoutPolicy::timeType;

    /// constructor
    SpiCard() noexcept : m_errorCode(ErrorCode::INIT_NOT_CALLED), m_type(CardType::UNK) {}

    /// initialize the SD card. Returns true if the card is successfully configured.
    bool begin();
    /// Get the card type
    CardType type() const { return m_type; }
    /// Read a card's CID register. The CID contains Manufacturer ID, product name, serial number, etc.
    std::optional<CID> readCID();
    /// get the CSD register that contains information reguarding the cards content
    std::optional<CSD> readCSD();
    /// get the OCR register
    std::optional<OCR> readOCR();
    /// get the 64 byte card status register
    std::optional<CardStatus> readStatus() { return std::optional<CardStatus>(); }

    /// Get the number of blocks in the SD card (each block is 512 Bytes)
    std::optional<uint32_t> cardCapacity() {
        const auto csd = readCSD();
        return csd.has_value() ? std::optional<uint32_t>(csd->blockCount()) : std::optional<uint32_t>();
    }

    /// Determine if card supports single block erase.
    bool eraseSingleBlockEnable() const {
        const auto csd = readCSD();
        return csd.has_value() ? csd->eraseBlockEnabled() : false;
    }

    /**
     * Read multiple 512 byte blocks from an SD card.
     * @param LBA [in] Logical block to be read.
     * @param buf [in] location to write the incoming blocks
     * @param LEN [out] number of blocks to read
     * @return The number of blocks read, or a value < 0 for an error
     */
    ssize_t readBlocks(uint32_t LBA, uint8_t* buf, const size_t LEN);

    /**
     * Write multiple 512 byte blocks to an SD card.
     * @param LBA [in] logical block to be written.
     * @param src [in] pointer to the location of the data to be written.
     * @param LEN [in] number of blocks to be written.
     * @return the number of blocks written, or a negative value.
     * @note if All blocks are not written the state of the remaining blocks are undefined
     */
    ssize_t writeBlocks(uint32_t LBA, const uint8_t* src, size_t LEN);

private:
    Response1 cardAcmd(SDCMD cmd, uint32_t arg) {
        cardCommand(SDCMD::CMD55, 0);
        return cardCommand(cmd, arg);
    }

    Response1 cardCommand(SDCMD cmd, uint32_t arg = 0)
    {
        // wait if busy unless CMD0
        if (cmd != SDCMD::CMD0) {
            waitNotBusy(TimeoutPolicy::cmdTimeout::value);
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
        const Response1 r1( waitResponse(TimeoutPolicy::cmdTimeout::value) );
        return r1;
    }

    /// wait for count while outputting SD fill character
    void spiWait(const uint8_t count) {
        for (uint8_t i = 0; i < count; i++) {
            SPIShim::write(0xFF);
        }
    }

    /**
     * Check for busy.  MISO low indicates the card is busy.
     * @return true once the card is not busy response is 0xFF). False if timeout occurs
     */
    bool waitNotBusy(const uint16_t timeoutMS) {
        auto t0 = TimeoutPolicy::getTime();
        while (SPIShim::read() != 0XFF) {
            if (TimeoutPolicy::isTimedOut(t0, timeoutMS)) {
                return false;
            }
        }
        return true;
    }

    uint8_t waitResponse(const uint16_t timeoutMS) {
        uint8_t response;
        auto t0 = TimeoutPolicy::getTime();
        do {
            response = SPIShim::write(0xFF);
        } while(response == 0xFF && !TimeoutPolicy::isTimedOut(t0, timeoutMS) );
        return response;
    }

    /// start a read operation
    bool readStart(uint32_t LBA, const uint32_t COUNT);
    /// read a single block of data with CRC if specified. Rtuen TRUE if CRC passes (or unused)
    bool readData(uint8_t* buf);
    /// end a multi-block read sequence
    bool readStop();
    /// start a multi-block write with an erase command before starting
    bool writeStart(uint32_t LBA, const uint32_t COUNT);
    /// write a single 512 block of data, with CRC and response check
    bool writeData(const uint8_t token, const uint8_t* src);
    /// Stop a write
    bool writeStop();

    static constexpr uint8_t DATA_START_BLOCK = 0xFE;       //< start data token for read or write single block*/
    static constexpr uint8_t STOP_TRAN_TOKEN = 0xFD;        //< stop token for write multiple blocks
    static constexpr uint8_t WRITE_MULTIPLE_TOKEN = 0xFC;   //< start data token for write multiple blocks
    static constexpr uint8_t DATA_RES_MASK = 0x1F;          //< mask for data response tokens after a write block operation
    static constexpr uint8_t DATA_RES_ACCEPTED = 0x05;      //< write data accepted token

    ErrorCode       m_errorCode;
    CardType        m_type;
};

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::begin()
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

    for (int i = 0; i < TimeoutPolicy::cmd0_retry::value; i++) {
        SPISD_DEBUG("Sending CMD0: Set to idle state...\n");

        spiWait(4);
        SPIShim::select();
        r1 = cardCommand( SDCMD::CMD0 );
        SPIShim::deSelect();
        spiWait(2);

        if (r1.idle()) {
            SPISD_DEBUG("    CMD0 Success!\n");
            break;
        }

        spiWait(1);
        SPIShim::select();
        writeStop();
        waitResponse(520);
        SPIShim::deSelect();
        spiWait(2);
    }


    if(!r1.idle()) {
        SPISD_DEBUG("    CMD0 Failed!\n");
        return false;
    }

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
            return false;
        }
    }
    SPIShim::deSelect();
    spiWait(2);

    // initialize card and send host supports SDHC if SD2
    // TODO: check CMD1 for old cards if ACMD41 returns an error or no response
    const uint32_t arg = (m_type == CardType::SD2 ? 0X40000000 : 0);
    for(int i = 0; i < 3 && !r1.ready(); ++i ) {
        SPISD_DEBUG("Sending ACMD41: activate card init %s...\n", (arg == 0 ? "" : "and asserting SDHC capabilty" ) );

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
        const auto ocr = readOCR();
        if(ocr.has_value()) {
            if(ocr->ccs() && ocr->pwrUpStatus()) {
                m_type = CardType::SDHC;
            }
            SPISD_DEBUG("    OCR: 0x04X ... %s type card\n", (m_type == CardType::SDHC ? "SDHC" : "non-SDHC") );
        }
        else {
            SPISD_DEBUG("Failed to receive OCR!\n");
            return false;
        }
        SPIShim::deSelect();
        spiWait(2);
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

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
std::optional<CID> SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readCID()
{
    CID cid;
    bool success = false;

    SPIShim::select();
    const auto r1 = cardCommand(SDCMD::CMD10, 0);
    if(r1) {
        const auto dt = waitResponse(TimeoutPolicy::cmdTimeout::value);
        if(DATA_START_BLOCK == dt) {
            success = SPIShim::read(cid.raw.data(), cid.raw.size());
        }
    }
    SPIShim::deSelect();
    spiWait(2);
    return success ? std::optional<CID>(cid) : std::optional<CID>();
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
std::optional<CSD> SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readCSD()
{
    CSD csd;
    bool success = false;

    SPIShim::select();
    const auto r1 = cardCommand(SDCMD::CMD9, 0);
    if(r1) {
        const auto dt = waitResponse(TimeoutPolicy::cmdTimeout::value);
        if(DATA_START_BLOCK == dt) {
            success = SPIShim::read(csd.raw.data(), csd.raw.size());
        }
    }
    SPIShim::deSelect();
    spiWait(2);
    return success ? std::optional<CSD>(csd) : std::optional<CSD>();
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
std::optional<OCR> SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readOCR()
{
    OCR ocr;  // return value OCR register
    bool success = false;

    SPIShim::select();
    if(const auto r1 = cardCommand(SDCMD::CMD58, 0); r1) {
        success = SPIShim::read(ocr.raw.data(), ocr.raw.size());
    }
    SPIShim::deSelect();
    spiWait(2);
    return success ? std::optional<OCR>(ocr) : std::optional<OCR>();
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
ssize_t SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readBlocks(uint32_t LBA, uint8_t* buf, const size_t LEN)
{
    ssize_t readCount = 0;

    SPISD_DEBUG("Reading %d blocks starting at block 0x%08X\n", LEN, LBA);
    spiWait(1);
    SPIShim::select();
    if(!readStart(LBA, LEN)) {
        SPIShim::deSelect();
        spiWait(2);
        return -1;
    }

    for(readCount = 0; readCount < LEN; readCount++, buf += 512) {
        SPISD_DEBUG("  Reading block %d!\n", readCount);
        if(!readData(buf)) {
            SPISD_DEBUG("    Read Data Failed!\n");
            SPIShim::deSelect();
            spiWait(2);
            break;
        }
    }

    if(LEN > 1) {
        readStop();
    }
    SPIShim::deSelect();
    spiWait(2);
    return readCount;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
ssize_t SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::writeBlocks(uint32_t LBA, const uint8_t* src, const size_t LEN)
{
    ssize_t writeCount = 0;

    SPISD_DEBUG("Writing %d blocks starting at block 0x%08X\n", LEN, LBA);
    spiWait(1);
    SPIShim::select();
    if(!writeStart(LBA, LEN)) {
        SPIShim::deSelect();
        spiWait(2);
        return -1;
    }

    spiWait(1);

    const uint8_t startToken = LEN > 1 ? WRITE_MULTIPLE_TOKEN : DATA_START_BLOCK;
    for(writeCount = 0; writeCount < LEN; writeCount++, src += 512) {
        SPISD_DEBUG("  Writing block %d!\n", writeCount);
        if(!writeData(startToken, src)) {
            if(LEN > 1) { writeStop(); }
            SPIShim::deSelect();
            spiWait(2);
            SPISD_DEBUG("    Write Data Failed!\n");
            break;
        }

        if(!waitNotBusy(TimeoutPolicy::writeTimeout::value)) {
            SPIShim::deSelect();
            spiWait(2);
            SPISD_DEBUG("    Post-Write timeout!\n");
            return -1;
        }
    }

    if(LEN > 1) {
        writeStop();
        spiWait(1);
    }

    // TODO: check write status using CMD13 ACMD22 according to spec
//    if(!waitNotBusy(TimeoutPolicy::writeTimeout::value)) {
//        SPIShim::deSelect();
//        spiWait(2);
//        SPISD_DEBUG("    Post-Write timeout!\n");
//        return -1;
//    }
//
//    if(LEN > 1) {
//        const auto r1 = cardAcmd(SDCMD::ACMD22);
//    }
//    else {
//        const auto r1 = cardCommand(SDCMD::CMD13);
//        Response2 r2{SPIShim::read()};
//        if (!r1 || !r2) {
//            SPIShim::deSelect();
//            spiWait(2);
//            SPISD_DEBUG("    Write Error %02X!\n", r2.rawStatus);
//            return -1;
//        }
//    }

    SPIShim::deSelect();
    spiWait(2);
    return writeCount;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readStart(uint32_t LBA, const uint32_t COUNT)
{
    // Byte addressing for non-sdhc cards, so multiply address by 512
    if(m_type != CardType::SDHC) { LBA = LBA<<9; }

    // cmd18 for reading multiple blocks, otherwise single block read
    const SDCMD readCommand = (COUNT > 1 ? SDCMD::CMD18 : SDCMD::CMD17);

    SPISD_DEBUG("Reading %d blocks starting at block 0x%08X\n", COUNT, LBA);
    const auto r1 = cardCommand(readCommand, LBA);
    if(!r1.ready()) { SPISD_DEBUG("    Read start failed! (0x%02X)\n", r1.rawStatus); }
    return r1;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readData(uint8_t* buf)
{
    const uint8_t dt = waitResponse(TimeoutPolicy::cmdTimeout::value);
    if(DATA_START_BLOCK == dt) {
        SPIShim::read(buf, 512);

        const uint16_t crc = (SPIShim::read() << 8) | SPIShim::read();

        if( SDPolicy::useCRC16 && (crc != SDPolicy::CRC_CCITT(buf, 512)) ) {
            SPISD_DEBUG("    CRC check failed! (0x%04X)\n", crc);
            return false;
        }
        return true;
    }

    if(dt == 0xFF) { SPISD_DEBUG("    Timed Out with no response! (0x%02X)\n", dt); }
    else if(dt & (1UL<<1)) { SPISD_DEBUG("    CC ERROR! (0x%02X)\n", dt); }
    else if(dt & (1UL<<2)) { SPISD_DEBUG("    CARD ECC FAILED! (0x%02X)\n", dt); }
    else if(dt & (1UL<<3)) { SPISD_DEBUG("    ADDRESS OUT OF RANGE! (0x%02X)\n", dt); }
    else if(dt & (1UL<<4)) { SPISD_DEBUG("    CARD LOCKED! (0x%02X)\n", dt); }
    return false;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::readStop()
{
    const auto r = cardCommand(SDCMD::CMD12);
    if(!r.ready()) {
        SPISD_DEBUG("CMD12 Error: Stopping Read (0x%02X)\n", r.rawStatus);
    }
    return r.ready();
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::writeStart(uint32_t LBA, const uint32_t COUNT)
{
    // Byte addressing for non-sdhc cards, so multiply address by 512
    if(m_type != CardType::SDHC) { LBA = LBA<<9; }

    Response1 r;

    // send pre-erase count for faster writing if we are writing multiple blocks
    if(COUNT > 1) {
        r = cardAcmd(SDCMD::ACMD23, COUNT);
        if (!r.ready()) {
            SPISD_DEBUG("ACMD23 Error! (0x02X)\n", r.rawStatus);
            return false;
        }

        r = cardCommand(SDCMD::CMD25, LBA);
    }
    else {
        r = cardCommand(SDCMD::CMD24, LBA);
    }

    if(!r.ready()) { SPISD_DEBUG("    Write Start Failed! (0x%02X)\n", r.rawStatus); }
    return r;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::writeData(const uint8_t token, const uint8_t* src)
{
    const uint16_t crc = SDPolicy::CRC_CCITT(src, 512);

    SPIShim::write(token);
    SPIShim::write(src, 512);
    SPIShim::write(crc >> 8);
    SPIShim::write(crc & 0xFF);

    const uint8_t status = SPIShim::read();
    const bool success = (status & DATA_RES_MASK) == DATA_RES_ACCEPTED;
    if (!success) {
        SPISD_DEBUG("    BLOCK WRITE ERROR! (0x%02X)\n", status);
    }
    return success;
}

template<class SPIShim, class SDPolicy, class TimeoutPolicy >
bool SpiCard<SPIShim, SDPolicy, TimeoutPolicy>::writeStop()
{
    if (!waitNotBusy(TimeoutPolicy::writeTimeout::value)) {
        SPISD_DEBUG("    Write Stop: SD card timed out as busy!\n");
        return false;
    }
    SPIShim::write(STOP_TRAN_TOKEN);
    return true;
}

}       // sd namespace
#endif  // include gaurd