/*
 * crc.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>

uint_fast16_t getCRC( uint_fast8_t * data, uint_fast8_t length ) {
    static uint32_t ii;

    MAP_CRC32_setSeed(0xFFFF, CRC16_MODE);

    for ( ii = 0; ii < length; ii++ )
        MAP_CRC32_set8BitDataReversed(data[ii], CRC16_MODE);

    /* Getting the result from the hardware module */
    return MAP_CRC32_getResult(CRC16_MODE);
}
