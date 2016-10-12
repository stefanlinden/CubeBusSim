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
#include "mcp2515.h"
#include "delay.h"
#include "simpackets.h"
#include "i2cdriver.h"
#include "candriver.h"
#include "messagequeue.h"
#include "random.h"
#include "CubeBusSim.h"
#include "addresstable.h"
#include "serialcom.h"
#include "serialmenu.h"

/* Select the correct subsystem here */
//#define SUBSYSTEM SUBSYS_OBC
//#define SUBSYSTEM SUBSYS_EPS
//#define SUBSYSTEM SUBSYS_ADCS
#define SUBSYSTEM SUBSYS_PL

/* Interfaces */
I2CInterface i2cInterface;
CANInterface canInterface;

/* Prototypes */
DataPacket * generateDataPacket( uint_fast8_t, bool );

/* Handlers */
void DataHandle( DataPacket * packet );
void HeaderHandle( HeaderPacket * packet );

/* Some standard packets */
HeaderPacket ACKPacket(1, 0);
HeaderPacket NAKPacket(1, 0);

/* Counters */
volatile uint_fast8_t RXCounter;
volatile int ownAddress;
volatile long loopCounter;

/* Variables for testing */
volatile bool doTest, doSleep;

/* Boot Counter */
#pragma DATA_SECTION(".bootCount");
volatile uint32_t bootCount;

int main( void ) {
    int i;

    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    /* Increment the boot counter */
    bootCount++;

    // Initialise the USB CS (to avoid floating)
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

    doTest = true;
    doSleep = true;

    /* Initialise the LEDs */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0); /* red LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1); /* green LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); /* red 'busy' LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2); /* Blue LED */

    Serial_init();

    while(1) {
    	MAP_PCM_gotoLPM0();
    }

#if SUBSYSTEM == SUBSYS_OBC

    uint_fast8_t result;
    ownAddress = 0;

    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    i2cInterface.init(true, 0);
    canInterface.init(true, 0);

    HeaderPacket pingADCS(0, SUBSYS_ADCS);
    pingADCS.setCommand(PKT_PING, 0, 0, 0);
    pingADCS.calculateNewCRC( );

    HeaderPacket pingEPS(0, SUBSYS_EPS);
    pingEPS.setCommand(PKT_SWITCHON, 3, 0, 0);
    pingEPS.calculateNewCRC( );

    HeaderPacket pingPL(0, SUBSYS_PL);
    pingPL.setCommand(PKT_PING, 0, 0, 0);
    pingPL.calculateNewCRC( );


#elif SUBSYSTEM == SUBSYS_EPS
    ownAddress = 1;
#elif SUBSYSTEM == SUBSYS_ADCS
    ownAddress = 2;
#elif SUBSYSTEM == SUBSYS_PL
    ownAddress = 3;
#endif
#if SUBSYSTEM != SUBSYS_OBC
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
    for( i = 0; i < 50000; i++);
    i2cInterface.init(false, ownAddress);
    canInterface.init(false, ownAddress);
    while ( 1 ) {
        MAP_PCM_gotoLPM0InterruptSafe( );
    }
#endif
}

void HeaderHandle( HeaderPacket * packet ) {
    uint_fast8_t ii;

#if SUBSYSTEM != SUBSYS_OBC
    /* toggle the red LED to let us know its running */
    //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
#endif

    if ( packet->checkCRC( ) ) {
        /* Error! Send NAK */
        canInterface.sendHeader(&NAKPacket);
    } else {
        /* First of all, send an ACK */
        canInterface.sendHeader(&ACKPacket);
        switch ( packet->cmd ) {
        case PKT_PING:
        case PKT_SWITCHON:
        case PKT_SWITCHOFF:
            /* Do nothing, only ACK */
            break;
        case PKT_DATAPULL:
            /* Generate fake data */
            for ( ii = 0; ii < packet->param[0]; ii++ )
                canInterface.queueData(generateDataPacket(5, true));
            canInterface.sendData( );
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

extern "C" {
void T32_INT1_IRQHandler( void ) {
    /* Handles the triggering of the 32 bit timer */
    MAP_Timer32_haltTimer(TIMER32_BASE);
    MAP_Timer32_clearInterruptFlag(TIMER32_BASE);
    if(doSleep) {
        doSleep = false;
        return;
    }
    doTest = false;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
}
}
