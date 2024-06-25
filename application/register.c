/*
 * register.c
 *
 *  Created on: Jun 24, 2024
 *      Author: pablo-jean
 */

#include "register.h"

/** Privates **/



/** Publics **/
register_err_e register_start(register_t *Register){
	BoardAssert(Register);

	storage_start(&Register->Storage);
	Register->u32TotalSpace = REGISTER_MAXIMUM_CARDS;
	register_cards_registed(Register);

	return REGISTER_OK;
}

uint32_t register_cards_registed(register_t *Register){
	uint32_t i, address, qtd;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);

	qtd = 0;
	address = REGISTER_EE_CARDS_START;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		if (RegCard->u8UsedMask == REGISTER_USED_MASK){
			qtd++;
		}
		address += REGISTER_EE_CARDS_STEP;
	}
	Register->u32CardsRegistered = qtd;
	Register->u32FreeSpace = Register->u32TotalSpace - Register->u32CardsRegistered;

	return qtd;
}

register_err_e register_check_card(register_t *Register, uint64_t CardID, uint32_t *IDX){
	uint32_t i, address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);

	address = REGISTER_EE_CARDS_START;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		if (RegCard->u8UsedMask == REGISTER_USED_MASK &&
				RegCard->u64CardID == CardID){
			if (IDX != NULL){
				*IDX = i;
			}
			return REGISTER_OK;
		}
		address += REGISTER_EE_CARDS_STEP;
	}

	return REGISTER_NOT_FOUND;
}

register_err_e register_add_card(register_t *Register, uint64_t CardID){
	uint32_t i, freeIDX, address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);

	if (Register->u32FreeSpace == 0){
		return REGISTER_NO_FREE_SPACES;
	}

	// Start with an Invalid Index
	freeIDX = 0xFF;
	address = REGISTER_EE_CARDS_START;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		if (RegCard->u8UsedMask == REGISTER_USED_MASK &&
				RegCard->u64CardID == CardID){
			return REGISTER_CARD_ALREADY_EXISTS;
		}
		if (RegCard->u8UsedMask == REGISTER_UNUSED_MASK &&
				freeIDX == 0xFF){
			freeIDX = i;
		}
		address += REGISTER_EE_CARDS_STEP;
	}
	RegCard->u8UsedMask = REGISTER_USED_MASK;
	RegCard->u64CardID = CardID;
	address = REGISTER_EE_CARDS_START + (freeIDX * REGISTER_EE_CARDS_STEP);
	storage_write(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));

	Register->u32CardsRegistered++;
	Register->u32FreeSpace--;

	return REGISTER_OK;
}

uint32_t register_list_cards(register_t *Register, uint64_t *OutArray){
	uint32_t i, It, address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);
	BoardAssert(OutArray);

	address = REGISTER_EE_CARDS_START;
	It = 0;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		if (RegCard->u8UsedMask == REGISTER_USED_MASK){
			OutArray[It] = RegCard->u64CardID;
			It++;
		}
		address += REGISTER_EE_CARDS_STEP;
	}

	return It;
}

register_err_e register_del_card_by_ID(register_t *Register, uint64_t CardID){
	uint32_t i, address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);

	if (Register->u32CardsRegistered == 0){
		// we return OK, for easily synchronization
		return REGISTER_OK;
	}

	address = REGISTER_EE_CARDS_START;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		if (RegCard->u8UsedMask == REGISTER_USED_MASK &&
				RegCard->u64CardID == CardID){
			RegCard->u64CardID = 0x0;
			RegCard->u8UsedMask = REGISTER_UNUSED_MASK;
			storage_write(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
			Register->u32CardsRegistered--;
			Register->u32FreeSpace++;
			break;
		}
		address += REGISTER_EE_CARDS_STEP;
	}
	return REGISTER_OK;
}

register_err_e register_del_card_by_IDX(register_t *Register, uint32_t IDX){
	uint32_t address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);
	BoardAssert(IDX < REGISTER_MAXIMUM_CARDS);

	if (Register->u32CardsRegistered == 0){
		// we return OK, for easily synchronization
		return REGISTER_OK;
	}

	address = REGISTER_EE_CARDS_START + (REGISTER_EE_CARDS_STEP * IDX);
	storage_read(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
	if (RegCard->u8UsedMask == REGISTER_USED_MASK){
		Register->u32CardsRegistered--;
		Register->u32FreeSpace++;
		RegCard->u64CardID = 0x0;
		RegCard->u8UsedMask = REGISTER_UNUSED_MASK;
		storage_write(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
	}

	return REGISTER_OK;
}

register_err_e register_del_all_cards(register_t *Register){
	uint32_t i, address;
	storage_t *Storage = &Register->Storage;
	reg_card_t *RegCard = &Register->RegCard;

	BoardAssert(Register);

	if (Register->u32CardsRegistered == 0){
		// we return OK, for easily synchronization
		return REGISTER_OK;
	}

	RegCard->u64CardID = 0x0;
	RegCard->u8UsedMask = REGISTER_UNUSED_MASK;
	address = REGISTER_EE_CARDS_START;
	for (i=0 ; i<REGISTER_MAXIMUM_CARDS ; i++){
		storage_write(Storage, address, (uint8_t*)RegCard, sizeof(reg_card_t));
		address += REGISTER_EE_CARDS_STEP;
	}

	Register->u32CardsRegistered = 0;
	Register->u32FreeSpace = Register->u32TotalSpace;

	return REGISTER_OK;
}
