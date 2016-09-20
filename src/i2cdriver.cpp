/*
 * i2cdriver.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "i2cdriver.h"
#include "businterface.h"
#include "CubeBusSim.h"
#include <DWire.h>
#include "addresstable.h"

DWire wire = DWire( );
uint_fast8_t i2c_rxBuffer[64];
uint_fast8_t i2c_mode;
void (*i2c_HeaderHandler)( HeaderPacket * );
void (*i2c_DataHandler)( DataPacket * );

HeaderPacket * headerBuffer;
DataPacket * dataBuffer;
DataQueue * queue;

/* Interrupt handlers */
void handleReceive( uint8_t );
void handleRequest( void );

/* Class method definitions */
uint_fast8_t I2CInterface::init( bool asMaster, uint_fast8_t ownAddress ) {

    this->isMaster = asMaster;
    this->ownAddress = ownAddress;
    wire.setFastMode( );

    headerBuffer = 0;
    dataBuffer = 0;

    wire.onReceive(handleReceive);
    wire.onRequest(handleRequest);

    queue = &dataQueue;

    if ( asMaster )
        wire.begin( );
    else
        wire.begin(getI2CAddress(ownAddress));
    return 0;
}

uint_fast8_t I2CInterface::sendHeader( HeaderPacket * header ) {
    uint_fast8_t ii, address;
    uint_fast8_t * dataPtr;

    if ( isMaster ) {
        header->calculateNewCRC( );
        address = getI2CAddress(header->targetNode);
        wire.beginTransmission(address);

        dataPtr = header->getRawData( );

        for ( ii = 0; ii < 8; ii++ ) {
            wire.write(dataPtr[ii]);
        }

        wire.endTransmission(true);

        if ( !wire.requestFrom(address, 8) )
            return ERR_TIMEOUT;

        for ( ii = 0; ii < 8; ii++ )
            i2c_rxBuffer[ii] = (uint_fast8_t) wire.read( );

        if ( i2c_rxBuffer[0] == PKT_ACK )
            return 0;
        else if ( i2c_rxBuffer[0] == PKT_NAK )
            return ERR_NAK;
        else
            return ERR_UNEXPECTED;
    } else {
        /* When we're a slave, then we have to wait until the master requests data */
        /* Load the data into the buffer */
        headerBuffer = new HeaderPacket(header);
        return 0;
    }
}

uint_fast8_t I2CInterface::requestData( uint_fast8_t howMuch, uint_fast8_t address ) {
    /* Request data packages. One package is 64 bytes in total (including CRC and command byte */
    uint_fast8_t ii, res, readii;
    uint_fast8_t * dataBuffer;
    DataPacket * dataPkt;
    bool crcerror;

    crcerror = false;

    HeaderPacket * pullRequest = new HeaderPacket(0, 1);
    pullRequest->setCommand(PKT_DATAPULL, howMuch, 0, 0);
    pullRequest->calculateNewCRC( );

    /* Send the request header */
    res = sendHeader(pullRequest);
    delete pullRequest;
    if(res != 0)
        return res;

    /* Load the messages */
    for ( ii = 0; ii < howMuch; ii++ ) {
        res = wire.requestFrom(getI2CAddress(address), 64);
        if(res == 0)
            return ERR_NAK;
        dataBuffer = new uint_fast8_t[64];
        for(readii = 0; readii < 64; readii++)
            dataBuffer[readii] = wire.read();

        /* Create a packet */
        dataPkt = new DataPacket(61, &dataBuffer[1]);

        /* Load and check the CRC */
        dataPkt->crc = ((uint_fast16_t) dataBuffer[62] << 8) | (dataBuffer[63] & 0xFF);
        delete dataBuffer;
        if(dataPkt->checkCRC())
            crcerror = true;

        delete dataPkt;
    }

    if(crcerror)
        return ERR_CRC;
    else
        return 0;
}

/* The set*Handler methods are overridden here to create a global handle */
void I2CInterface::setHeaderHandler( void (*handler)( HeaderPacket * ) ) {
    HeaderHandler = handler;
    i2c_HeaderHandler = handler;
}

void I2CInterface::setDataHandler( void (*handler)( DataPacket * ) ) {
    DataHandler = handler;
    i2c_DataHandler = handler;
}

/**
 * Receive interrupt handler
 * This interrupt is triggered by DWire when a full frame has been received
 * (i.e. after receiving a STOP)
 */
void handleReceive( uint8_t numBytes ) {
    uint_fast8_t ii;

    for ( ii = 0; ii < numBytes; ii++ )
        i2c_rxBuffer[ii] = wire.read( );

    if ( i2c_rxBuffer[0] == PKT_DATA )
        i2c_DataHandler(new DataPacket(numBytes, &i2c_rxBuffer[0]));
    else
        i2c_HeaderHandler(new HeaderPacket(&i2c_rxBuffer[0]));
}

/**
 * Request interrupt handler
 * This request is called on a read request from a master node.
 *
 */
void handleRequest( void ) {
    uint_fast8_t * rawData;
    uint_fast8_t ii;

    if ( headerBuffer ) {
        headerBuffer->calculateNewCRC( );
        rawData = headerBuffer->getRawData( );
        for ( ii = 0; ii < 8; ii++ )
            wire.write(rawData[ii]);

        delete headerBuffer;
        headerBuffer = 0;

    } else if ( queue->getLength( ) ) {
        /* Send data */
        DataPacket * packet = queue->pop( );
        wire.write(PKT_DATA);
        for ( ii = 0; ii < packet->length; ii++ )
            wire.write(packet->data[ii]);

        /* Add the CRC */
        wire.write((((uint_fast8_t) packet->crc) >> 8) & 0xFF);
        wire.write(((uint_fast8_t) packet->crc) & 0xFF);

        delete packet;
    }
}
