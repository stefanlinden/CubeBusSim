/*
 * businterface.cpp
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "businterface.h"
#include "CubeBusSim.h"


uint_fast8_t BusInterface::init(bool asMaster, uint_fast8_t ownAddress) {
    return ERR_UNDEFINED;
}

uint_fast8_t BusInterface::transmitData(uint_fast8_t node, uint_fast8_t * data, uint_fast8_t size) {
	return ERR_UNDEFINED;
}

uint_fast8_t BusInterface::requestData(uint_fast8_t howMuch, uint_fast8_t address ) {
    return ERR_UNDEFINED;
}

void BusInterface::setDataHandler(void (*handler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t)) {
    DataHandler = handler;
}


uint_fast8_t BusInterface::getLastStatus( void ) {
    return lastStatus;
}

