/*
 * crc.h
 *
 *  Created on: 16 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_CRC_H_
#define INCLUDE_CRC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t getCRC(uint_fast8_t *, uint_fast8_t );

void beginCRC( void );
void addIntForCRC(uint_fast8_t data);
void addDataForCRC(uint_fast8_t * data, uint_fast8_t length);
uint16_t getCRCResult( void );


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CRC_H_ */
