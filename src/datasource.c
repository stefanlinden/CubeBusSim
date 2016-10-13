/*
 * datasource.c
 *
 *  Created on: 13 Oct 2016
 *      Author: stefa_000
 */

#include <stdint.h>

#include "datasource.h"

uint_fast8_t NumBytesFromOBC[9] = {0, 2, 2, 2, 2, 2, 2, 2, 250};

uint_fast8_t * getDataForNode(uint_fast8_t node) {
return 0;
}

uint_fast8_t getNumBytesFromOBC(uint_fast8_t node) {
	return NumBytesFromOBC[node];
}
