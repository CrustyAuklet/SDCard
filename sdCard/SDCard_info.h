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

/** start data token for read or write single block*/
constexpr uint8_t DATA_START_BLOCK = 0xFE;
/** stop token for write multiple blocks*/
constexpr uint8_t STOP_TRAN_TOKEN = 0xFD;
/** start data token for write multiple blocks*/
constexpr uint8_t WRITE_MULTIPLE_TOKEN = 0xFC;
/** mask for data response tokens after a write block operation */
constexpr uint8_t DATA_RES_MASK = 0x1F;
/** write data accepted token */
constexpr uint8_t DATA_RES_ACCEPTED = 0x05;

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

static_assert(sizeof(CardStatus) == 4, "CardStatus response must be 4 bytes!");

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

static_assert(sizeof(Response1) == 1, "Response1 must be 1 byte!");

struct OCR {
    constexpr bool pwrUpStatus() const      { return raw[0] & (1UL<<7); }
    constexpr bool ccs() const              { return raw[0] & (1UL<<6); }
    constexpr bool uhs2() const             { return raw[0] & (1UL<<5); }
    constexpr bool canSwitch1v8() const     { return raw[0] & (1UL<<0); }
    constexpr uint16_t vRange() const       { return ( (((uint16_t)raw[1])<<1) | (raw[2]>>7) ); }

    std::array<uint8_t, 4> raw; //< raw OCR status
};

static_assert(sizeof(OCR) == 4, "OCR response must be 4 bytes!");

/**
 * @brief Card IDentification (CID) register.
 */
struct CID {
    /// Manufacturer ID
    constexpr uint8_t mid() const { return raw[0]; }
    /// OEM / Application ID
    constexpr uint8_t oid() const { return (raw[1]<<8) | raw[2]; }
    /// product name (use array_view if we add GSL as dependancy or get C++20?)
    std::array<uint8_t, 5> productName() const {
        return std::array<uint8_t, 5>{ raw[3], raw[4], raw[5], raw[6], raw[7] };
    }

    /// get the product revision as floating point number
    constexpr float ProductRevision() const { return ProductRev_Major() + (ProductRev_Minor()/10); }
    /// get product revision major number
    constexpr unsigned ProductRev_Major() const { return ((raw[8]&0xF0)>>4); }
    /// get product revision minor number
    constexpr unsigned ProductRev_Minor() const { return (raw[8]&0x0F); }

    /// product name (use array_view if we add GSL as dependancy or get C++20?)
    constexpr uint32_t SerialNumber() const {
        return (raw[9]<<24) | (raw[12]<<16) | (raw[13]<<8) | (raw[14]);
    }

    /// Manufacture Month
    constexpr unsigned ManufMonth() const { return (raw[13] & 0x0F) ;} // (manufDate & 0x0F00)>>8
    /// Manufacture Year
    constexpr unsigned ManufYear() const { return raw[14]+ 2000; }
    /// Get the CRC value
    constexpr uint8_t getCRC7() const { return (raw[15]&0xFE)>>1; }
    /// check the CRC7 value against provided value
    constexpr bool checkCRC7(const uint8_t refCrc7) const { return getCRC7() == refCrc7; }

    // manufacturer ID       [0]
    // OEM/Application ID    [1:2]
    // Product Name          [3:7]
    // product revision      [8]
    // Product Serial Number [9:12]
    // manufacturing Date    [13:14]
    // CRC7 checksum         [15]
    std::array<uint8_t, 16> raw;
};

static_assert(sizeof(CID) == 16, "CID response must be 16 bytes!");

struct CSD {
    /// check the version of the CSD structure. TRUE if it is a v2 CSD, FALSE if it is a v1 CSD
    constexpr bool csdv2() const { return raw[0]&0xC0; }
    /// defines the asynchronous part of the data access time
    constexpr uint8_t taac() const { return raw[1]; }
    /// Defines the worst case for the clock-dependant factor of the data access time
    constexpr uint8_t nsac() const { return raw[2]; }
    /// defines the max transfer rate for one data line according to table in the specification
    constexpr uint8_t transferSpeed() const { return raw[3]; }
    /// defines the compatable SD classes. Each bit represents a class. see the specification.
    constexpr uint_least16_t ccc() const { return (((uint16_t)raw[4])<<4) | ((raw[5]&0xF0)>>4); }

    /// the maxiumum read data block length. In set { 512, 1024, 2048 }
    constexpr uint_least16_t readBlockLength() const { return 1<<(raw[5] & 0x0F); }
    /// partial block read is available (ALWAYS TRUE in SD card)
    constexpr bool readBlockPartial() const { return raw[6] & 0x80; }
    /// TRUE if the data block to be written by one command can be spread over more than one physical block of the memory device
    constexpr bool writeBlockMisaligned() const { return raw[6] & 0x40; }
    /// TRUE if the data block to be read by one command can be spread over more than one physical block of the memory device
    constexpr bool readBlockMisaligned() const { return raw[6] & 0x20; }
    /// TRUE if the configurable driver stage is integrated on the card.
    constexpr bool DSRImplemented() const { return raw[6] & 0x10; }

    /// the device size (as a raw value) used in conjunction with cSizeMulti to get the block count.
    uint32_t cSize() const {
        if(csdv2()) { // CSD version 2
            return (((uint32_t)(raw[7]&0x3F))<<16) | (((uint32_t)(raw[8]))<<8) | (raw[9]);
        }
        else { // CSD version 1
            return (((uint32_t)(raw[6]&0x03))<<10) | ((raw[7])<<2) | ((raw[8])>>6);
        }
    }

    /// the size multiplyer, used to find the number of blocks on the device
    uint_least16_t cSizeMult() const {
        const uint16_t exponent = ((raw[9]&0x03)<<1) | ((raw[10]&0x80)>>7);
        return 1 << (exponent+2);
    }

    /// returns the number of blocks on the device. The size can be found by multiplying this by readBlockLenth()
    uint32_t blockCount() const {
        if(csdv2()) {
            return (cSize() + 1) << 10;
        }
        else {
            return (cSize() + 1) * cSizeMult();
        }
    }

    /// returns the card capacity in bytes
    uint32_t cardCapacity() const {
        return blockCount() * readBlockLength();
    }

    /// TRUE if device can erase at the block level. FALSE if erasure must be at the sector level
    constexpr bool eraseBlockEnabled() const { return raw[10] & 0x40; }
    /// the number of write blocks (WRITE_BL_LEN) that make up an erasable sector
    constexpr uint8_t sectorSize() const { return (((raw[10]&0x3F)<<1) | ((raw[11]&0x80)>>7)) + 1; }
    /// The size of a write protected group in sectors.
    constexpr uint8_t wpGroupSize() const { return (raw[11] & 0x7F)+1; }
    /// FALSE if no group write protection possible
    constexpr bool wpGroupEnable() const { return raw[12] & 0x80; }
    /// the typical block program time as a multiple of the read access time. power of two between 1 and 32.
    constexpr uint8_t r2wFactor() const { return 1<<((raw[12] & 0x1C)>>2); }

    /// the maxiumum write data block length. In set { 512, 1024, 2048 } and should match read block length
    constexpr uint8_t writeBlockLength() const { return 1 << (((raw[12]&0x03)<<2) | ((raw[13])&0xC0)>>6); }

    /// TRUE if partial block sizes can be used in block write commands.
    /// FALSE if only WRITE_BL_LEN and partial derivatives, in resolution of 512 bytes, can be used for writing
    constexpr bool writeBlockPartial() const { return raw[13]&0x20; }
    /// Indicates the selected group of file formats according to the specification
    constexpr bool fileFormatGroup() const { return raw[14] & 0x80; }
    /// TRUE if the contents have been copied (OTP products sold to consumers)
    constexpr bool copy() const { return raw[14] & 0x40; }
    /// TRUE if all write and erase commands are permanently disabled
    constexpr bool permWriteProtect() const { return raw[14] & 0x20; }
    /// TRUE if all write and erase commands are temporarily disabled
    constexpr bool tempWriteProtect() const { return raw[14] & 0x10; }
    /// Indicates the file format on the card according to table 5-15 in the SD specification
    constexpr bool fileFormat() const { return raw[14] & 0x08; }
    /// CRC for the contents according to chapter 4.5 in the SD specification
    constexpr uint8_t crc7() const { return raw[15]>>1; }

    std::array<uint8_t, 16> raw;    //< raw data of the CSD register
};
static_assert(sizeof(CSD) == 16, "CSD response must be 16 bytes!");

}   // namespace sd

#endif  // Header guard
