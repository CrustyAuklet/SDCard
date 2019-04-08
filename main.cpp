#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <array>
#include "SDCard.hpp"
#include "SDPolicies.h"

SPIDriver spiTester;
std::string SpiDriverPort;

uint8_t cmd0[6] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
uint8_t cmd1[6] = { 0x41, 0x00, 0x00, 0x00, 0x00, 0xFF };

int main(int argc, char *argv[])
{
    sd::SpiCard<SPIShim, SDCardPolicy> sdCard;
    std::array<uint8_t, 512> buf = {0};

    if (argc < 2) {
        printf("Usage: spicl <PORTNAME>\n");
        exit(1);
    }

    SpiDriverPort = std::string(argv[1]);

    const bool beganWell = sdCard.begin();

    printf("SD Card was %sbegan!!! \n", beganWell ? "" : "NOT " );

    uint32_t i  = 0;
    ssize_t  rd = 0;
    do {
        rd = sdCard.readBlock(i++, buf.data());
        for(auto& c : buf) {
            if(std::isprint(c)) {
                printf("%c", c);
            }
        }
    } while(rd > 0);

    return 0;
}
