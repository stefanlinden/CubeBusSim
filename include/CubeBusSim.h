/*
 * CubeBusSim.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_CUBEBUSSIM_H_
#define INCLUDE_CUBEBUSSIM_H_

#include <stdint.h>

/* A header packet is used to initiate a transaction. Can contain a command or start of a data transaction */
class HeaderPacket {
public:
    uint_fast8_t originNode;
    uint_fast8_t targetNode;
    uint_fast8_t cmd;
    uint_fast8_t * param;
    uint_fast16_t crc;

    HeaderPacket(uint_fast8_t origin, uint_fast8_t target);

    void setCommand(uint_fast8_t cmd, uint_fast8_t param0, uint_fast8_t param1, uint_fast8_t param2, uint_fast8_t param3);
};

/* A simple wrapper for a data set */
class DataPacket {
public:
    uint_fast8_t * data;
    uint_fast8_t length;
    uint_fast16_t crc;
};

class BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( );
    virtual uint_fast8_t sendData( uint_fast8_t * );

    /* Bus MASTER methods */
    virtual uint_fast8_t sendHeader( HeaderPacket * );

    /* Bus Slave methods */
    void setCommandHandler(void *(*)(HeaderPacket *));
    void setDataHandler(void *(*)(DataPacket *));
};

#endif /* INCLUDE_CUBEBUSSIM_H_ */
