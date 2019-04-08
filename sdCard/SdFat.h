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
#ifndef SdFat_h
#define SdFat_h
#define ENABLE_ARDUINO_FEATURES 1
#include "SdCard_spi.h"
#include "FatLib/FatLib.h"
#include "HardwareVariant.h"
//------------------------------------------------------------------------------
/** SdFat version 1.1.0 */
#define SD_FAT_VERSION 10100
//==============================================================================
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
//-----------------------------------------------------------------------------
#if ENABLE_ARDUINO_FEATURES
/**
 * \class SdFile
 * \brief Class for backward compatibility.
 */
class SdFile : public PrintFile {
 public:
  SdFile() {}
  /**  Create a file object and open it in the current working directory.
   *
   * \param[in] path A path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   * bitwise-inclusive OR of open flags. see
   * FatFile::open(FatFile*, const char*, oflag_t).
   */
  SdFile(const char* path, oflag_t oflag) : PrintFile(path, oflag) {}
};
#endif  // #if ENABLE_ARDUINO_FEATURES
//-----------------------------------------------------------------------------
/**
 * \class SdFileSystem
 * \brief Virtual base class for %SdFat library.
 */
class SdFileSystem : public FatFileSystem {
 public:
    SdFileSystem(SPIClass& spi, port::PinType_t csPin) : m_card(spi, csPin) {}

  /** Initialize file system.
   * \return true for success else false.
   */
  bool begin() {
    return FatFileSystem::begin(&m_card);
  }
  /** \return Pointer to SD card object */
  SdSpiCard *card() {
    m_card.syncBlocks();
    return &m_card;
  }

  /** %Print any SD error code and halt.
   *
   * \param[in] pr Print destination.
   */
//   void errorHalt() {
//     errorPrint(pr);
//   }

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
  SdSpiCard m_card;
};
//==============================================================================

/**
 * \class SdFat
 * \brief Main file system class for %SdFat library.
 */
class SdFat : public SdFileSystem {
public:

    SdFat(SPIClass& spi, port::PinType_t csPin) : SdFileSystem(spi, csPin) { }

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

private:
    // SdFatSpiDriver m_spi;
};

#endif  // sdfat header guard