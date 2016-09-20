/*
 * messagequeue.cpp
 *
 *  Created on: 20 Sep 2016
 *      Author: Stefan van der Linden
 */

#include "messagequeue.h"
#include "simpackets.h"

DataQueue::DataQueue( void ) {
    length = 0;
    head = 0;
    tail = 0;
}

uint_fast8_t DataQueue::getLength( void ) {
    return length;
}

void DataQueue::add( DataPacket * packet ) {
    DataItem * item = new DataItem;
    item->packet = packet;

    item->nextItem = 0;

    if(length == 0) {
        head = item;
        tail = item;
    } else {
        tail->nextItem = item;
        tail = item;
    }
    length++;
}

DataPacket * DataQueue::pop( void ) {
    if(!length)
        return 0;

    DataPacket * result;
    DataItem * newHead;

    result = head->packet;

    if(length == 1) {
        newHead = 0;
    } else {
        newHead = head->nextItem;
    }

    delete head;
    head = newHead;
    length--;

    return result;
}
