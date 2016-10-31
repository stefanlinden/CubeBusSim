/*
 * rs485driver.cpp
 *
 *  Created on: 3 Oct 2016
 *      Author: Stefan van der Linden
 */

#include <driverlib.h>
#include <stdint.h>
#include <msp.h>

#include "rs485driver.h"
#include "businterface.h"
#include "addresstable.h"
#include "CubeBusSim.h"
#include "datasource.h"
#include "tests.h"
#include "crc.h"

/* Globals */
void (*rs485_DataHandler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t);
RS485Interface * rs485Instance;

extern uint_fast8_t rxBuffer[256];
uint_fast8_t txBuffer[256];

const uint_fast8_t ack = 1;

uint_fast8_t * testData;

volatile uint32_t dataRXSize, dataRXCount, requestSize;
volatile bool preambleRcvd = false;

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
/* baud rate: 1048576 bps */
const eUSCI_UART_Config uartConfig_RS485 = { EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
		2,                                       // BRDIV
		13,                                      // UCxBRF
		221,                                     // UCxBRS
		EUSCI_A_UART_NO_PARITY,                  // No Parity
		EUSCI_A_UART_LSB_FIRST,                  // LSB First
		EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
		EUSCI_A_UART_MODE,            			 // UART mode
		EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
		};

/* Main methods */
uint_fast8_t RS485Interface::init(bool asMaster, uint_fast8_t ownAddress) {
	this->isMaster = asMaster;
	this->ownAddress = ownAddress;

	rs485Instance = this;

	testData = getData();

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
	MAP_Interrupt_enableMaster();

	/* Enable the RE/DE pin and set to RX (low). Pin is active high for TX */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
	return 0;
}

void RS485Interface::setDataHandler(
		void (*handler)(uint_fast8_t, uint_fast8_t *, uint_fast8_t)) {
	DataHandler = handler;
	rs485_DataHandler = handler;
}

uint_fast8_t RS485Interface::requestData(uint_fast8_t howMuch,
		uint_fast8_t address) {
	requestSize = howMuch + 1;
	dataRXCount = 0;
	preambleRcvd = false;
	return 0;
}

uint_fast8_t RS485Interface::transmitData(uint_fast8_t node,
		uint_fast8_t * data, uint_fast8_t size) {
	uint_fast8_t ii, address;

	address = getRS485Address(node);

	/* Get the corresponding CRC */
	beginCRC();
	addIntForCRC(size);
	addIntForCRC(address);
	addDataForCRC(data, size);
	uint16_t crc = getCRCResult();

	/* Enable TX */
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);

	/* Transmit the preamble and the number of data bytes first */
	MAP_UART_transmitData(EUSCI_A2_BASE, 0x55);
	MAP_UART_transmitData(EUSCI_A2_BASE, size);

	/* Transmit the address */
	while (!(UCA2IFG & UCTXIFG))
		;
	MAP_UART_transmitData(EUSCI_A2_BASE, address);

	/* Loop through the bytes, transmitting each one */
	for (ii = 0; ii < size; ii++) {
		/* Block until we can write to the buffer */
		while (!(UCA2IFG & UCTXIFG))
			;

		/* Transmit the data byte */
		MAP_UART_transmitData(EUSCI_A2_BASE, data[ii]);
	}

	/* Transmit the CRC */
	MAP_UART_transmitData(EUSCI_A2_BASE, (crc >> 8) & 0xFF);
	while (!(UCA2IFG & UCTXIFG))
		;
	MAP_UART_transmitData(EUSCI_A2_BASE, crc & 0xFF);

	/* Block until everything has been transmitted */
	while (UCA2STATW & UCBUSY)
		;

	/* Disable TX */
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);

	return 0;
}

/* Interrupt handlers */
extern "C" {
void EUSCIA2_IRQHandler(void) {
	uint16_t crc_received, crc_check;
	uint_fast8_t databyte;

	/* Get the interrupt status */
	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

	// do NOT clear flags here
	// MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

	if (status & EUSCI_A_UART_RECEIVE_INTERRUPT) {

		databyte = MAP_UART_receiveData(EUSCI_A2_BASE);

		if (!preambleRcvd) {
			if (databyte == 0x55) {
				dataRXSize = 0;
				dataRXCount = 0;
				preambleRcvd = true;
			}
			return;
		}

		if (!dataRXSize) {
			/* First byte is number of data payload bytes in the message, second byte is address. Last two bytes are CRC-16 */
			dataRXSize = databyte + 3;
			dataRXCount = 0;
			return;
		}

		rxBuffer[dataRXCount] = databyte;
		dataRXCount++;

		if (dataRXCount >= dataRXSize) {

			beginCRC();
			addIntForCRC(dataRXCount - 3);
			addIntForCRC(rxBuffer[0]);
			addDataForCRC(&rxBuffer[1], dataRXSize - 3);
			crc_check = getCRCResult();

			crc_received = (rxBuffer[dataRXCount - 2] << 8)
					+ rxBuffer[dataRXCount - 1];
			if (rxBuffer[0] == getRS485Address(rs485Instance->ownAddress)
					&& crc_check == crc_received) {
				rs485_DataHandler(RS485BUS, &rxBuffer[1], dataRXSize - 3);

				/* If we got a command byte, then respond */
				if (!rs485Instance->isMaster && dataRXSize == 5) {
					MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);

					/* Transmit a response */
					testData[0] = rxBuffer[1];
					rs485Instance->transmitData(SUBSYS_OBC,
							(uint_fast8_t *) testData,
							getNumBytesFromSlave(rs485Instance->ownAddress));
				} else if (!rs485Instance->isMaster && dataRXSize == 253) {
					MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);

					/* Transmit a response */
					rs485Instance->transmitData(SUBSYS_OBC,
							(uint_fast8_t *) &ack, 1);
				}
			}
			dataRXCount = 0;
			dataRXSize = 0;
			preambleRcvd = false;
		}

	}

}
}
