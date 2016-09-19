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

class HeaderItem {
public:
    HeaderItem * nextItem;
    HeaderPacket * packet;
};

class DataItem {
public:
    DataItem * nextItem;
    DataPacket * packet;

};

class TXItem {
public:
    uint_fast8_t type;
    TXItem * nextItem;
    union {
        HeaderPacket * headerPacket;
        DataPacket * dataPacket;
    };
};

#endif /* INCLUDE_MESSAGEQUEUE_H_ */
