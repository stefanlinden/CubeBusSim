/*
 * serialcom.c
 *
 *  Created on: 10 Oct 2016
 *      Author: Stefan van der Linden
 */

#include <stdint.h>
#include <driverlib.h>
#include <msp.h>

#include "serialcom.h"
#include "serialmenu.h"

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */

volatile uint_fast8_t lastChar;
extern volatile uint_fast8_t testsToRun;

/* Current setting: 19200 bps */
const eUSCI_UART_Config uartConfig_PC = {
EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
		156,                                       // BRDIV
		4,                                       // UCxBRF
		0,                                      // UCxBRS
		EUSCI_A_UART_NO_PARITY,                  // No Parity
		EUSCI_A_UART_LSB_FIRST,                  // LSB First
		EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
		EUSCI_A_UART_MODE,                       // UART mode
		EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
		};

void Serial_init(void) {
	/* Selecting P1.2 and P1.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
	GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Configuring UART Module */
	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig_PC);

	/* Enable UART module */
	MAP_UART_enableModule(EUSCI_A0_BASE);

	/* Enabling interrupts */
	MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
	MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
	//MAP_Interrupt_enableSleepOnIsrExit();
	MAP_Interrupt_enableMaster();

	/* board LED */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

	Menu_displayBootMsg();
}

uint_fast8_t Serial_puts(const char * str) {
	int status = -1;

	if (str != 0) {
		status = 0;

		while (*str != '\0') {
			/* Wait for the transmit buffer to be ready */
			while (!(UCA0IFG & BIT1 ))
				;

			/* Transmit data */
			MAP_UART_transmitData(EUSCI_A0_BASE, *str);

			/* If there is a line-feed, add a carriage return */
			if (*str == '\n') {
				/* Wait for the transmit buffer to be ready */
				while (!(UCA0IFG & BIT1 ))
					;
				UCA0TXBUF = '\r';
			}

			str++;
		}
	}

	return status;
}

void Serial_putchar(uint_fast8_t c) {
	while (!(UCA0IFG & BIT1 ))
		;

	/* Transmit data */
	MAP_UART_transmitData(EUSCI_A0_BASE, c);
}

void Serial_putint(uint32_t num) {
	char str[11];
	uint_fast8_t i;

	itoa(str, 11, num, 10);

	for(i = 0; i < 11; i++) {
		if(str[i] != '0')
			break;
	}

	if(i == 10)
		Serial_putchar('0');
	else
		Serial_puts((const char *) &str[i]);
}

void Serial_disableISR( void ) {
	MAP_Interrupt_disableInterrupt(INT_EUSCIA0);
}

void Serial_enableISR( void ) {
	MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
}

void itoa(char * str, uint8_t len, uint32_t val, uint8_t base) {
	uint8_t i;

	for (i = 1; i < len; i++) {
		str[len - (i + 1)] = (uint8_t) ((val % base) + '0');
		val /= base;
	}
	str[i - 1] = '\0';
}


void EUSCIA0_IRQHandler(void) {
	uint_fast8_t result, data;

	result = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

	if (result & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) {
		data = MAP_UART_receiveData(EUSCI_A0_BASE);

		while (! MAP_UART_getInterruptStatus(EUSCI_A0_BASE,
		EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)
				& EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)
			;
		MAP_UART_transmitData(EUSCI_A0_BASE, data);
		if (data == '\r') {
			if (lastChar != 0) {
				Serial_puts("\n");
				Menu_parseOption(lastChar);
			}
			lastChar = 0;
			if(!testsToRun)
				Serial_puts("\n> ");
		} else {
			lastChar = data;
		}

	}
}
