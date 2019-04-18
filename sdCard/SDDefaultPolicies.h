//
// Created by Ethan on 13-Apr-19.
//

#ifndef SDCARD_SDDEFAULTPOLICIES_H
#define SDCARD_SDDEFAULTPOLICIES_H

#include <chrono>

namespace sd {

    struct ShiftedCRC {
        /// CRC7 implementation
        static constexpr bool useCRC7 = false;

        uint8_t getCRC7(const uint8_t *data, const uint8_t n) {
            uint8_t crc = 0;
            for (uint8_t i = 0; i < n; i++) {
                uint8_t d = data[i];
                for (uint8_t j = 0; j < 8; j++) {
                    crc <<= 1;
                    if ((d & 0x80) ^ (crc & 0x80)) {
                        crc ^= 0x09;
                    }
                    d <<= 1;
                }
            }
            return (crc << 1) | uint8_t(1);
        }

        /// CRC16 implementation
        static constexpr bool useCRC16 = true;

        uint16_t CRC_CCITT(const uint8_t *data, const size_t n) {
            uint16_t crc = 0;
            for (size_t i = 0; i < n; i++) {
                crc = (uint8_t)(crc >> 8) | (crc << 8);
                crc ^= data[i];
                crc ^= (uint8_t)(crc & 0xff) >> 4;
                crc ^= crc << 12;
                crc ^= (crc & 0xff) << 5;
            }
            return crc;
        }

        // timout values and time types
        using timeType = std::chrono::milliseconds::rep;

        timeType getTime() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
        }

        bool isTimedOut(const timeType t0, const uint32_t Timeout) { return (getTime() - t0) > Timeout; }

        using cmd0_retry   = std::integral_constant<uint8_t, 10>;
        using cmdTimeout   = std::integral_constant<uint32_t, 300>;
        using initTimeout  = std::integral_constant<uint32_t, 2000>;
        using eraseTimeout = std::integral_constant<uint32_t, 10000>;
        using readTimeout  = std::integral_constant<uint32_t, 1000>;
        using writeTimeout = std::integral_constant<uint32_t, 2000>;
    };

} // sd namespace

#endif //SDCARD_SDDEFAULTPOLICIES_H
