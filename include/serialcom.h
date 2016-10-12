/*
 * serialcom.h
 *
 *  Created on: 10 Oct 2016
 *      Author: Stefan van der Linden
 */

#ifndef SERIALCOM_H_
#define SERIALCOM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Serial_init( void );

uint_fast8_t Serial_puts(const char * str);

void Serial_putint(uint_fast8_t num);

void Serial_putchar(uint_fast8_t c);

void itoa(char * str, uint8_t len, uint32_t val, uint8_t base);

#ifdef __cplusplus
}
#endif

#endif /* SERIALCOM_H_ */
