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

class BusInterface {
public:
    /* Interrupt handlers */
    void (*HeaderHandler)(HeaderPacket *);
    void (*DataHandler)(DataPacket *);
    HeaderPacket * (*HeaderQueueHandler)( void );
    DataPacket * (*DataQueueHandler)( void );

    /* The address of this node */
    uint_fast8_t ownAddress;
    bool isMaster;

    /* General fields and methods */
    virtual uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );
    virtual uint_fast8_t sendData( DataPacket * );

    /* Bus MASTER methods */
    virtual uint_fast8_t sendHeader( HeaderPacket * );

    /* Handlers */
    virtual void setHeaderHandler(void (*)(HeaderPacket *));
    virtual void setDataHandler(void (*)(DataPacket *));
    virtual void setHeaderQueueCallBack(HeaderPacket* (*)(void));
    virtual void setDataQueueCallBack(DataPacket* (*)(void));
};


#endif /* INCLUDE_BUSINTERFACE_H_ */
