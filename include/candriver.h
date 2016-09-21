/*
 * candriver.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_CANDRIVER_H_
#define INCLUDE_CANDRIVER_H_

#include "simpackets.h"
#include "businterface.h"

class CANInterface: public BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    /* Bus MASTER methods */
    uint_fast8_t sendHeader( HeaderPacket * );
    uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );

    void setHeaderHandler( void (*)( HeaderPacket * ) );
    void setDataHandler( void (*)( DataPacket * ) );

    void queueData( DataPacket * );

    uint_fast8_t sendData( void );
};

#endif /* INCLUDE_CANDRIVER_H_ */
