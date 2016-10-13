/*
 * i2cdriver.h
 *
 *  Created on: 15 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_I2CDRIVER_H_
#define INCLUDE_I2CDRIVER_H_

#include "businterface.h"

#define I2C_MODE_NORMAL         0
#define I2C_MODE_DATARECEIVE    1

class I2CInterface: public BusInterface {
public:
    /* General fields and methods */
    uint_fast8_t init( bool asMaster, uint_fast8_t ownAddress );

    /* Bus MASTER methods */
    //uint_fast8_t requestData( uint_fast8_t, uint_fast8_t );

    void setDataHandler(void (*)(uint_fast8_t, uint_fast8_t *, uint_fast8_t));
    uint_fast8_t transmitData(uint_fast8_t node, uint_fast8_t * data, uint_fast8_t size);
    uint_fast8_t requestData(uint_fast8_t howMuch, uint_fast8_t node);
};

#endif /* INCLUDE_I2CDRIVER_H_ */
