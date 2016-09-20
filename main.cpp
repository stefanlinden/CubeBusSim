/*
 * main.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <stdint.h>

#include "msp.h"
#include "driverlib.h"
#include "DWire.h"
#include "max3421e.h"
#include "mcp2515.h"
#include "delay.h"
#include "simpackets.h"
#include "i2cdriver.h"
#include "candriver.h"
#include "messagequeue.h"
#include "random.h"

#define ISMASTER

/* Interfaces */
I2CInterface i2cInterface;
CANInterface canInterface;

/* Prototypes */
DataPacket * generateDataPacket( uint_fast8_t, bool );

/* Handlers */
void DataHandle( DataPacket * packet );
void HeaderHandle( HeaderPacket * packet );

/* Some standard packets */
HeaderPacket ACKPacket(0, 1);
HeaderPacket NAKPacket(0, 1);

/* Counters */

int main( void ) {
    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    i2cInterface.setDataHandler(DataHandle);
    i2cInterface.setHeaderHandler(HeaderHandle);
    canInterface.setDataHandler(DataHandle);
    canInterface.setHeaderHandler(HeaderHandle);


    /* Generate the standard packets */
    ACKPacket.setCommand(PKT_ACK, 0, 0, 0);
    ACKPacket.calculateNewCRC( );
    NAKPacket.setCommand(PKT_NAK, 0, 0, 0);
    NAKPacket.calculateNewCRC( );

#ifdef ISMASTER
    int i;

    i2cInterface.init(true, 0);

    HeaderPacket pingPkt(0, 1);
    pingPkt.setCommand(PKT_PING, 0, 0, 0);
    pingPkt.calculateNewCRC( );

    while ( 1 ) {
        i2cInterface.sendHeader(&pingPkt);
        for ( i = 0; i < 50000; i++ )
            ;
        i2cInterface.requestData(20, 1);
    }

    //MAP_PCM_gotoLPM0InterruptSafe( );
#else

    i2cInterface.init(false, 1);

    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
#endif
}

void HeaderHandle( HeaderPacket * packet ) {
    uint_fast8_t ii;

    if ( packet->checkCRC( ) ) {
        /* Error! Send NAK */
        i2cInterface.sendHeader(&NAKPacket);
    } else {
        /* First of all, send an ACK */
        i2cInterface.sendHeader(&ACKPacket);
        switch ( packet->cmd ) {
        case PKT_PING:
            /* Do nothing, only ACK */
            break;
        case PKT_DATAPULL:
            /* Generate fake data */
            for ( ii = 0; ii < packet->param[0]; ii++ )
                i2cInterface.queueData(generateDataPacket(61, true));
            break;
        }
    }
    delete packet;
}

void DataHandle( DataPacket * packet ) {
    delete packet;
}

DataPacket * generateDataPacket( uint_fast8_t length, bool doCRC ) {
    /* Generate test data using a simple PRNG */
    uint_fast8_t ii;
    DataPacket * packet;

    uint_fast8_t * data = new uint_fast8_t[length];
    for ( ii = 0; ii < length; ii++ ) {
        data[ii] = (uint_fast8_t) random( );
    }

    packet = new DataPacket(length, data);

    delete data;

    if ( doCRC )
        packet->generateCRC( );

    return packet;
}
