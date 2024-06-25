/*
 * callbacks.c
 *
 *  Created on: Jun 24, 2024
 *      Author: pablo-jean
 */

#include "board.h"

#include "interface.h"
#include "app.h"

void board_btn_sel_callback(){
	// TODO implement handler interface
	interface_button_pressed(interface, BUTTON_SEL);
}

void board_btn_enter_callback(){
	interface_button_pressed(interface, BUTTON_ENTER);
}
