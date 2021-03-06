/*
 * menu.c
 *
 *  Created on: 12 Oct 2016
 *      Author: stefa_000
 */

#include <stdbool.h>
#include <driverlib.h>
#include "serialmenu.h"
#include "serialcom.h"
#include "tests.h"

extern volatile uint32_t bootCount;
extern volatile uint_fast16_t testsToRun;

void Menu_display(void) {

	Serial_puts("\rMenu:\n\n");
	Serial_puts(" 1) Display Boot Counter\n");
	Serial_puts(" 2) Reset Boot Counter\n");
	Serial_puts("\n 3) Start I2C Test with Timer\n");
	Serial_puts(" 4) Start CAN Test with Timer\n");
	Serial_puts(" 5) Start RS485 Test with Timer\n");

	Serial_puts("\n 6) Start I2C Test with Packet Limit\n");
	Serial_puts(" 7) Start CAN Test with Packet Limit\n");
	Serial_puts(" 8) Start RS485 Test with Packet Limit\n");

	Serial_puts("\n A) Start I2C Test for Power Measurement\n");
	Serial_puts(" B) Start CAN Test for Power Measurement\n");
	Serial_puts(" C) Start RS485 Test for Power Measurement\n");

	Serial_puts("\n 9) Reboot\n");

	Serial_puts("\n 0) Display this menu\n");

}

void Menu_displayBootCount(bool Reset) {
	if (Reset)
		bootCount = 0;
	Serial_puts("   Boot-count: ");
	Serial_putint(bootCount);
	Serial_puts("\n");
}

void Menu_displayBootMsg(void) {
	/* Transmit the startup message */
	Serial_puts("\n\n****************************************************");
	Serial_puts("\n\n");
	Serial_puts("   CubeSat Bus Simulator v0.1 (");
	Serial_puts(__DATE__);
	Serial_puts(")\n\n");
	Serial_puts("   Boot-count: ");
	Serial_putint(bootCount);
	Serial_puts("\n\n");
	Serial_puts("\r****************************************************\n\n");

	Menu_display();
	Serial_puts("\n> ");
}

void Menu_parseOption(uint_fast8_t option) {
	switch (option) {
	case '0':
		Menu_display();
		break;
	case '1':
		Menu_displayBootCount(false);
		break;
	case '2':
		Menu_displayBootCount(true);
		break;
	case '3':
		testsToRun |= TESTI2C;
		break;
	case '4':
		testsToRun |= TESTCAN;
		break;
	case '5':
		testsToRun |= TESTRS485;
		break;
	case '6':
		testsToRun |= TESTI2CNOTIME;
		break;
	case '7':
		testsToRun |= TESTCANNOTIME;
		break;
	case '8':
		testsToRun |= TESTRS485NOTIME;
		break;
	case '9':
		Serial_puts("\n *** REBOOT ***   \n\n\n");
		MAP_ResetCtl_initiateHardReset();
		break;
	case 'a':
	case 'A':
		testsToRun |= TESTI2CPOWER;
		break;
	case 'b':
	case 'B':
		testsToRun |= TESTCANPOWER;
		break;
	case 'c':
	case 'C':
		testsToRun |= TESTRS485POWER;
		break;
	default:
		Serial_puts("Error: unknown option '");
		Serial_putchar(option);
		Serial_puts("'\n");
	}
}
