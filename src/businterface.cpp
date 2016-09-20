/*
 * businterface.cpp
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "businterface.h"
#include "CubeBusSim.h"
#include "simpackets.h"
#include "messagequeue.h"


uint_fast8_t BusInterface::init(bool asMaster, uint_fast8_t ownAddress) {
    return ERR_UNDEFINED;
}

uint_fast8_t BusInterface::sendData(DataPacket * param) {
    return ERR_UNDEFINED;
}

uint_fast8_t BusInterface::sendHeader(HeaderPacket * packet) {
    return ERR_UNDEFINED;
}

uint_fast8_t BusInterface::requestData(uint_fast8_t howMuch, uint_fast8_t address ) {
    return ERR_UNDEFINED;
}

void BusInterface::setHeaderHandler(void (*handler)(HeaderPacket *)) {
    HeaderHandler = handler;
}

void BusInterface::setDataHandler(void (*handler)(DataPacket *)) {
    DataHandler = handler;
}

void BusInterface::queueData(DataPacket * packet) {
    dataQueue.add(packet);
}

