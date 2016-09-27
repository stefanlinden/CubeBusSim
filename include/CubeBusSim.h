/*
 * CubeBusSim.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_CUBEBUSSIM_H_
#define INCLUDE_CUBEBUSSIM_H_

#define ERR_TIMEOUT     0x01
#define ERR_NAK         0x02
#define ERR_CRC         0x03
#define ERR_UNEXPECTED  0x04
#define ERR_NODATASET   0x05
#define ERR_UNDEFINED   0x06

#define SUBSYS_OBC  0
#define SUBSYS_EPS  1
#define SUBSYS_ADCS 2
#define SUBSYS_PL   3

#endif /* INCLUDE_CUBEBUSSIM_H_ */
