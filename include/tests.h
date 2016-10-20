/*
 * tests.h
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#ifndef INCLUDE_TESTS_H_
#define INCLUDE_TESTS_H_

#include <msp.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I2CBUS 		0
#define CANBUS 		1
#define RS485BUS 	2

#define TESTI2C 		BIT0
#define TESTCAN 		BIT1
#define TESTRS485		BIT2
#define TESTI2CNOTIME 	BIT3
#define TESTCANNOTIME	BIT4
#define TESTRS485NOTIME BIT5

void TestI2C( bool withTimer );
void TestCAN( bool withTimer );
void TestRS485( bool withTimer );

void DataHandleMaster(uint_fast8_t bus, uint_fast8_t * data, uint_fast8_t size);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_TESTS_H_ */
