/*
 * addresstable.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_ADDRESSTABLE_H_
#define INCLUDE_ADDRESSTABLE_H_

#include <stdint.h>

/* Addresses:
 * (0) OBC - (1) EPS - (2) ADCS - (3) GPS - (4) Propulsion - (5) TC Radio - (6) PL - (7) MM - (8) PL Radio
 */

#define SUBSYS_OBC  	0
#define SUBSYS_EPS  	1
#define SUBSYS_ADCS 	2
#define SUBSYS_GPS   	3
#define SUBSYS_PROP 	4
#define SUBSYS_TCRADIO 	5
#define SUBSYS_PL 		6
#define SUBSYS_MM 		7
#define SUBSYS_PLRADIO 	8

uint_fast8_t getI2CAddress(uint_fast8_t);
uint_fast8_t getCANAddress(uint_fast8_t);
uint_fast8_t getUSBAddress(uint_fast8_t);
uint_fast8_t getRS485Address(uint_fast8_t);

#endif /* INCLUDE_ADDRESSTABLE_H_ */
