/*
 * addresstable.cpp
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <stdint.h>
#include "addresstable.h"

/* Addresses:
 * (0) OBC - (1) EPS - (2) ADCS - (3) GPS - (4) Propulsion - (5) TC Radio - (6) PL - (7) MM - (8) PL Radio
 */
const uint_fast8_t i2cAddresses[9]   = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
const uint_fast8_t canAddresses[9]   = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
const uint_fast8_t usbAddresses[4]   = {0x00, 0x01, 0x02, 0x03};
const uint_fast8_t rs485Addresses[9] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

uint_fast8_t getI2CAddress(uint_fast8_t simAddress) {
    return i2cAddresses[simAddress];
}

uint_fast8_t getCANAddress(uint_fast8_t simAddress) {
    return canAddresses[simAddress];
}

uint_fast8_t getUSBAddress(uint_fast8_t simAddress) {
    return usbAddresses[simAddress];
}

uint_fast8_t getRS485Address(uint_fast8_t simAddress) {
    return rs485Addresses[simAddress];
}
