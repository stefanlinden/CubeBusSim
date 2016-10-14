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
#include "rs485driver.h"
#include "addresstable.h"
#include "tests.h"
#include "datasource.h"

extern I2CInterface i2cInterface;
extern CANInterface canInterface;
extern RS485Interface rs485Interface;

volatile long loopCounter;
volatile uint_fast8_t lastLength;

/* Prototypes */
void setTimer(void);
void stopTimer(void);

extern "C" {

void TestI2C(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();

	if (withTimer)
		setTimer();

	while (1) {
		i2cInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata, 2);
		if (!i2cInterface.requestData(10, SUBSYS_ADCS))
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);

		if (withTimer)
			MAP_PCM_gotoLPM0();
	}
	stopTimer();
}

void TestCAN(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();

	if (withTimer)
		setTimer();

	while (1) {
		lastLength = 0;
		canInterface.requestData(10, SUBSYS_ADCS);
		canInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata, 2);
		while (lastLength != 10)
			;
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);

		if (withTimer)
			MAP_PCM_gotoLPM0();
	}
	stopTimer();
}

void TestRS485(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();

	if (withTimer)
		setTimer();

	while (1) {
		lastLength = 0;
		rs485Interface.requestData(10, SUBSYS_ADCS);
		rs485Interface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata, 2);
		while (lastLength != 10)
			;

		lastLength = 0;
		rs485Interface.requestData(10, SUBSYS_PL);
		rs485Interface.transmitData(SUBSYS_PL, (uint_fast8_t *) testdata, 2);
		while (lastLength != 10)
			;

		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);

		if (withTimer)
			MAP_PCM_gotoLPM0();
	}
	stopTimer();
}

void T32_INT1_IRQHandler(void) {
	/* Handles the triggering of the 32 bit timer */
	MAP_Timer32_clearInterruptFlag(TIMER32_BASE);
}

void DataHandleMaster(uint_fast8_t bus, uint_fast8_t * data,
		uint_fast8_t length) {
	loopCounter++;
	lastLength = length;
}

}

void setTimer(void) {
	MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to repeatedly trigger every second */
	MAP_Timer32_setCount( TIMER32_BASE, 187500);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_BASE, false);
}

void stopTimer(void) {
	MAP_Interrupt_disableInterrupt(INT_T32_INT1);
	MAP_Timer32_haltTimer(TIMER32_BASE);
}
