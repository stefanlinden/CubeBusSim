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

volatile uint32_t packetErrorCounter, tick;
volatile bool runTest, wait;
char result[100];
uint_fast8_t * testData;

/* Prototypes */
void setIntervalTimer(void);
void stopIntervalTimer(void);

void setBeatTimer(void);
void stopBeatTimer(void);

void startTick(void);
void stopTick(void);

bool requestPacketOverI2C(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverI2C(uint_fast8_t node, uint_fast8_t length);

bool requestPacketOverCAN(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverCAN(uint_fast8_t node, uint_fast8_t length);

bool requestPacketOverRS485(uint_fast8_t node, uint_fast8_t length);
bool transmitPacketOverRS485(uint_fast8_t node, uint_fast8_t length);

extern "C" {

void TestI2C(bool withTimer, bool beatTimer) {

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
	wait = false;
	packetErrorCounter = 0;

	if (withTimer)
		setIntervalTimer();

	if (beatTimer)
		setBeatTimer();

	startTick();

	while (runTest) {
		if (beatTimer)
			wait = true;

		testData[0] = checkByte;

		/* Add a single HK cycle here */
		if (requestPacketOverI2C(SUBSYS_EPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_ADCS, 120))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_GPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_PROP, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_TCRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_PL, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_MM, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverI2C(SUBSYS_PLRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverI2C(SUBSYS_MM, 250))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverI2C(SUBSYS_TCRADIO, 250))
			packetErrorCounter++;
		packetCounter++;

		checkByte++;
		if (checkByte == 0xFF)
			checkByte = 0;

		loopCounter++;

		if (!withTimer && packetCounter >= I2CPACKETLIMIT)
			break;

		while (wait)
			;
	}

	stopTick();

	stopIntervalTimer();
	stopBeatTimer();

	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	uint16_t milli = tick % 1000;
	uint16_t sec = (tick - milli) / 1000;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\nTime: %u.%.3u\n",
			packetCounter, packetErrorCounter, ratio, sec, milli);
	Serial_puts(result);
}

void TestCAN(bool withTimer, bool beatTimer) {

	testData = getData();
	uint_fast8_t checkByte = 0;
	uint32_t loopCounter, packetCounter;
	wait = false;

	static bool CANInit = false;

	if (!CANInit) {
		canInterface.setDataHandler(DataHandleMaster);
		canInterface.init(true, 0);
		CANInit = true;
	}

	runTest = true;

	if (withTimer)
		setIntervalTimer();

	if (beatTimer)
		setBeatTimer();

	packetErrorCounter = 0;
	packetCounter = 0;
	loopCounter = 0;

	startTick();

	while (runTest) {
		if (beatTimer)
			wait = true;

		testData[0] = checkByte;

		/* Add a single HK cycle here */
		if (requestPacketOverCAN(SUBSYS_EPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_ADCS, 120))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_GPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_PROP, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_TCRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_PL, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_MM, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverCAN(SUBSYS_PLRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverCAN(SUBSYS_MM, 250))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverCAN(SUBSYS_TCRADIO, 250))
			packetErrorCounter++;
		packetCounter++;

		checkByte++;
		loopCounter++;
		if (checkByte == 0xFF)
			checkByte = 0;

		if (!withTimer && packetCounter >= CANPACKETLIMIT)
			break;

		while (wait)
			;
	}
	stopTick();
	stopIntervalTimer();
	stopBeatTimer();

	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	uint16_t milli = tick % 1000;
	uint16_t sec = (tick - milli) / 1000;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\nTime: %u.%.3u\n",
			packetCounter, packetErrorCounter, ratio, sec, milli);
	Serial_puts(result);
}

void TestRS485(bool withTimer, bool beatTimer) {

	testData = getData();
	uint_fast8_t checkByte = 0;
	uint32_t loopCounter, packetCounter;
	static bool RS485Init = false;
	wait = false;

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

	if (beatTimer)
		setBeatTimer();

	startTick();

	while (runTest) {
		if (beatTimer)
			wait = true;

		testData[0] = checkByte;

		/* Add a single HK cycle here */
		if (requestPacketOverRS485(SUBSYS_EPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_ADCS, 120))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_GPS, 30))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_PROP, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_TCRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_PL, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_MM, 10))
			packetErrorCounter++;
		packetCounter++;

		if (requestPacketOverRS485(SUBSYS_PLRADIO, 10))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverRS485(SUBSYS_MM, 250))
			packetErrorCounter++;
		packetCounter++;

		if (transmitPacketOverRS485(SUBSYS_TCRADIO, 250))
			packetErrorCounter++;
		packetCounter++;

		loopCounter++;
		checkByte++;
		if (checkByte == 0xFF)
			checkByte = 0;

		if (!withTimer && packetCounter >= RS485PACKETLIMIT)
			break;

		while(wait);

	}
	stopTick();
	stopIntervalTimer();
	stopBeatTimer();

	uint64_t ratio = packetErrorCounter * 1000000ll / packetCounter;
	uint16_t milli = tick % 1000;
	uint16_t sec = (tick - milli) / 1000;
	sprintf(result,
			"Packet Counter: %u\nPacket Error Counter: %u\nPacket Error Ratio: 0.%06llu\nTime: %u.%.3u\n",
			packetCounter, packetErrorCounter, ratio, sec, milli);

	Serial_puts(result);
}

void DataHandleMaster(uint_fast8_t bus, uint_fast8_t * data,
		uint_fast8_t length) {
	lastLength = length;
	lastCheckByteResponse = data[0];
}

/* Interrupt to service the 'beat timer' */
void T32_INT1_IRQHandler(void) {
	/* Handles the triggering of the 32 bit timer */
	MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
	wait = false;
}

/* Interrupt to service the 10s test timer */
void T32_INT2_IRQHandler(void) {
	MAP_Timer32_clearInterruptFlag(TIMER32_1_BASE);
	runTest = false;
}

void SysTick_Handler(void) {
	tick++;
}

}

void startTick(void) {
	/* Enable systick to tick once every ms */
	MAP_SysTick_enableModule();
	MAP_SysTick_setPeriod(48000);
	//MAP_Interrupt_enableSleepOnIsrExit();
	MAP_SysTick_enableInterrupt();
	MAP_Interrupt_enableMaster();
	tick = 0;
}

void stopTick(void) {
	MAP_SysTick_disableInterrupt();
	MAP_SysTick_disableModule();
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

	/* Check whether we received an ack */
	if (i2cInterface.requestData(1, node) == 1) {
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		return 0;
	} else {
		return 1;
	}
}

bool requestPacketOverCAN(uint_fast8_t node, uint_fast8_t length) {
	uint32_t timeout;

	lastLength = 0;
	canInterface.requestData(length, node);
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
	uint32_t timeout;

	lastLength = 0;
	canInterface.requestData(1, node);
	if (!canInterface.transmitData(node, (uint_fast8_t *) testData, length)) {

		timeout = CANTIMEOUT;
		while (lastLength != 1 && timeout)
			timeout--;

		if (!timeout) {
			canInterface.doSoftReset();
		}

		if (lastCheckByteResponse == 1 && timeout) {
			MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
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
	uint32_t timeout;

	lastLength = 0;
	rs485Interface.requestData(1, node);
	rs485Interface.transmitData(node, (uint_fast8_t *) testData, length);

	timeout = RS485TIMEOUT;
	while (lastLength < 1 && timeout)
		timeout--;

	if (lastCheckByteResponse == 1 && timeout) {
		MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
		return 0;
	} else {
		return 1;
	}
}

void setIntervalTimer(void) {
	MAP_Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to trigger once after 10 seconds */
	MAP_Timer32_setCount( TIMER32_1_BASE, 1875000);
	MAP_Interrupt_enableInterrupt(INT_T32_INT2);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_1_BASE, true);
}

void stopIntervalTimer(void) {
	MAP_Timer32_haltTimer(TIMER32_1_BASE);
}

void setBeatTimer(void) {
	MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT,
	TIMER32_PERIODIC_MODE);
	/* Set the timer to repeatedly trigger every 1 seconds */
	MAP_Timer32_setCount( TIMER32_0_BASE, 187500);
	MAP_Interrupt_enableInterrupt(INT_T32_INT1);
	MAP_Interrupt_enableMaster();
	MAP_Timer32_startTimer(TIMER32_0_BASE, false);
}

void stopBeatTimer(void) {
	MAP_Timer32_haltTimer(TIMER32_0_BASE);
}
