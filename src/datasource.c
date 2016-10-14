/*
 * datasource.c
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#include <stdint.h>
#include <stdbool.h>

#include "datasource.h"
#include "crc.h"

const uint_fast8_t NumBytesFromOBC[9] = { 0, 2, 2, 2, 2, 2, 2, 2, 250 };
const uint_fast8_t NumBytesFromSlave[9] = { 0, 10, 10, 10, 10, 10, 10, 10, 10 };

/* A random data set generated using random.org */
const uint_fast8_t testdata[250] = { 43, 102, 135, 148, 51, 75, 224, 251, 30, 7,
		143, 216, 179, 206, 15, 63, 62, 172, 191, 229, 47, 14, 114, 41, 125,
		130, 247, 70, 66, 181, 180, 49, 137, 64, 111, 65, 144, 9, 84, 55, 166,
		186, 240, 165, 160, 105, 106, 107, 81, 127, 79, 158, 149, 5, 189, 1, 96,
		100, 90, 133, 197, 152, 201, 200, 57, 76, 184, 115, 24, 245, 196, 215,
		173, 80, 22, 116, 108, 89, 25, 183, 147, 85, 156, 29, 219, 150, 134,
		237, 68, 131, 67, 249, 153, 228, 242, 188, 233, 226, 238, 12, 71, 122,
		170, 161, 86, 58, 61, 97, 37, 33, 248, 217, 146, 243, 120, 93, 169, 50,
		74, 48, 118, 234, 198, 126, 195, 202, 0, 110, 26, 17, 141, 2, 132, 112,
		103, 45, 72, 52, 193, 225, 78, 151, 211, 213, 164, 3, 138, 94, 28, 244,
		4, 46, 109, 44, 157, 31, 199, 218, 239, 139, 32, 13, 99, 124, 182, 212,
		77, 11, 220, 73, 19, 253, 204, 178, 174, 128, 205, 163, 36, 209, 92,
		255, 91, 167, 34, 231, 21, 98, 252, 23, 223, 136, 208, 236, 256, 221, 8,
		154, 59, 227, 177, 168, 140, 95, 87, 82, 121, 175, 53, 185, 117, 10, 27,
		176, 159, 190, 145, 194, 42, 113, 171, 155, 214, 162, 254, 232, 246, 6,
		16, 207, 192, 20, 56, 40, 35, 39, 88, 250, 241, 123, 101, 203, 129, 222,
		69, 60, 38, 83, 235, 142 };

uint16_t crc2, crc10, crc30, crc120, crc250;

/* Generate all the CRCs */
void initDataCRC(void) {
	crc2 = getCRC((uint_fast8_t *) testdata, 2);
	crc10 = getCRC((uint_fast8_t *) testdata, 10);
	crc30 = getCRC((uint_fast8_t *) testdata, 30);
	crc120 = getCRC((uint_fast8_t *) testdata, 120);
	crc250 = getCRC((uint_fast8_t *) testdata, 250);
}

uint_fast8_t * getData( void ) {
	return (uint_fast8_t *) testdata;
}

uint16_t getCRCForData(uint_fast8_t size) {
	switch (size) {
	case 2:
		return crc2;
	case 10:
		return crc10;
	case 30:
		return crc30;
	case 120:
		return crc120;
	case 250:
		return crc250;
	default:
		return 0;
	}
}

uint_fast8_t getNumBytesFromOBC(uint_fast8_t node) {
	return NumBytesFromOBC[node];
}

uint_fast8_t getNumBytesFromSlave(uint_fast8_t node) {
	return NumBytesFromSlave[node];
}

