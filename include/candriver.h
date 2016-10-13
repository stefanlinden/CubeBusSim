/*
 * candriver.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_CANDRIVER_H_
#define INCLUDE_CANDRIVER_H_

#include "businterface.h"

class CANInterface: public BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    /* Bus MASTER methods */
    uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );
    uint_fast8_t transmitData( uint_fast8_t, uint_fast8_t *, uint_fast8_t );

    void setDataHandler( void (*)( uint_fast8_t, uint_fast8_t *, uint_fast8_t ) );

};

#endif /* INCLUDE_CANDRIVER_H_ */
