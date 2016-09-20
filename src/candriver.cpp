/*
 * candriver.cpp
 *
 *  Created on: 20 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <mcp2515.h>
#include "candriver.h"
#include "simpackets.h"
#include "addresstable.h"
#include "CubeBusSim.h"

/* SPI Timing Config */
const MCP_CANTimingConfig CANTimingConfig = { 20000000, /* Oscillator Frequency */
8, /* Baud Rate Prescaler */
1, /* Propagation Delay */
3, /* Phase Segment 1 */
3, /* Phase Segment 2 */
1 /* Synchronisation Jump Width */
};

void msgHandler( MCP_CANMessage * msg );

void (*can_HeaderHandler)( HeaderPacket * );
void (*can_DataHandler)( DataPacket * );

uint_fast8_t * dataBuffer;
uint_fast8_t dataRXCount;

uint_fast8_t CANInterface::init( bool asMaster, uint_fast8_t ownAddress ) {
    uint_fast8_t canAddress;
    isMaster = asMaster;

    dataBuffer = 0;
    dataRXCount = 0;

    MCP_init( );

    MCP_reset( );

    while ( (MCP_readRegister(RCANSTAT) >> 5) != MODE_CONFIG )
        ;

    MCP_setTiming(&CANTimingConfig);

    /* Register an interrupt on TX0 and RX0 */
    /* These interrupts are handled internally, but kept externally for control */
    MCP_enableInterrupt(
            MCP_ISR_RX0IE | MCP_ISR_RX1IE | MCP_ISR_TX0IE | MCP_ISR_TX1IE
                    | MCP_ISR_TX2IE | MCP_ISR_ERRIE);

    /* Set the handler to be called when a message is received */
    MCP_setReceivedMessageHandler(&msgHandler);

    /* CAN is a multi-master system, so there is no big difference between the OBC and
     * other 'slave'-subsystems, except for filtering of messages */
    if ( asMaster ) {
        /* This is the simple case: disable all filters to receive all messages */
        /* The 'BUKT' bit is also set, enabling rollover of messages in case the RXB0
         * is unavailable */
        MCP_writeRegister(RRXB0CTRL, 0x64);
        MCP_writeRegister(RRXB1CTRL, 0x60);
    } else {
        MCP_writeRegister(RRXB0CTRL, 0x24);
        canAddress = getCANAddress(ownAddress);

        /* Enable the full masks for standard identifiers */
        MCP_writeRegister(RRXM0SIDH, 0xFF);
        MCP_writeRegister(RRXM0SIDL, 0xE0);

        MCP_writeRegister(RRXF0SIDL, canAddress << 5);
        MCP_writeRegister(RRXF0SIDH, (canAddress & 0xFF) >> 3);
    }
    /* Go into NORMAL mode */
    MCP_setMode(MODE_NORMAL);
    return 0;
}

uint_fast8_t CANInterface::sendHeader( HeaderPacket * packet ) {
    MCP_CANMessage canMsg;

    packet->calculateNewCRC( );

    canMsg.ID = getCANAddress(packet->targetNode);
    canMsg.data = packet->getRawData( );
    canMsg.length = 8;
    canMsg.isExtended = 0;
    canMsg.isRequest = 0;

    if ( MCP_sendMessage(&canMsg) )
        return ERR_TIMEOUT;
    else
        return 0;
}

uint_fast8_t CANInterface::requestData( uint_fast8_t, uint_fast8_t ) {
    return 0;
}

void msgHandler( MCP_CANMessage * msg ) {
    uint_fast8_t k, ii, lim;

    if ( msg->data[0] == PKT_DATA ) {
        /* Data packets are split up over CAN, so we need to merge them back together */
        if ( !dataBuffer ) {
            dataBuffer = new uint_fast8_t[63];
        }

        lim = dataRXCount * 7;
        k = 1;
        /* Copy the data */
        for(ii = lim; ii < lim + 7; ii++) {
            dataBuffer[ii] = msg->data[k];
            k++;
        }
        dataRXCount++;
        if(dataRXCount == 7) {
            /* We have just received the final part of this packet */
            DataPacket * dataPkt = new DataPacket(61, dataBuffer);
            dataPkt->crc =  ((uint_fast16_t) dataBuffer[61] << 8) | (dataBuffer[62] & 0xFF);

            delete[] dataBuffer;
            dataBuffer = 0;
            can_DataHandler(dataPkt);
        }
    } else {
        can_HeaderHandler(new HeaderPacket(msg->data));
    }
}

void CANInterface::queueData(DataPacket * packet) {
    dataQueue.add(packet);
    // Trigger queue
}

/* The set*Handler methods are overridden here to create a global handle */
void CANInterface::setHeaderHandler( void (*handler)( HeaderPacket * ) ) {
    HeaderHandler = handler;
    can_HeaderHandler = handler;
}

void CANInterface::setDataHandler( void (*handler)( DataPacket * ) ) {
    DataHandler = handler;
    can_DataHandler = handler;
}
