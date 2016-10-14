/*
 * datasource.h
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#ifndef INCLUDE_DATASOURCE_H_
#define INCLUDE_DATASOURCE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct {
	uint_fast8_t * data;
	uint_fast8_t size;
} DataPack;

void initDataCRC( void );
uint_fast8_t * getData( void );
uint16_t getCRCForData( uint_fast8_t size );
uint_fast8_t getNumBytesFromOBC(uint_fast8_t node);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DATASOURCE_H_ */
