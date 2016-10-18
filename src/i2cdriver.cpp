/*
 * i2cdriver.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <DWire.h>
#include "i2cdriver.h"
#include "businterface.h"
#include "CubeBusSim.h"
#include "addresstable.h"
#include "tests.h"
#include "datasource.h"
#include "crc.h"

DWire wire = DWire();
extern uint_fast8_t rxBuffer[256];
uint_fast8_t i2c_mode;
uint_fast8_t * testData;
void (*i2c_DataHandler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t);

/* Interrupt handlers */
void handleReceive(uint8_t);
void handleRequest(void);

uint_fast8_t checkByteResponse;

/* Class method definitions */
uint_fast8_t I2CInterface::init(bool asMaster, uint_fast8_t ownAddress) {

	this->isMaster = asMaster;
	this->ownAddress = ownAddress;
	wire.setFastMode();

	wire.onReceive(handleReceive);
	wire.onRequest(handleRequest);

	testData = getData();

	if (asMaster)
		wire.begin();
	else
		wire.begin(getI2CAddress(ownAddress));
	return 0;
}

uint_fast8_t I2CInterface::requestData(uint_fast8_t howMuch,
		uint_fast8_t node) {
	uint_fast8_t ii;
	uint16_t crc_check, crc_received;


	if (wire.requestFrom(getI2CAddress(node), howMuch + 2) != howMuch + 2)
		return 0xFF;

	for (ii = 0; ii < howMuch + 2; ii++)
		rxBuffer[ii] = wire.read();

	crc_check = getCRC(rxBuffer, howMuch);
	crc_received = (rxBuffer[howMuch] << 8) + (rxBuffer[howMuch + 1] & 0xFF);

	if(crc_check != crc_received)
		return 0xFF;

	DataHandler(I2CBUS, rxBuffer, ii + 1);

	return rxBuffer[0];
}

void I2CInterface::setDataHandler(
		void (*handler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t)) {
	DataHandler = handler;
	i2c_DataHandler = handler;
}

uint_fast8_t I2CInterface::transmitData(uint_fast8_t node, uint_fast8_t * data,
		uint_fast8_t size) {
	uint_fast8_t ii;

	/* Start a transaction to the given address */
	wire.beginTransmission(getI2CAddress(node));

	uint16_t crc = getCRC(data, size);

	/* Buffer the data */
	for (ii = 0; ii < size; ii++)
		wire.write(data[ii]);

	/* Add the CRC */
	wire.write((crc >> 8) & 0xFF);
	wire.write(crc & 0xFF);

	/* Transmit! */
	wire.endTransmission(true);
	return 0;
}

/**
 * Receive interrupt handler
 * This interrupt is triggered by DWire when a full frame has been received
 * (i.e. after receiving a STOP)
 */
void handleReceive(uint8_t numBytes) {
	uint_fast8_t ii;

	for (ii = 0; ii < numBytes; ii++)
		rxBuffer[ii] = wire.read();

	if(numBytes == 4) /* Command is two bytes + CRC-16 */
		checkByteResponse = rxBuffer[0];

	i2c_DataHandler(I2CBUS, rxBuffer, numBytes);
}

/**
 * Request interrupt handler
 * This request is called on a read request from a master node.
 */
void handleRequest(void) {
	uint_fast8_t ii;

	testData[0] = checkByteResponse;

	uint16_t crc = getCRC(testData, 10);

	for (ii = 0; ii < 10; ii++) {
		wire.write(testData[ii]);
	}

	/* Add the CRC */
	wire.write((crc >> 8) & 0xFF);
	wire.write(crc & 0xFF);
}
