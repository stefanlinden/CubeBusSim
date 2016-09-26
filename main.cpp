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
#include "usbdriver.h"
#include "messagequeue.h"
#include "random.h"

//#define ISMASTER

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
volatile uint_fast8_t RXCounter;
volatile int debugger;

int main( void ) {
    int i;
    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);

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

    for( i = 0; i < 5000000; i++);

    uint_fast8_t result;

    //i2cInterface.init(true, 0);
    canInterface.init(true, 0);

    HeaderPacket pingPkt(0, 1);
    pingPkt.setCommand(PKT_PING, 0, 0, 0);
    pingPkt.calculateNewCRC( );

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0); /* red LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1); /* green LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); /* red 'busy' LED */

    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    debugger = 0;

    while ( 1 ) {
        result = 0;
        canInterface.sendHeader(&pingPkt);
        while(!result)
            result = canInterface.getLastStatus();

        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //if(result == PKT_ACK) {
        /* Turn on green LED */
        //    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
        //    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
        //} else {
        /* Turn on red LED */
        //    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
        //    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
        //}
        for ( i = 0; i < 50000; i++ )
            ;
        //i2cInterface.requestData(20, 1);
        //RXCounter = 0;
        //canInterface.requestData(1, 1);
        /*while ( RXCounter != 10 )
            ;
        debugger++;*/
        //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
    }

    while ( 1 )
        MAP_PCM_gotoLPM0InterruptSafe( );

#else
    for( i = 0; i < 50000; i++);
    i2cInterface.init(false, 1);
    canInterface.init(false, 1);
    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
#endif
}

void HeaderHandle( HeaderPacket * packet ) {
    uint_fast8_t ii;

    if ( packet->checkCRC( ) ) {
        /* Error! Send NAK */
        canInterface.sendHeader(&NAKPacket);
    } else {
        /* First of all, send an ACK */
        canInterface.sendHeader(&ACKPacket);
        switch ( packet->cmd ) {
        case PKT_PING:
            /* Do nothing, only ACK */
            break;
        case PKT_DATAPULL:
            /* Generate fake data */
            for ( ii = 0; ii < packet->param[0]; ii++ )
                canInterface.queueData(generateDataPacket(61, true));
            canInterface.sendData();
            break;
        }
    }
    delete packet;
}

void DataHandle( DataPacket * packet ) {
    RXCounter++;
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
