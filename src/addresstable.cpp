/*
 * addresstable.cpp
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <stdint.h>
#include "addresstable.h"

const uint_fast8_t i2cAddresses[3] = {0x00, 0x01, 0x02};
const uint_fast8_t canAddresses[3] = {0x00, 0x10, 0x20};
const uint_fast8_t usbAddresses[3] = {0x00, 0x01, 0x02};

uint_fast8_t getI2CAddress(uint_fast8_t simAddress) {
    return i2cAddresses[simAddress];
}

uint_fast8_t getCANAddress(uint_fast8_t simAddress) {
    return canAddresses[simAddress];
}

uint_fast8_t getUSBAddress(uint_fast8_t simAddress) {
    return usbAddresses[simAddress];
}
