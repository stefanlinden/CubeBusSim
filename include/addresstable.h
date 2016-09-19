/*
 * addresstable.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_ADDRESSTABLE_H_
#define INCLUDE_ADDRESSTABLE_H_

#include <stdint.h>

uint_fast8_t getI2CAddress(uint_fast8_t);
uint_fast8_t getCANAddress(uint_fast8_t);
uint_fast8_t getUSBAddress(uint_fast8_t);

#endif /* INCLUDE_ADDRESSTABLE_H_ */
