/*
 * businterface.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_BUSINTERFACE_H_
#define INCLUDE_BUSINTERFACE_H_

#include <stdint.h>

class BusInterface {
public:

    /* Interrupt handlers */
    void (*DataHandler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t);

    /* The address of this node */
    uint_fast8_t ownAddress;

    bool isMaster;
    uint_fast8_t lastStatus;

    /* General fields and methods */
    virtual uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    uint_fast8_t getLastStatus( void );

    /* Bus MASTER methods */
    virtual uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );
    virtual uint_fast8_t transmitData(uint_fast8_t node, uint_fast8_t * data, uint_fast8_t size);

    /* Handlers */
    virtual void setDataHandler(void (*)(uint_fast8_t, uint_fast8_t *, uint_fast8_t));

};


#endif /* INCLUDE_BUSINTERFACE_H_ */
