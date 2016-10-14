/*
 * rs485driver.h
 *
 *  Created on: 3 Oct 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_RS485DRIVER_H_
#define INCLUDE_RS485DRIVER_H_

#include <stdint.h>

#include "businterface.h"

class RS485Interface: public BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    /* Bus MASTER methods */
    uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );
    uint_fast8_t transmitData(uint_fast8_t node, uint_fast8_t * data, uint_fast8_t size);

    void setDataHandler(void (*)(uint_fast8_t, uint_fast8_t *, uint_fast8_t));


    uint_fast8_t sendData( void );
};

#endif /* INCLUDE_RS485DRIVER_H_ */
