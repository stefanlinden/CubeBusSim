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
#include "messagequeue.h"

//#define ISMASTER

/* Interfaces */
I2CInterface i2cInterface;

/* Handlers */
void DataHandle( DataPacket * packet );
void HeaderHandle( HeaderPacket * packet );

/* Message Queue */
DataItem * dataQFirst;
DataItem * dataQLast;
TXItem * TXFirst;
TXItem * TXLast;
uint32_t dataQSize, TXSize;

/* Some standard packets */
HeaderPacket ACKPacket(0, 1);
HeaderPacket NAKPacket(0, 1);


int main( void ) {
    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    dataQFirst = 0;
    dataQLast = 0;
    dataQSize = 0;

    TXFirst = 0;
    TXLast = 0;
    TXSize = 0;

    i2cInterface.setDataHandler(DataHandle);
    i2cInterface.setHeaderHandler(HeaderHandle);

    /* Generate the standard packets */
    ACKPacket.setCommand(PKT_ACK, 0, 0, 0);
    ACKPacket.calculateNewCRC();
    NAKPacket.setCommand(PKT_NAK, 0, 0, 0);
    NAKPacket.calculateNewCRC();

#ifdef ISMASTER
    int i;

    i2cInterface.init(true, 0);

    HeaderPacket * pingPkt = new HeaderPacket(0, 1);
    pingPkt->setCommand(PKT_PING, 0, 0, 0);
    pingPkt->calculateNewCRC( );

    while ( 1 ) {

    }

}*/
i2cInterface.sendHeader(pingPkt);
for ( i = 0; i < 50000; i++ )
;
}

 //MAP_PCM_gotoLPM0InterruptSafe( );
#else
    i2cInterface.init(false, 1);

    DataItem * dItem;

    while ( 1 ) {

        while ( dataQSize ) {
            /* Handle a  data packet */
            delete dataQFirst->packet;
            if ( dataQFirst->nextItem ) {
                dItem = dataQFirst;
                dataQFirst = dItem->nextItem;
                dataQSize--;
                delete dItem;
            } else {
                delete dataQFirst;
                dataQFirst = 0;
                dataQLast = 0;
                dataQSize = 0;
            }
        }

        MAP_PCM_gotoLPM0InterruptSafe( );
    }
#endif
}

void HeaderHandle( HeaderPacket * packet ) {
    if ( packet->checkCRC( ) ) {
        /* Error! Send NAK */
        i2cInterface.sendHeader(&NAKPacket);
    } else {
        i2cInterface.sendHeader(&ACKPacket);
        switch ( packet->cmd ) {
        case PKT_PING:
            /* Do nothing, only ACK */
            break;
        }
    }
    delete packet;
}

void DataHandle( DataPacket * packet ) {
    DataItem * item = new DataItem( );
    item->packet = packet;
    if ( dataQSize == 0 ) {
        dataQFirst = item;
        dataQLast = item;
    } else {
        dataQLast->nextItem = item;
        dataQLast = item;
    }
    dataQSize++;
}
