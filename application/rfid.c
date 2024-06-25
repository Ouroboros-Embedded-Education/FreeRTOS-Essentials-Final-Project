/*
 * rfid.c
 *
 *  Created on: Jun 24, 2024
 *      Author: pablo-jean
 */


#include "rfid.h"

/**
 * Privates
 */

/* Macros */
#define _DELAY_WHEN_DETECT		1500
#define _DELAY_WHEN_NOT_DETECT	300

/* Enumerates */

typedef enum{
	_RFID_CARD_NOT_PRESENT,
	_RFID_CARD_DETECTED
}_rfid_card_det_e;

/** Externs from MFRC630 driver **/
void mfrc630_SPI_transfer(uint8_t* tx, uint8_t* rx, uint16_t len){
	board_clrc663_txrx(tx, rx, (uint32_t)len);
}

void mfrc630_SPI_select(){
	board_spi_lock();
	board_clrc663_select();
}

void mfrc630_SPI_unselect(){
	board_clrc663_deselect();
	board_spi_unlock();
}

/* RFID Functions */

void _rfid_init(rfid_t *Rfid){

	// Reset the CLRC663
	board_clrc663_powerdown();
	vTaskDelay(pdMS_TO_TICKS(10));
	board_clrc663_powerup();
	vTaskDelay(pdMS_TO_TICKS(10));

	// Here, we only use the pooling mode
	// A better approach is to use the LPCD Mode
	Rfid->u8VersionReg = mfrc630_read_reg(MFRC630_REG_VERSION);
	mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
}

_rfid_card_det_e _rfid_pool_detect(rfid_t *Rfid){
	uint16_t ret;
	uint64_t uid = 0;
	uint8_t sak, uid_len;

	ret = mfrc630_iso14443a_REQA();
	if (ret != 0){
		uid_len = mfrc630_iso14443a_select((uint8_t*)&uid, &sak);
		Rfid->u8TagUidLen = uid_len;
		if (uid_len > 0){
			Rfid->u64TagUID = uid;

			return _RFID_CARD_DETECTED;
		}
		else{
			return _RFID_CARD_NOT_PRESENT;
		}
	}
	else{
		return _RFID_CARD_NOT_PRESENT;
	}
}

/** Task **/
void _task_Rfid(void *pvParams){
	rfid_t *Rfid = (rfid_t*)pvParams;
	TickType_t xDelay = _DELAY_WHEN_NOT_DETECT;

	_rfid_init(Rfid);
	loop {
		if (_rfid_pool_detect(Rfid) == _RFID_CARD_DETECTED){
			rfid_card_detected(Rfid);
			xDelay = _DELAY_WHEN_DETECT;
		}
		else{
			xDelay = _DELAY_WHEN_NOT_DETECT;
		}
		vTaskDelay(pdMS_TO_TICKS(xDelay));
	}
}


/**
 * Publics
 */

void rfid_start(rfid_t *Rfid){
	BaseType_t xErr;

	BoardAssert(Rfid != NULL);

	xErr = xTaskCreate(_task_Rfid,
			"Task Rfid",
			256,
			(void*)(Rfid),
			BOARD_TASK_PRIO_HIGH,
			&Rfid->xTask);
	BoardAssert(xErr == pdPASS);
}

uint64_t rfid_read(rfid_t *Rfid){
	uint64_t u64TagId;

	BoardAssert(Rfid != NULL);

	u64TagId = Rfid->u64TagUID;
	Rfid->u64TagUID = 0;

	return u64TagId;
}

/* Callback */

void __WEAK rfid_card_detected(rfid_t *Rfid){
	(void)Rfid;
}
