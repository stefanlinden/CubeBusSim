/*
 * candriver.cpp
 *
 *  Created on: 20 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <stdio.h>
#include <mcp2515.h>
#include "candriver.h"
#include "addresstable.h"
#include "CubeBusSim.h"
#include "tests.h"
#include "datasource.h"

/* SPI Timing Config */
const MCP_CANTimingConfig CANTimingConfig = { 20000000, /* Oscillator Frequency */
8, /* Baud Rate Prescaler */
1, /* Propagation Delay */
1, /* Phase Segment 1 */
1, /* Phase Segment 2 */
1 /* Synchronisation Jump Width */
};

void msgHandler(MCP_CANMessage * msg);
void ErrorHandler(uint_fast8_t errorFlags);

/* Using some globals for storing the top level handlers
 * This shouldn't be a problem because there is only one instance of the CANInterface */
void (*can_DataHandler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t);
CANInterface * canInstance;

extern uint_fast8_t rxBuffer[256];
uint_fast8_t txBuffer[256];

uint_fast8_t * testData;

volatile uint_fast8_t dataRXSize, dataRXCount;

uint_fast8_t CANInterface::init(bool asMaster, uint_fast8_t ownAddress) {
	isMaster = asMaster;
	this->ownAddress = ownAddress;

	testData = getData();

	dataRXSize = 0;
	dataRXCount = 0;

	canInstance = this;

	/* Set the handler to be called when a message is received */
	MCP_init();

	MCP_setReceivedMessageHandler(&msgHandler);
	MCP_setErrorHandler(&ErrorHandler);

	doSoftReset();

	return 0;
}

uint_fast8_t CANInterface::requestData(uint_fast8_t howMuch,
		uint_fast8_t address) {
	dataRXSize = 0;
	dataRXCount = 0;
	return 0;
}

void CANInterface::doSoftReset(void) {
	uint_fast8_t canAddress;
	MCP_reset();

	while ((MCP_readRegister(RCANSTAT) >> 5) != MODE_CONFIG)
		;

	MCP_setTiming(&CANTimingConfig);

	/* Register an interrupt on TX0 and RX0 */
	/* These interrupts are handled internally, but kept externally for control */
	MCP_enableInterrupt(
	MCP_ISR_RX0IE | MCP_ISR_RX1IE | MCP_ISR_ERRIE);

	/* CAN is a multi-master system, so there is no big difference between the OBC and
	 * other 'slave'-subsystems, except for filtering of messages */
	if (isMaster) {
		/* This is the simple case: disable all filters to receive all messages */
		/* The 'BUKT' bit is also set, enabling rollover of messages in case the RXB0
		 * is unavailable */
		MCP_writeRegister(RRXB0CTRL, 0x64);
		MCP_writeRegister(RRXB1CTRL, 0x60);
	} else {
		/* The slightly more complicated case requires the setting of filters */
		MCP_writeRegister(RRXB0CTRL, 0x24);
		canAddress = getCANAddress(ownAddress);

		/* Enable the full masks for standard identifiers */
		MCP_writeRegister(RRXM0SIDH, 0xFF);
		MCP_writeRegister(RRXM0SIDL, 0xE0);

		MCP_writeRegister(RRXM1SIDH, 0xFF);
		MCP_writeRegister(RRXM1SIDL, 0xE0);

		canAddress = getCANAddress(ownAddress);
		MCP_writeRegister(RRXF0SIDL, canAddress << 5);
		MCP_writeRegister(RRXF0SIDH, (canAddress & 0xFF) >> 3);

		MCP_writeRegister(RRXF1SIDL, canAddress << 5);
		MCP_writeRegister(RRXF1SIDH, (canAddress & 0xFF) >> 3);
	}

	/* Go into NORMAL mode */
	MCP_setMode(MODE_NORMAL);
}

void msgHandler(MCP_CANMessage * msg) {
	uint_fast8_t ii;

	if (!dataRXSize && msg->length == 1) {
		dataRXSize = msg->data[0];
		dataRXCount = 0;
		return;
	}
	/* If we're a master, then we're receiving data after a request */
	if (!dataRXSize)
		return;

	for (ii = 0; ii < msg->length; ii++) {
		rxBuffer[dataRXCount + ii] = msg->data[ii];
	}

	dataRXCount += ii;

	if (dataRXCount >= dataRXSize) {
		can_DataHandler(CANBUS, rxBuffer, dataRXSize);
		if (!canInstance->isMaster) {
			testData[0] = rxBuffer[0];
			canInstance->transmitData(SUBSYS_OBC, (uint_fast8_t *) testData,
					getNumBytesFromSlave(canInstance->ownAddress));
		}
		dataRXCount = 0;
		dataRXSize = 0;
	}
}

void CANInterface::setDataHandler(
		void (*handler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t)) {
	DataHandler = handler;
	can_DataHandler = handler;
}

uint_fast8_t CANInterface::transmitData(uint_fast8_t node, uint_fast8_t * data,
		uint_fast8_t size) {
	MCP_CANMessage canMsg;
	uint_fast8_t ptr = size;

	/* Send the length of the message first */

	canMsg.ID = getCANAddress(node);
	canMsg.isExtended = 0;
	canMsg.isRequest = 0;

	canMsg.data = &size;
	canMsg.length = 1;

	if (MCP_sendBulk(&canMsg, 1) != 0)
		return ERR_TIMEOUT;

	while (ptr != 0) {

		canMsg.data = &data[size - ptr];

		if (ptr > 7)
			canMsg.length = 8;
		else
			canMsg.length = ptr;

		ptr -= canMsg.length;

		if (MCP_sendBulk(&canMsg, 1) != 0) {
			return ERR_TIMEOUT;
		}

	}

	return 0;
}

void ErrorHandler(uint_fast8_t errorFlags) {
	canInstance->doSoftReset();
}
