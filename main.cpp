/*
 * main.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <stdio.h>

#include "msp.h"
#include "driverlib.h"
#include "DWire.h"
#include "max3421e.h"
#include "mcp2515.h"
#include "delay.h"
#include "simpackets.h"
#include "i2cdriver.h"

#define ISMASTER

I2CInterface i2cInterface;

void DataHandle( DataPacket * packet );
void HeaderHandle( HeaderPacket * packet );

int main( void ) {
    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    i2cInterface.setDataHandler(DataHandle);
    i2cInterface.setHeaderHandler(HeaderHandle);

#ifdef ISMASTER
    int i;

    i2cInterface.init(true, 0);

    HeaderPacket * pingPkt = new HeaderPacket(0, 1);
    pingPkt->setCommand(PKT_PING, 0, 0, 0);
    pingPkt->calculateNewCRC( );

    while ( 1 ) {
        i2cInterface.sendHeader(pingPkt);
        for(i = 0; i < 50000; i++);
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
delete packet;
}

void DataHandle( DataPacket * packet ) {
delete packet;
}
