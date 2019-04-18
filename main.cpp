#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <array>
#include "SDCard.hpp"
#include "SDPolicies.h"

SPIDriver spiTester;
std::string SpiDriverPort;

int main(int argc, char *argv[])
{
    sd::SpiCard<SPIShim, SDCardPolicy> sdCard;
    std::array<uint8_t, 512> bufRead  = {0};
    std::array<uint8_t, 512> bufWrite = {0};

    if (argc < 2) {
        printf("Usage: spicl <PORTNAME>\n");
        exit(1);
    }

    SpiDriverPort = std::string(argv[1]);

    const bool beganWell = sdCard.begin();

    printf("SD Card was %sbegan!!! \n", beganWell ? "" : "NOT " );

    snprintf((char*)bufWrite.data(), bufWrite.size(), "Tiglax is a stinky puppy! Tiglax is a stinky puppy! Tiglax is a stinky puppy! Tiglax is a stinky puppy!");

    //sdCard.writeBlock(300, bufWrite.data());
    sdCard.readBlock(0x300, bufRead.data());
    sdCard.writeBlock(0x300, bufWrite.data());

    uint32_t i  = 0x300;
    ssize_t  rd = 0;
    bool write = false;
    do {
        rd = sdCard.readBlock(i++, bufRead.data());
        for(auto& c : bufRead) {
            if(std::isprint(c)) {
                printf("%c", c);
            }
        }
        fflush(stdout);
        fflush(stderr);
    } while(rd > 0);

    return 0;
}
