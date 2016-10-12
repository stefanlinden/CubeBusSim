/*
 * serialmenu.h
 *
 *  Created on: 12 Oct 2016
 *      Author: stefa_000
 */

#ifndef SERIALMENU_H_
#define SERIALMENU_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Menu_display( void );
void Menu_parseOption( uint_fast8_t option );
void Menu_displayBootCount(bool Reset);
void Menu_displayBootMsg(void);

#ifdef __cplusplus
}
#endif


#endif /* SERIALMENU_H_ */
