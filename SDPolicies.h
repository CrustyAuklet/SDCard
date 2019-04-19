//
// Created by Ethan on 07-Apr-19.
//

#ifndef SDCARD_SDPOLICIES_H
#define SDCARD_SDPOLICIES_H

#include <type_traits>
#include <cstdio>
#include <string>
#include <chrono>

extern "C" {
#include "spiDriver/spidriver.h"
}

extern SPIDriver spiTester;
extern std::string SpiDriverPort;

struct SPIShim {
    bool active() { return spiTester.connected > 0; }
    bool begin() { spi_connect(&spiTester, SpiDriverPort.c_str()); return active(); }
    void select() { spi_sel(&spiTester); }
    void deSelect() { spi_unsel(&spiTester); }
    void write(const uint8_t* buf, const size_t LEN) { spi_write(&spiTester, (const char*)buf, LEN); }
    uint8_t write(uint8_t val) { return read(val); }
    void read(uint8_t* buf, const size_t LEN) { spi_read(&spiTester, (char*)buf, LEN); }
    uint8_t read(uint8_t val = 0xFF) { spi_writeread(&spiTester, (char*)(&val), 1); return val; }
};

#endif //SDCARD_SDPOLICIES_H
