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
    uint_fast8_t sendHeader( HeaderPacket * );
    uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );

    void setHeaderHandler(void (*)(HeaderPacket *));
    void setDataHandler(void (*)(DataPacket *));
};

#endif /* INCLUDE_RS485DRIVER_H_ */
