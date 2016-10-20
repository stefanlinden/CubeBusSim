/*
 * tests.c
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#include <stdint.h>
#include <stdio.h>
#include <driverlib.h>
#include <mcp2515.h>

#include "i2cdriver.h"
#include "candriver.h"
#include "rs485driver.h"
#include "addresstable.h"
#include "tests.h"
#include "datasource.h"
#include "serialcom.h"

#define RS485TIMEOUT 50000
#define CANTIMEOUT	500000

extern I2CInterface i2cInterface;
extern CANInterface canInterface;
extern RS485Interface rs485Interface;

volatile long loopCounter;
volatile uint_fast8_t lastLength;
volatile uint_fast8_t lastCheckByteResponse;

volatile uint32_t packetErrorCounter;
volatile bool runTest;
char result[100];

/* Prototypes */
void setBeatTimer(void);
void stopBeatTimer(void);
void setIntervalTimer(void);
void stopIntervalTimer(void);

extern "C" {

void TestI2C(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();
	uint_fast8_t checkByte = 0;
	uint32_t loopCounter = 0;

	runTest = true;
	packetErrorCounter = 0;

	if (withTimer)
		setIntervalTimer();

	while (runTest) {
		testdata[0] = checkByte;
		i2cInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata, 2);
		if (i2cInterface.requestData(10, SUBSYS_ADCS) == checkByte)
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		else
			packetErrorCounter++;

		checkByte++;
		if (checkByte == 0xFF)
			checkByte = 0;

		loopCounter++;

	}
	//stopBeatTimer();
	stopIntervalTimer();
	uint64_t ratio = packetErrorCounter * 1000000ll / loopCounter;
	sprintf(result,
			"Loop Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			loopCounter, packetErrorCounter, ratio);
	Serial_puts(result);
}

void TestCAN(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();
	uint_fast8_t checkByte = 0;
	uint32_t timeout, loopCounter;

	runTest = true;

	if (withTimer)
		setIntervalTimer();

	packetErrorCounter = 0;
	loopCounter = 0;

	while (runTest) {
		testdata[0] = checkByte;
		lastLength = 0;
		canInterface.requestData(10, SUBSYS_ADCS);
		if (!canInterface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata,
				2)) {

			timeout = TIMEOUT;
			while (lastLength != 10 && timeout)
				timeout--;

			if (!timeout) {
				//__nop();
				//status = MCP_readStatus();
				//status = MCP_readRegister(RCANINTE);
				//status = MCP_readRegister(RCANINTF);
				canInterface.doSoftReset();
			}

			if (lastCheckByteResponse == checkByte && timeout)
				MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
			else
				packetErrorCounter++;
		} else {
			packetErrorCounter++;
		}
		checkByte++;
		loopCounter++;
		if (checkByte == 0xFF)
			checkByte = 0;
	}
	stopIntervalTimer();
	uint_fast8_t tec = MCP_readRegister(RTEC);
	uint_fast8_t rec = MCP_readRegister(RREC);
	uint_fast8_t ifstatus = MCP_readRegister(RCANINTF);
	uint64_t ratio = packetErrorCounter * 1000000ll / loopCounter;
	sprintf(result,
			"Loop Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			loopCounter, packetErrorCounter, ratio);
	Serial_puts(result);
}

void TestRS485(bool withTimer) {

	uint_fast8_t * testdata;
	testdata = getData();
	uint_fast8_t checkByte = 0;
	uint32_t timeout, loopCounter;

	runTest = true;
	loopCounter = 0;
	packetErrorCounter = 0;

	if (withTimer)
		setIntervalTimer();

	while (runTest) {
		testdata[0] = checkByte;

		lastLength = 0;
		rs485Interface.requestData(10, SUBSYS_ADCS);
		rs485Interface.transmitData(SUBSYS_ADCS, (uint_fast8_t *) testdata, 2);

		timeout = TIMEOUT;
		while (lastLength < 10 && timeout)
			timeout--;

		if (lastCheckByteResponse == checkByte && timeout)
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		else
			packetErrorCounter++;

		checkByte++;
		loopCounter++;
		if (checkByte == 0xFF)
			checkByte = 0;

	}

	uint64_t ratio = packetErrorCounter * 1000000ll / loopCounter;
	sprintf(result,
			"Loop Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			loopCounter, packetErrorCounter, ratio);
	Serial_puts(result);

	stopIntervalTimer();
}

void T32_INT1_IRQHandler(void) {
	/* Handles the triggering of the 32 bit timer */
	MAP_Timer32_clearInterruptFlag(TIMER32_BASE);

	runTest = false;
}

void DataHandleMaster(uint_fast8_t bus, uint_fast8_t * data,
		uint_fast8_t length) {
	//loopCounter++;
	lastLength = length;
	lastCheckByteResponse = data[0];
}
}

void setBeatTimer(void) {
	MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to repeatedly trigger every second */
	MAP_Timer32_setCount( TIMER32_BASE, 187500);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_BASE, false);
}

void stopBeatTimer(void) {
	MAP_Interrupt_disableInterrupt(INT_T32_INT1);
	MAP_Timer32_haltTimer(TIMER32_BASE);
}

void setIntervalTimer(void) {
	MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to repeatedly trigger every second */
	MAP_Timer32_setCount( TIMER32_BASE, 1875000);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_BASE, true);
}

void stopIntervalTimer(void) {
	MAP_Timer32_haltTimer(TIMER32_BASE);
}
