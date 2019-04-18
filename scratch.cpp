//
// Created by Ethan on 13-Apr-19.
//

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