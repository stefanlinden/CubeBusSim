/*
 * rs485driver.cpp
 *
 *  Created on: 3 Oct 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>
#include <stdint.h>

#include "rs485driver.h"
#include "businterface.h"
#include "simpackets.h"
#include "addresstable.h"
#include "CubeBusSim.h"

/* Globals */
void (*rs485_DataHandler)( uint_fast8_t *, uint_fast8_t );
uint_fast8_t rxBuffer[8];
volatile uint_fast8_t rxPtr, requestDataCtr;
RS485Interface * rs485Instance;

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
/* baud rate: 1048576 bps */
const eUSCI_UART_Config uartConfig_RS485 = { EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
        2,                                       // BRDIV
        13,                                       // UCxBRF
        221,                                      // UCxBRS
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
        };

/* Prototypes */
void transmitBytes( uint_fast8_t * data, uint_fast8_t length );

/* Main methods */
uint_fast8_t RS485Interface::init( bool asMaster, uint_fast8_t ownAddress ) {
    this->isMaster = asMaster;
    this->ownAddress = ownAddress;

    rs485Instance = this;

    rxPtr = 0;
    requestDataCtr = 0;

    /* Select the pins for the UART RX and TX */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
    GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig_RS485);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A2_BASE);

    /* Enable interrupts */
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
    MAP_Interrupt_enableMaster( );

    /* Enable the RE/DE pin and set to RX (low). Pin is active high for TX */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
    return 0;
}


void RS485Interface::setDataHandler( void (*handler)( uint_fast8_t *, uint_fast8_t ) ) {
    DataHandler = handler;
    rs485_DataHandler = handler;
}


uint_fast8_t RS485Interface::requestData( uint_fast8_t howMuch,
        uint_fast8_t address ) {
    uint_fast8_t res;

    return 0;
}

void transmitBytes( uint_fast8_t * data, uint_fast8_t length ) {
    uint_fast8_t ii;

    /* Enable TX */
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);

    /* Make sure the 'transmit complete' ISR is reset */


    /* Loop through the bytes, transmitting each one */
    for ( ii = 0; ii < length; ii++ ) {
        /* Block until we can write to the buffer */
        while ( !MAP_UART_getInterruptStatus(EUSCI_A2_BASE,
        EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG) )
            ;

        /* Transmit the byte */
        MAP_UART_transmitData(EUSCI_A2_BASE, data[ii]);
        UCA2IFG &= ~BIT3;
    }

    /* Block until everything has been transmitted */
    //while ( !(UCA2IFG & BIT3) );
    while(UCA2STATW & BIT0);

    /* Disable TX */
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
}

uint_fast8_t RS485Interface::sendData( void ) {
    DataPacket * packet;
    uint_fast8_t txBuffer[8];
    uint_fast8_t ii, it, nMessages;

    nMessages = dataQueue.getLength( );
    if ( !nMessages )
        return 1;

    for ( it = 0; it < nMessages; it++ ) {
        packet = dataQueue.pop( );

        txBuffer[0] = PKT_DATA;
        for(ii = 0; ii < 5; ii++)
            txBuffer[ii + 1] = packet->data[ii];
        txBuffer[6] = ((uint_fast8_t) packet->crc) >> 8;
        txBuffer[7] = (uint_fast8_t) (packet->crc & 0xFF);

        /* Transmit the tx buffer */
        transmitBytes(txBuffer, 8);

        /* Clean up */
        delete packet;
    }

    return 0;
}


/* Interrupt handlers */
extern "C" {
void EUSCI_A2_IRQHandler( void ) {
    /* Get the interrupt status */
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if ( status & EUSCI_A_UART_RECEIVE_INTERRUPT ) {
        rxBuffer[rxPtr] = MAP_UART_receiveData(EUSCI_A2_BASE);
        rxPtr++;

        if ( rxPtr == 8 ) {
            /* Check if this packet is data or not */
            if ( requestDataCtr > 0 && rxBuffer[0] == PKT_DATA ) {
                /* Copy the data into a new DataPacket */
                DataPacket * dataPkt = new DataPacket(5, rxBuffer);
                dataPkt->crc = ((uint_fast16_t) rxBuffer[6] << 8)
                        | (rxBuffer[7] & 0xFF);
                rxPtr = 0;
                requestDataCtr--;
                rs485_DataHandler(dataPkt);
                return;
            }

            /* If it's not data, then it's a header packet. Check whether it's meant for this node */
            if ( rxBuffer[1] != getRS485Address(rs485Instance->ownAddress) ) {
                rxPtr = 0;
                return;
            }

            if ( rxBuffer[0] == PKT_ACK ) {
                rs485Instance->lastStatus = PKT_ACK;
            } else if ( rxBuffer[0] == PKT_NAK ) {
                rs485Instance->lastStatus = PKT_NAK;
            } else {
                rs485_HeaderHandler(new HeaderPacket(rxBuffer));
            }
            rxPtr = 0;
        }
    }
}
}
