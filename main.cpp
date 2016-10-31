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
#include "i2cdriver.h"
#include "candriver.h"
#include "rs485driver.h"
#include "CubeBusSim.h"
#include "addresstable.h"
#include "serialcom.h"
#include "serialmenu.h"
#include "tests.h"
#include "datasource.h"

#define USE_CAN
#define USE_I2C
#define USE_RS485

/* Select the correct subsystem here */
#define SUBSYSTEM SUBSYS_OBC
//#define SUBSYSTEM SUBSYS_EPS
//#define SUBSYSTEM SUBSYS_ADCS
//#define SUBSYSTEM SUBSYS_GPS
//#define SUBSYSTEM SUBSYS_PROP
//#define SUBSYSTEM SUBSYS_TCRADIO
//#define SUBSYSTEM SUBSYS_PL
//#define SUBSYSTEM SUBSYS_MM
//#define SUBSYSTEM SUBSYS_PLRADIO

/* Interfaces */
I2CInterface i2cInterface;
CANInterface canInterface;
RS485Interface rs485Interface;

/* Counters */
volatile uint_fast8_t RXCounter;

/* Variables for testing */
volatile uint_fast16_t testsToRun;

/* Buffers */
uint_fast8_t rxBuffer[256];

/* Boot Counter */
#pragma DATA_SECTION(".bootCount");
volatile uint32_t bootCount;
volatile uint_fast8_t testMode;

int main(void) {
	/* Disabling the Watchdog */
	MAP_WDT_A_holdTimer();

	/* Increment the boot counter */
	bootCount++;

	/* preinit all pulldowns/pullups */
	P1REN = 0xFF;
	P2REN = 0xFF;
	P3REN = 0xFF;
	P4REN = 0xFF;
	P5REN = 0xFF;
	P6REN = 0xFF;
	P7REN = 0xFF;
	P8REN = 0xFF;
	P9REN = 0xFF;
	P10REN = 0xFF;

	// Initialise the USB CS (to avoid floating)
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7);
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);

	/* Initialise the LEDs */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0); /* red 'busy' LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0); /* red LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1); /* green LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2); /* Blue LED */

	/* Initialise push button for mode select */
	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

	/* If testMode == 0, then boot in I2C/CAN. If testMode == 1, then boot in RS485 mode */
	/* Default is I2C only. Only applicable for slave. */
	if (MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4)) {
		testMode = 0;
	} else {
		testMode = 1;
	}

	/* Extra mode: if testMode == 2, then start with I2C only, otherwise with I2C/CAN */
	if (testMode == 0 && MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1))
		testMode = 2;

	/* Initialise the test data set */
	initDataCRC();

#if SUBSYSTEM == SUBSYS_OBC

	testsToRun = 0;

	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

	Serial_init();

	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

	while (1) {

		/* Check whether we have to run a test */
		if (testsToRun & TESTI2C) {
			Serial_disableISR();
			Serial_puts("\n *** Running I2C Test with Timer... ***\n");
			TestI2C(true, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTI2CNOTIME) {
			Serial_disableISR();
			Serial_puts("\n *** Running I2C Test with Packet Limit... ***\n");
			TestI2C(false, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTI2CPOWER) {
			Serial_disableISR();
			Serial_puts(
					"\n *** Running I2C Test for Power Measurement... ***\n");
			TestI2C(true, true);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTCAN) {
			Serial_disableISR();
			Serial_puts("\n *** Running CAN Test with Timer... ***\n");
			TestCAN(true, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTCANNOTIME) {
			Serial_disableISR();
			Serial_puts("\n *** Running CAN Test with Packet Limit... ***\n");
			TestCAN(false, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTCANPOWER) {
			Serial_disableISR();
			Serial_puts(
					"\n *** Running CAN Test for Power Measurement... ***\n");
			TestCAN(true, true);
			Serial_puts("\n> ");
			Serial_enableISR();
		}
		if (testsToRun & TESTRS485) {
			Serial_disableISR();
			Serial_puts("\n *** Running RS485 Test with Timer... ***\n");
			TestRS485(true, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTRS485NOTIME) {
			Serial_disableISR();
			Serial_puts("\n *** Running RS485 Test with Packet Limit... ***\n");
			TestRS485(false, false);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		if (testsToRun & TESTRS485POWER) {
			Serial_disableISR();
			Serial_puts(
					"\n *** Running RS485 Test for Power Measurement... ***\n");
			TestRS485(true, true);
			Serial_puts("\n> ");
			Serial_enableISR();
		}

		testsToRun = 0;
		while(!testsToRun);
		//MAP_PCM_gotoLPM0();
	}

#else

	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

	if (!testMode || testMode == 2) {
		i2cInterface.setDataHandler(DataHandleMaster);
		i2cInterface.init(false, SUBSYSTEM);

		if(testMode != 2) {
			canInterface.setDataHandler(DataHandleMaster);
			canInterface.init(false, SUBSYSTEM);
		}

		MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
	} else {
		rs485Interface.setDataHandler(DataHandleMaster);
		rs485Interface.init(false, SUBSYSTEM);
		MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
	}

	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
	while (1) {
		//MAP_PCM_gotoLPM0InterruptSafe();
		__nop();
	}

#endif
}
