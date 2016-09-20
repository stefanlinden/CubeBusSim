/*
 * i2cdriver.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_I2CDRIVER_H_
#define INCLUDE_I2CDRIVER_H_

#include "businterface.h"
#include "simpackets.h"

#define I2C_MODE_NORMAL         0
#define I2C_MODE_DATARECEIVE    1

class I2CInterface: public BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );
    uint_fast8_t sendData( DataPacket * );

    /* Bus MASTER methods */
    uint_fast8_t sendHeader( HeaderPacket * );
    uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );

    void setHeaderHandler(void (*)(HeaderPacket *));
    void setDataHandler(void (*)(DataPacket *));
};

#endif /* INCLUDE_I2CDRIVER_H_ */
