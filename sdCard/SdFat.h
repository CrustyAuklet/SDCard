#ifndef SD_FAT_FS_H
#define SD_FAT_FS_H
#include "SDCard.hpp"
#include "FatLib/FatLib.h"

#define FATFS_DEBUG(...)  do { fprintf(stderr, __VA_ARGS__); } while(0)

/**
 * \class SdBaseFile
 * \brief Class for backward compatibility.
 */
class SdBaseFile : public FatFile {
 public:
  SdBaseFile() {}
  /**  Create a file object and open it in the current working directory.
   *
   * \param[in] path A path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   * bitwise-inclusive OR of open flags. see
   * FatFile::open(FatFile*, const char*, oflag_t).
   */
  SdBaseFile(const char* path, oflag_t oflag) : FatFile(path, oflag) {}
};

/**
 * \class SdFileSystem
 * \brief Virtual base class for %SdFat library.
 */
template<class SPIShim>
class SdFileSystem : public FatFileSystem {
public:
    SdFileSystem() = default;

    /** Initialize file system.
    * \return true for success else false.
    */
    bool begin() {
        return FatFileSystem::begin(&m_card);
    }

    /** \return Pointer to SD card object */
//    SdSpiCard *card() {
//        m_card.syncBlocks();
//        return &m_card;
//    }

    /** %Print any SD error code and halt.
    *
    * \param[in] pr Print destination.
    */
//    void errorHalt() {
//        errorPrint(pr);
//    }

    /** %Print msg, any SD error code and halt.
    *
    * \param[in] msg Message to print.
    */
    void errorHalt(char const* msg) {
        fprintf(stderr, msg);
    }

    /** %Print any SD error code.
    * \param[in] pr Print device.
    */
    void errorPrint() {
        if (!cardErrorCode()) {
            return;
        }
        fprintf(stderr, "SD errorCode: 0x%X, 0x%X\n", cardErrorCode(), cardErrorData());
    }

    /** %Print msg, any SD error code.
    *
    * \param[in] pr Print destination.
    * \param[in] msg Message to print.
    */
    void errorPrint(char const* msg) {
        fprintf(stderr, "error: %s\n", msg);
    }

    /** %Print error details and halt after begin fails.
    *
    * \param[in] pr Print destination.
    */
    void initErrorHalt() {
        initErrorPrint();
    }

    /**Print message, error details, and halt after begin() fails.
    * \param[in] pr Print device.
    * \param[in] msg Message to print.
    */
    void initErrorHalt(char const *msg) {
        fprintf(stderr, msg);
        initErrorHalt();
    }

    /** Print error details after begin() fails.
    *
    * \param[in] pr Print destination.
    */
    void initErrorPrint() {
        if (cardErrorCode()) {
            fprintf(stderr, "Can't access SD card. Do not reformat.\n");
            if (cardErrorCode() == SD_CARD_ERROR_CMD0) {
                fprintf(stderr, "No card, wrong chip select pin, or SPI problem?\n");
            }
            errorPrint();
        }
        else if (vol()->fatType() == 0) {
            fprintf(stderr, "Invalid format, reformat SD.\n");
        }
        else if (!vwd()->isOpen()) {
            fprintf(stderr, "Can't open root directory.");
        }
        else {
            fprintf(stderr, "No error found.");
        }
    }

    /**Print message and error details and halt after begin() fails.
    *
    * \param[in] msg Message to print.
    */
    void initErrorPrint(char const *msg) {
        initErrorPrint(msg);
    }

    /** \return The card error code */
    uint8_t cardErrorCode() {
        return m_card.errorCode();
    }
    /** \return the card error data */
    uint32_t cardErrorData() {
        return m_card.errorData();
    }

protected:
    sd::SpiCard<SPIShim, sd::ShiftedCRC> m_card;
};

/**
 * \class SdFat
 * \brief Main file system class for %SdFat library.
 */
class SdFat : public SdFileSystem {
public:

    SdFat() = default;

    /** Initialize SD card and file system.
     * \return true for success else false.
     */
    bool begin() { return m_card.begin() && SdFileSystem::begin(); }

    /** Initialize SD card for diagnostic use only.
     * \return true for success else false.
     */
    bool cardBegin() { return m_card.begin(); }

    /** Initialize file system for diagnostic use only.
     * \return true for success else false.
     */
    bool fsBegin() { return FatFileSystem::begin(card()); }
};

#endif  // sdfat header guard