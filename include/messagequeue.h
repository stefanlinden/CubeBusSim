/*
 * messagequeue.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_MESSAGEQUEUE_H_
#define INCLUDE_MESSAGEQUEUE_H_

#define QTYPE_HEADER 0
#define QTYPE_DATA 1

#include "simpackets.h"

class DataItem {
public:
    DataItem * nextItem;
    DataPacket * packet;
};

class DataQueue {
public:
    DataQueue( void );
    uint_fast8_t getLength( void );
    void add( DataPacket * );
    DataPacket * pop( void );

private:
    DataItem * head;
    DataItem * tail;
    uint_fast8_t length;

};


#endif /* INCLUDE_MESSAGEQUEUE_H_ */
