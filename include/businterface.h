/*
 * businterface.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_BUSINTERFACE_H_
#define INCLUDE_BUSINTERFACE_H_

#include <stdint.h>
#include "simpackets.h"
#include "messagequeue.h"

class BusInterface {
public:

    /* Interrupt handlers */
    void (*HeaderHandler)(HeaderPacket *);
    void (*DataHandler)(DataPacket *);
    HeaderPacket * (*HeaderQueueHandler)( void );
    DataPacket * (*DataQueueHandler)( void );

    /* Data queue */
    DataQueue dataQueue;

    /* The address of this node */
    uint_fast8_t ownAddress;
    bool isMaster;

    /* General fields and methods */
    virtual uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    virtual void queueData( DataPacket * );

    /* Bus MASTER methods */
    virtual uint_fast8_t sendHeader( HeaderPacket * );
    virtual uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );

    /* Handlers */
    virtual void setHeaderHandler(void (*)(HeaderPacket *));
    virtual void setDataHandler(void (*)(DataPacket *));
};


#endif /* INCLUDE_BUSINTERFACE_H_ */
