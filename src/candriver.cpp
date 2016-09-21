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
void TxBufferAvailable( void );

/* Using some globals for storing the top level handlers
 * This shouldn't be a problem because there is only one instance of the CANInterface */
void (*can_HeaderHandler)( HeaderPacket * );
void (*can_DataHandler)( DataPacket * );
CANInterface * canInstance;

uint_fast8_t * rxDataBuffer;
uint_fast8_t * txDataBuffer;
volatile uint_fast8_t dataRXCount, dataTXCount, debugger;

uint_fast8_t CANInterface::init( bool asMaster, uint_fast8_t ownAddress ) {
    uint_fast8_t canAddress;
    isMaster = asMaster;

    rxDataBuffer = 0;
    txDataBuffer = 0;
    dataRXCount = 0;
    dataTXCount = 0;
    debugger = 0;

    canInstance = this;

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

    /* And a second one to notify when a buffer is available (for message queueing) */
    MCP_setBufferAvailableCallback(TxBufferAvailable);

    /* CAN is a multi-master system, so there is no big difference between the OBC and
     * other 'slave'-subsystems, except for filtering of messages */
    if ( asMaster ) {
        /* This is the simple case: disable all filters to receive all messages */
        /* The 'BUKT' bit is also set, enabling rollover of messages in case the RXB0
         * is unavailable */
        MCP_writeRegister(RRXB0CTRL, 0x64);
        MCP_writeRegister(RRXB1CTRL, 0x60);
    } else {
        /* The slightly more complicated case requires the setting of filters */
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

    lastStatus = 0;

    if ( MCP_sendMessage(&canMsg) )
        return ERR_TIMEOUT;

    return 0;
}

uint_fast8_t CANInterface::requestData( uint_fast8_t howMuch,
        uint_fast8_t address ) {
    /* Request data packages. One package is 64 bytes in total (including CRC and command byte */
    uint_fast8_t res;

    HeaderPacket * pullRequest = new HeaderPacket(0, 1);
    pullRequest->setCommand(PKT_DATAPULL, howMuch, 0, 0);
    pullRequest->calculateNewCRC( );

    /* Send the request header */
    lastStatus = 0;
    res = sendHeader(pullRequest);



    delete pullRequest;
    if ( res != 0 )
        return ERR_TIMEOUT;

    while(lastStatus != 0); //TODO timeout
    /*if(lastStatus == PKT_ACK)
        uint_fast8_t t = 0;*/

    return 0;
}

void msgHandler( MCP_CANMessage * msg ) {
    uint_fast8_t k, ii, lim;

    if ( msg->data[0] == PKT_DATA ) {
        /* Data packets are split up over CAN, so we need to merge them back together */
        if ( !rxDataBuffer ) {
            rxDataBuffer = new uint_fast8_t[63];
            dataRXCount = 0;
        }

        lim = dataRXCount * 7;
        k = 1;
        /* Copy the data */
        for ( ii = lim; ii < lim + 7; ii++ ) {
            rxDataBuffer[ii] = msg->data[k];
            k++;
        }
        dataRXCount++;
        if ( dataRXCount == 7 ) {
            /* We have just received the final part of this packet */
            DataPacket * dataPkt = new DataPacket(61, rxDataBuffer);
            dataPkt->crc = ((uint_fast16_t) rxDataBuffer[61] << 8)
                    | (rxDataBuffer[62] & 0xFF);

            delete[] rxDataBuffer;

            rxDataBuffer = 0;
            can_DataHandler(dataPkt);
        }
    } else if ( msg->data[0] == PKT_ACK ) {
        canInstance->lastStatus = PKT_ACK;
    } else if ( msg->data[0] == PKT_NAK ) {
        canInstance->lastStatus = PKT_NAK;
    } else {
        can_HeaderHandler(new HeaderPacket(msg->data));
    }
}

void CANInterface::queueData( DataPacket * packet ) {
    dataQueue.add(packet);
    sendData( );
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

uint_fast8_t CANInterface::sendData( void ) {
    MCP_CANMessage msg;
    DataPacket * packet;
    uint_fast8_t res, ii, start;
    if ( !txDataBuffer && dataQueue.getLength( ) ) {
        /* If there is nothing being sent AND there is a message in the queue, then start
         * sending it */
        packet = dataQueue.pop( );
        txDataBuffer = new uint_fast8_t[63];
        for ( ii = 0; ii < 61; ii++ )
            txDataBuffer[ii] = packet->data[ii];
        //std::memcpy(txDataBuffer, packet->data,
        //        packet->length * sizeof(uint_fast8_t));
        txDataBuffer[61] = ((uint_fast8_t) packet->crc) >> 8;
        txDataBuffer[62] = (uint_fast8_t) (packet->crc & 0xFF);
        dataTXCount = 0;
        delete packet;
    } else if ( !txDataBuffer && !dataQueue.getLength( ) )
        return 0;

    /* Populate the message */
    msg.ID = 10;
    msg.isExtended = 0;
    msg.isRequest = 0;
    msg.length = 8;

    msg.data = new uint_fast8_t[8];

    //msg.data[1] = txDataBuffer[dataTXCount * 7];
    msg.data[0] = PKT_DATA;

    start = dataTXCount * 7;
    for ( ii = 0; ii < 7; ii++ )
        msg.data[ii + 1] = txDataBuffer[start + ii];

    /* Send the actual message */
    res = MCP_sendMessage(&msg);
    if ( res ) {
        return ERR_TIMEOUT;
    }

    delete[] msg.data;

    //uint_fast8_t regval = 1;
    /*while(regval) {
     regval = MCP_readRegister(RTXB0CTRL);
     }*/
    //regval = MCP_readRegister(RCANINTF);
    dataTXCount++;

    if ( dataTXCount == 9 ) {
        delete[] txDataBuffer;
        txDataBuffer = 0;
        dataTXCount = 0;
    }
    return 0;
}

void TxBufferAvailable( void ) {
    if ( !canInstance->isMaster )
        canInstance->sendData( );
}
