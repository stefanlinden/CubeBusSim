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
#include "CubeBusSim.h"
#include "addresstable.h"

/* Select the correct subsystem here */
#define SUBSYSTEM SUBSYS_OBC
//#define SUBSYSTEM SUBSYS_EPS
//#define SUBSYSTEM SUBSYS_ADCS
//#define SUBSYSTEM SUBSYS_PL

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
volatile int ownAddress;
volatile long packetCounter;

/* Variables for testing */
volatile bool doTest;

int main( void ) {
    int i;

    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

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

#if SUBSYSTEM == SUBSYS_OBC

    uint_fast8_t result;
    ownAddress = 0;

    for ( i = 0; i < 5000000; i++ )
        ;

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

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0); /* red LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1); /* green LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); /* red 'busy' LED */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);

    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

    /* Right before the main loop, we start the 32 bit timer to trigger after 10s */
    packetCounter = 0;
    /*MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                TIMER32_PERIODIC_MODE);
    MAP_Timer32_setCount( TIMER32_BASE, 10*48E6 );
    MAP_Interrupt_enableInterrupt(INT_T32_INT1);
    MAP_Interrupt_enableMaster();
    MAP_Timer32_startTimer(TIMER32_BASE, true);*/

    while ( 1 ) {
        /*** CAN ***/
        /* PINGs */
        /*canInterface.sendHeader(&pingADCS);
        while ( !result )
            result = canInterface.getLastStatus( );
        canInterface.sendHeader(&pingEPS);
        while ( !result )
            result = canInterface.getLastStatus( );
        canInterface.sendHeader(&pingPL);
        while ( !result )
            result = canInterface.getLastStatus( );*/

        /*** I2C ***/
        result = i2cInterface.sendHeader(&pingPL);


        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        //for ( i = 0; i < 100000; i++ )
        //    ;
        //i2cInterface.requestData(20, 1);
        //RXCounter = 0;
        //canInterface.requestData(2, 1);
        //while ( RXCounter != 1 )
        //    ;
        //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
        packetCounter++;
        if(!doTest)
            break;
    }

    while ( 1 ) {
        long totalBytes = packetCounter*3*2*8;
        MAP_PCM_gotoLPM0InterruptSafe( );
    }

#elif SUBSYSTEM == SUBSYS_EPS
    ownAddress = 1;
#elif SUBSYSTEM == SUBSYS_ADCS
    ownAddress = 2;
#elif SUBSYSTEM == SUBSYS_PL
    ownAddress = 3;
#endif
#if SUBSYSTEM != SUBSYS_OBC
    for( i = 0; i < 50000; i++);
    i2cInterface.init(false, getI2CAddress(ownAddress));
    canInterface.init(false, getCANAddress(ownAddress));
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

#ifdef __cplusplus
extern "C" {
#endif
void T32_INT1_IRQHandler( void ) {
    /* Handles the triggering of the 32 bit timer */
    MAP_Timer32_haltTimer(TIMER32_BASE);
    MAP_Timer32_clearInterruptFlag(TIMER32_BASE);
    doTest = false;
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
}
#ifdef __cplusplus
}
#endif
