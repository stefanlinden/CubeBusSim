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

#define RS485PACKETLIMIT 20000
#define CANPACKETLIMIT	1000
#define I2CPACKETLIMIT	1000

extern I2CInterface i2cInterface;
extern CANInterface canInterface;
extern RS485Interface rs485Interface;

volatile long loopCounter;
volatile uint_fast8_t lastLength;
volatile uint_fast8_t lastCheckByteResponse;

volatile uint32_t packetErrorCounter;
volatile bool runTest;
char result[100];
uint_fast8_t * testData;

/* Prototypes */
void setIntervalTimer(void);
void stopIntervalTimer(void);

bool requestPacketOverI2C(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverI2C(uint_fast8_t node, uint_fast8_t length);

bool requestPacketOverCAN(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverCAN(uint_fast8_t node, uint_fast8_t length);

bool requestPacketOverRS485(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverRS485(uint_fast8_t node, uint_fast8_t length);

extern "C" {

void TestI2C(bool withTimer) {

	testData = getData();
	uint_fast8_t checkByte = 0;
	uint32_t loopCounter = 0, packetCounter = 0;
	static bool I2CInit = false;

	if (!I2CInit) {
		i2cInterface.setDataHandler(DataHandleMaster);
		i2cInterface.init(true, 0);
		I2CInit = true;
	}

	runTest = true;
	packetErrorCounter = 0;

	if (withTimer)
		setIntervalTimer();

	while (runTest) {
		testData[0] = checkByte;

		if (requestPacketOverI2C(SUBSYS_ADCS, 120))
			packetErrorCounter++;

		packetCounter++;

		checkByte++;
		if (checkByte == 0xFF)
			checkByte = 0;

		loopCounter++;

		if (!withTimer && packetCounter >= I2CPACKETLIMIT)
			break;

	}

	stopIntervalTimer();

	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			packetCounter, packetErrorCounter, ratio);
	Serial_puts(result);
}

void TestCAN(bool withTimer) {

	testData = getData();
	uint_fast8_t checkByte = 0;
	uint32_t timeout, loopCounter, packetCounter;

	static bool CANInit = false;

	if (!CANInit) {
		canInterface.setDataHandler(DataHandleMaster);
		canInterface.init(true, 0);
		CANInit = true;
	}

	runTest = true;

	if (withTimer)
		setIntervalTimer();

	packetErrorCounter = 0;
	packetCounter = 0;
	loopCounter = 0;

	while (runTest) {
		testData[0] = checkByte;

		if(requestPacketOverCAN(SUBSYS_ADCS, 120))
			packetErrorCounter++;
		packetCounter++;

		checkByte++;
		loopCounter++;
		if (checkByte == 0xFF)
			checkByte = 0;

		if(!withTimer && packetCounter >= CANPACKETLIMIT)
			break;
	}
	stopIntervalTimer();
	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			packetCounter, packetErrorCounter, ratio);
	Serial_puts(result);
}

void TestRS485(bool withTimer) {

	testData = getData();
	uint_fast8_t checkByte = 0;
	uint32_t loopCounter, packetCounter;
	static bool RS485Init = false;

	if (!RS485Init) {
		rs485Interface.setDataHandler(DataHandleMaster);
		rs485Interface.init(true, 0);
		RS485Init = true;
	}

	runTest = true;
	loopCounter = 0;
	packetCounter = 0;
	packetErrorCounter = 0;

	if (withTimer)
		setIntervalTimer();

	while (runTest) {
		testData[0] = checkByte;

		if (requestPacketOverRS485(SUBSYS_ADCS, 120))
			packetErrorCounter++;
		packetCounter++;

		loopCounter++;
		checkByte++;
		if (checkByte == 0xFF)
			checkByte = 0;

		if (!withTimer && packetCounter >= RS485PACKETLIMIT)
			break;

	}

	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\n",
			packetCounter, packetErrorCounter, ratio);
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
	lastLength = length;
	lastCheckByteResponse = data[0];
}
}

bool requestPacketOverI2C(uint_fast8_t node, uint_fast8_t length) {
	i2cInterface.transmitData(node, (uint_fast8_t *) testData, 2);
	if (i2cInterface.requestData(length, node) == testData[0]) {
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		return 0;
	} else {
		return 1;
	}
}
bool transmitPacketOverI2C(uint_fast8_t node, uint_fast8_t length) {
	i2cInterface.transmitData(node, (uint_fast8_t *) testData, length);
	return 0;
}

bool requestPacketOverCAN(uint_fast8_t node, uint_fast8_t length) {
	uint32_t timeout;

	lastLength = 0;
	canInterface.requestData(10, node);
	if (!canInterface.transmitData(node, (uint_fast8_t *) testData, 2)) {

		timeout = CANTIMEOUT;
		while (lastLength != length && timeout)
			timeout--;

		if (!timeout) {
			canInterface.doSoftReset();
		}

		if (lastCheckByteResponse == testData[0] && timeout) {
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}
bool transmitPacketOverCAN(uint_fast8_t node, uint_fast8_t length) {
	return canInterface.transmitData(node, (uint_fast8_t *) testData, 2);
}

bool requestPacketOverRS485(uint_fast8_t node, uint_fast8_t length) {
	uint32_t timeout;

	lastLength = 0;
	rs485Interface.requestData(length, node);
	rs485Interface.transmitData(node, (uint_fast8_t *) testData, 2);

	timeout = RS485TIMEOUT;
	while (lastLength < length && timeout)
		timeout--;

	if (lastCheckByteResponse == testData[0] && timeout) {
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		return 0;
	} else {
		return 1;
	}
}

bool transmitPacketOverRS485(uint_fast8_t node, uint_fast8_t length) {
	rs485Interface.transmitData(node, (uint_fast8_t *) testData, length);
	return 0;
}

void setIntervalTimer(void) {
	MAP_Timer32_initModule(TIMER32_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to repeatedly trigger every 10 seconds */
	MAP_Timer32_setCount( TIMER32_BASE, 1875000);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_BASE, true);
}

void stopIntervalTimer(void) {
	MAP_Timer32_haltTimer(TIMER32_BASE);
}
