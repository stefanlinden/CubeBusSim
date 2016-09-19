/*
 * packets.h
 *
 *  Created on: 19 Sep 2016
 *      Author: Stefan van der Linden
 */

#ifndef INCLUDE_SIMPACKETS_H_
#define INCLUDE_SIMPACKETS_H_

#include <stdint.h>

/* Basics */
#define PKT_ACK         0xFF
#define PKT_NAK         0x0F

/* DATA packets */
#define PKT_DATA        0x00
#define PKT_DATAPUSH    0x01
#define PKT_DATAPULL    0x02

/* EPS Switching Packets */
#define PKT_SWITCHON    0x10
#define PKT_SWITCHOFF   0x11
#define PKT_PING        0x20



/* A header packet is used to initiate a transaction. Can contain a command or start of a data transaction */
class HeaderPacket {
public:
    /* The command field identifies the type of message */
    uint_fast8_t cmd;

    /* The originNode field contains the address of the node which created the packet */
    uint_fast8_t originNode;

    /* The targetNode contains the address of the target */
    uint_fast8_t targetNode;

    /* A header packet may contain up to three parameters */
    uint_fast8_t * param;

    /* Two bytes contain the CRC-16 checksum */
    uint_fast16_t crc;

    /* Constructors */
    HeaderPacket(uint_fast8_t origin, uint_fast8_t target);
    HeaderPacket(uint_fast8_t * rawPacket);

    ~HeaderPacket();

    /* This method is used by a BusInterface to set the correct command + necessary parameters (this last thing may vary per interface type */
    void setCommand(uint_fast8_t cmd, uint_fast8_t param0, uint_fast8_t param1, uint_fast8_t param2);

    void calculateNewCRC( void );

    uint_fast8_t checkCRC( void );

    void clean( void );

    uint_fast8_t * getRawData( void );

private:
    /* An array to hold the raw bytes of the header */
    uint_fast8_t * rawData;

    void makeRawData( void );
};


/* A simple wrapper for a data set */
class DataPacket {
public:
    uint_fast8_t * data;
    uint_fast8_t length;
    uint_fast16_t crc;

    DataPacket(uint_fast8_t length, uint_fast8_t * data);
    ~DataPacket( void );

    void generateCRC( void );
};

#endif /* INCLUDE_SIMPACKETS_H_ */
