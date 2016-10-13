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
#include "i2cdriver.h"
#include "candriver.h"
#include "random.h"
#include "CubeBusSim.h"
#include "addresstable.h"
#include "serialcom.h"
#include "serialmenu.h"
#include "tests.h"

/* Select the correct subsystem here */
#define SUBSYSTEM SUBSYS_OBC
//#define SUBSYSTEM SUBSYS_EPS
//#define SUBSYSTEM SUBSYS_ADCS
//#define SUBSYSTEM SUBSYS_PL

/* Interfaces */
I2CInterface i2cInterface;
CANInterface canInterface;

/* Prototypes */
void DataHandleSlave(uint_fast8_t bus, uint_fast8_t * data, uint_fast8_t length);

/* Counters */
volatile uint_fast8_t RXCounter;
volatile int ownAddress;

/* Variables for testing */
volatile uint_fast8_t testsToRun;

/* Boot Counter */
#pragma DATA_SECTION(".bootCount");
volatile uint32_t bootCount;

int main(void) {
	/* Disabling the Watchdog */
	MAP_WDT_A_holdTimer();

	/* Increment the boot counter */
	bootCount++;

	// Initialise the USB CS (to avoid floating)
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);

	/* Initialise the LEDs */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); /* red 'busy' LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0); /* red LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1); /* green LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2); /* Blue LED */

#if SUBSYSTEM == SUBSYS_OBC

	testsToRun = 0;
	ownAddress = 0;

	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

	i2cInterface.setDataHandler(DataHandleMaster);
	canInterface.setDataHandler(DataHandleMaster);

	i2cInterface.init(true, 0);
	canInterface.init(true, 0);

	//Serial_init();

	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

	TestCAN();

	while (1) {

		/* Check whether we have to run a test */
		if (testsToRun & TESTI2C) {
			Serial_disableISR();
			Serial_puts("\n *** Running I2C Test... ***\n");
			TestI2C();
			Serial_puts("\n>");
			Serial_enableISR();
		}

		if (testsToRun & TESTCAN) {
			Serial_disableISR();
			Serial_puts("\n *** Running CAN Test... ***\n");
			TestCAN();
			Serial_puts("\n>");
			Serial_enableISR();
		}

		testsToRun = 0;
		MAP_PCM_gotoLPM0();
	}

#else

	ownAddress = SUBSYSTEM;

	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

	i2cInterface.setDataHandler(DataHandleSlave);
	canInterface.setDataHandler(DataHandleSlave);

	i2cInterface.init(false, ownAddress);
	canInterface.init(false, ownAddress);

	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
	while ( 1 ) {
		MAP_PCM_gotoLPM0InterruptSafe( );
	}

#endif
}

void DataHandleSlave(uint_fast8_t bus, uint_fast8_t * data, uint_fast8_t length) {
}
