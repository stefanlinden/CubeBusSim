/*
 * packets.cpp
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "CubeBusSim.h"
#include "simpackets.h"
#include "crc.h"


HeaderPacket::HeaderPacket( uint_fast8_t origin, uint_fast8_t target ) {
    this->cmd = 0;
    this->crc = 0;
    this->originNode = origin;
    this->targetNode = target;
    this->param = new uint_fast8_t[3];
    this->rawData = 0;
}

HeaderPacket::HeaderPacket( uint_fast8_t * rawPacket ) {
    uint_fast8_t ii;
    this->param = new uint_fast8_t[3];

    this->rawData = new uint_fast8_t[8];
    for ( ii = 0; ii < 8; ii++ )
        rawData[ii] = rawPacket[ii];

    this->cmd = rawPacket[0];
    this->targetNode = rawPacket[1];
    this->originNode = rawPacket[2];
    this->param[0] = rawPacket[3];
    this->param[1] = rawPacket[4];
    this->param[2] = rawPacket[5];
    this->crc = ((uint_fast16_t) rawPacket[6] << 8) | rawPacket[7];
}

HeaderPacket::HeaderPacket(HeaderPacket * orig) {
    uint_fast8_t ii;

    this->cmd = orig->cmd;
    this->crc = orig->crc;
    this->originNode = orig->originNode;
    this->targetNode = orig->targetNode;

    this->param = new uint_fast8_t[3];
    this->rawData = new uint_fast8_t[8];

    for(ii = 0; ii < 3; ii++)
        this->param[ii] = orig->param[ii];

    for(ii = 0; ii < 8; ii++)
        this->rawData[ii] = orig->rawData[ii];
}

HeaderPacket::~HeaderPacket( ) {
    delete[] param;

    if(rawData)
        delete[] rawData;
}


/* Other methods */
void HeaderPacket::setCommand( uint_fast8_t cmd, uint_fast8_t param0,
        uint_fast8_t param1, uint_fast8_t param2 ) {
    if ( rawData )
        clean( );
    this->cmd = cmd;
    param[0] = param0;
    param[1] = param1;
    param[2] = param2;
}

void HeaderPacket::clean( void ) {
    if ( rawData ) {
        delete[] rawData;
    }
    delete[] param;
    HeaderPacket(originNode, targetNode);
}

uint_fast8_t * HeaderPacket::getRawData( void ) {
    if ( !rawData || !crc )
        calculateNewCRC( );

    return rawData;
}

void HeaderPacket::makeRawData( void ) {
    if ( rawData )
        delete[] rawData;
    rawData = new uint_fast8_t[8];

    rawData[0] = cmd;
    rawData[1] = targetNode;
    rawData[2] = originNode;
    rawData[3] = param[0];
    rawData[4] = param[1];
    rawData[5] = param[2];
    rawData[6] = ((uint_fast8_t) crc) >> 8;
    rawData[7] = (uint_fast8_t) crc;
}

void HeaderPacket::calculateNewCRC( void ) {
    if ( !rawData )
        makeRawData( );
    crc = getCRC(rawData, 6);
    rawData[6] = ((uint_fast8_t) crc) >> 8;
    rawData[7] = (uint_fast8_t) crc & 0xFF;
}

uint_fast8_t HeaderPacket::checkCRC( void ) {
    if ( !rawData )
        return ERR_NODATASET;

    if ( getCRC(rawData, 6) == this->crc )
        return 0;
    else
        return ERR_CRC;
}

/**** DATA Packet ****/

DataPacket::DataPacket( uint_fast8_t length, uint_fast8_t * data ) {
    uint_fast8_t ii;

    this->length = length;
    this->data = new uint_fast8_t[length];

    /* Copy the array (memcpy and/or similar functions appear to be unsupported) */
    for ( ii = 0; ii < length; ii++ ) {
        this->data[ii] = data[ii];
    }
}

DataPacket::~DataPacket( void ) {
    delete[] this->data;
}

void DataPacket::generateCRC( void ) {
    this->crc = getCRC(data, length);
}

bool DataPacket::checkCRC( void ) {
    if(getCRC(data, length) == this->crc)
        return 0;
    else
        return ERR_CRC;
}
