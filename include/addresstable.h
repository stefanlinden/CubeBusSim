/*
 * addresstable.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_ADDRESSTABLE_H_
#define INCLUDE_ADDRESSTABLE_H_

#include <stdint.h>

#define SUBSYS_OBC  0
#define SUBSYS_EPS  1
#define SUBSYS_ADCS 2
#define SUBSYS_PL   3

uint_fast8_t getI2CAddress(uint_fast8_t);
uint_fast8_t getCANAddress(uint_fast8_t);
uint_fast8_t getUSBAddress(uint_fast8_t);
uint_fast8_t getRS485Address(uint_fast8_t);

#endif /* INCLUDE_ADDRESSTABLE_H_ */
