/*
 * tests.c
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#include <stdint.h>
#include <driverlib.h>

#include "i2cdriver.h"
#include "candriver.h"
#include "addresstable.h"
#include "tests.h"

extern I2CInterface i2cInterface;
extern CANInterface canInterface;

const uint_fast8_t twoBytes[2] = { 2, 3 };

volatile long loopCounter;
volatile uint_fast8_t lastLength;

extern "C" {

void TestI2C( void ) {
	int i;

	while (1) {
		i2cInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) twoBytes, 2);
		if (!i2cInterface.requestData(10, SUBSYS_ADCS))
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);

		for (i = 0; i < 50000; i++)
			;
	}
}


void TestCAN( void ) {
	int i;

	while(1) {
		lastLength = 0;
		canInterface.requestData(10, SUBSYS_ADCS);
		canInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) twoBytes, 2);
		while(lastLength != 10);
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);

		for (i = 0; i < 50000; i++);
	}
}

void T32_INT1_IRQHandler(void) {
	/* Handles the triggering of the 32 bit timer */
	MAP_Timer32_haltTimer(TIMER32_BASE);
	MAP_Timer32_clearInterruptFlag(TIMER32_BASE);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
}

void DataHandleMaster(uint_fast8_t bus, uint_fast8_t * data, uint_fast8_t length) {
	loopCounter++;
	lastLength = length;
}

}
