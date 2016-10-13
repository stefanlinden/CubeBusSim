/*
 * i2cdriver.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "i2cdriver.h"
#include "businterface.h"
#include "CubeBusSim.h"
#include <DWire.h>
#include "addresstable.h"
#include "tests.h"

DWire wire = DWire();
uint_fast8_t i2c_rxBuffer[256];
uint_fast8_t i2c_mode;
void (*i2c_DataHandler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t);


/* Interrupt handlers */
void handleReceive(uint8_t);
void handleRequest(void);

/* Class method definitions */
uint_fast8_t I2CInterface::init(bool asMaster, uint_fast8_t ownAddress) {

	this->isMaster = asMaster;
	this->ownAddress = ownAddress;
	wire.setFastMode();

	wire.onReceive(handleReceive);
	wire.onRequest(handleRequest);


	if (asMaster)
		wire.begin();
	else
		wire.begin(getI2CAddress(ownAddress));
	return 0;
}

uint_fast8_t I2CInterface::requestData(uint_fast8_t howMuch, uint_fast8_t node) {
	uint_fast8_t ii;

	if(wire.requestFrom(getI2CAddress(node), howMuch) != howMuch)
		return 1;

	for(ii = 0; ii < howMuch; ii++)
		i2c_rxBuffer[ii] = wire.read();

	DataHandler(I2CBUS, i2c_rxBuffer, ii + 1);

	return 0;
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

	/* Buffer the data */
	for (ii = 0; ii < size; ii++)
		wire.write(data[ii]);

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
		i2c_rxBuffer[ii] = wire.read();

	i2c_DataHandler(I2CBUS, i2c_rxBuffer, ii + 1);
}

/**
 * Request interrupt handler
 * This request is called on a read request from a master node.
 */
void handleRequest(void) {
	uint_fast8_t ii;

	for (ii = 0; ii < 10; ii++)
		wire.write(ii);
}
