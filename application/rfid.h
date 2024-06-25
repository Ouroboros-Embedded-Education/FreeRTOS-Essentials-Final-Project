/*
 * rfid.h
 *
 *  Created on: Jun 24, 2024
 *      Author: pablo-jean
 */

#ifndef RFID_H_
#define RFID_H_

/**
 * Includes
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <cmsis_gcc.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>
#include <semphr.h>
#include <timers.h>

#include "board.h"

#include "mfrc630.h"
#include "mfrc630_14443p4.h"

/**
 * Macros
 */


/**
 * Enumerates
 */


/**
 * Structs and Unions
 */

typedef struct{
	uint8_t u8VersionReg;
	uint64_t u64TagUID;
	uint8_t u8TagUidLen;

	TaskHandle_t xTask;
}rfid_t;

/**
 * Externs
 */


/**
 * Publics
 */

void rfid_start(rfid_t *Rfid);

uint64_t rfid_read(rfid_t *Rfid);

/* Callback */
void rfid_card_detected(rfid_t *Rfid);

#endif /* RFID_H_ */
