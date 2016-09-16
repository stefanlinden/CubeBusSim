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

uint_fast8_t buff[10];
uint_fast8_t response;
DWire wire = DWire();
uint32_t it;

#define ISMASTER

void handleReceive( uint8_t );
void handleRequest( void );

int main( void ) {
    /* Disabling the Watchdog */
    MAP_WDT_A_holdTimer( );

    wire.setFastMode();

    wire.onReceive(handleReceive);
    wire.onRequest(handleRequest);

#ifdef ISMASTER
    wire.begin();

    while(1) {
        wire.beginTransmission(0x44);
        wire.write(1);
        wire.write(2);
        wire.write(3);
        wire.endTransmission(true);

        response = wire.requestFrom(0x44, 4);
        printf("Response: %d\n", response);
        for(it = 0; it < 80000; it++);
    }

#else
    wire.begin(0x44);

    while(1) {
        MAP_PCM_gotoLPM0InterruptSafe();
    }
#endif
}

/**
 * Receive interrupt handler
 * This interrupt is triggered by DWire when a full frame has been received
 * (i.e. after receiving a STOP)
 */
void handleReceive( uint8_t numBytes ) {

    printf("Got a message\n");

    // Get the rx buffer's contents from the DWire object
    for ( int i = 0; i < numBytes; i++ ) {
        buff[i] = wire.read( );

        // Print the contents of the received byte
        //serial->print(buff[i]);
    }
    // End the line in preparation of the next receive event
    //serial->println( );
}

/**
 * Request interrupt handler
 * This request is called on a read request from a master node.
 *
 */
void handleRequest( void ) {
    // Send back the data received from the master
    //printf("Sending response.\n");
    wire.write(1);
    wire.write(2);
    wire.write(3);
    wire.write(4);
}
