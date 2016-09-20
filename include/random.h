/*
 * random.h
 *
 *  Created on: 20 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_RANDOM_H_
#define INCLUDE_RANDOM_H_


#ifdef __cplusplus
extern "C" {
#endif


void srandom(unsigned long seed);

long random(void);

long random_r(unsigned long *ctx);

static long do_random(unsigned long *ctx);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_RANDOM_H_ */
