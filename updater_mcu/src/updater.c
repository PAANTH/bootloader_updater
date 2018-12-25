/*
 * updater.c
 *	A complementary part of bootloader-updater-pc bunch for stm32f407.
 *  Created on: Oct 31, 2018
 *      Author: paanth
 */
#include "updater.h"
#include "defs.h"
#include "stm32f4xx_flash.h"
#include "misc.h"
#include "global_vars.h"
#include "stm32f4xx_crc.h"
#include "stm32f4xx_tim.h"

uint8_t write_header(uint8_t *buf);
uint8_t write_core_chunk(uint8_t *buf, uint16_t count, uint32_t addr);
uint8_t erase_sector(uint8_t sector);
uint32_t upd_calc_crc(uint32_t core_size);



extern glob_vars_t g_vars;

//flag that core update is in process. Once set, must never be reset
uint8_t g_upd=0;


/**
 * Checks if in received buffer is 3 bytes and they are "upd".
 * This means that a new version of a core is ready to be transmitted.
 * @params
 * 	-  *buf 		- pointer to buffer
 * 	-  counter		- amount of bytes (for request must be 3)
 * @return
 * 	- 1 if buffer contains "upd"
 * 	- 0 if not or count != 3
 * */
uint8_t check_upd_request(uint8_t *buf, uint16_t count){
	uint8_t ret_val=0;

	if((buf[0] == 'u') && (buf[1] == 'p') && (buf[2] == 'd')){ //upd is in place
		if(count == 3){  //correct amount
			ret_val = 1;
			g_upd=1;
			FLASH_Unlock();
		}
	}

	return ret_val;
}



/**
 * Handles a chunk of data. Decides whether it is a header or a core chunk. Writes to flash
 * @params
 * 	-  *buf 		- pointer to buffer
 * 	-  counter		- amount of bytes (for header must be of HEADER_SIZE)
 * @return
 * 	- GENERAL_ERROR if error; see errno
 * 	- ALL_OK ok
 * 	- UPDATER_UNREC_CMD unrecognized command
 * */
uint8_t handle_upd_packet(uint8_t *buf, uint16_t count){
	uint8_t ret_val = 0;
	static uint32_t addr=NEW_CORE_START_ADDRESS+HEADER_SIZE;
	static uint32_t total_core_size = HEADER_SIZE; //in bytes
	if(count == HEADER_SIZE){ //if it is header
		if((buf[0] == 'v') && (buf[1] == 'e') && (buf[2] == 'r')){
			ret_val=write_header(buf);
		}
		else{
			return UPDATER_UNREC_CMD;
		}

	}
	else if(count == UPD_COMMAND_SIZE ){ //if command "del"
		if((buf[0] == 'd') && (buf[1] == 'e') && (buf[2] == 'l')){ //in case if corrupt data pc-updater sends a command to erase and then reboot
			ret_val=erase_sector(NEW_CORE_SECTOR);
		}
		else if((buf[0] == 'c') && (buf[1] == 'r') && (buf[2] == 'c')){ //if calc crc32 command "crc"
			g_vars.crc_val = upd_calc_crc(total_core_size);
		}
		else if((buf[0] == 'r') && (buf[1] == 'e') && (buf[2] == 'b')){ //if reboot command "reb"
			upd_finished();
		}
		else{
			return UPDATER_UNREC_CMD;
		}
	}
	else{ //if it is core chunk
		ret_val = write_core_chunk(buf, count, addr);
		if(ret_val == ALL_OK){
			addr+=count;
			total_core_size+=count;
		}
	}
	return ret_val;
}



/**
 * Writes a received header to a flash at NEW_CORE_START_ADDRESS
 * @params
 * 	- buf - a pointer to a buffer with header
 * @returns
 *  - ret_val - ALL_OK if success, GENERAL_ERROR if not(see errno)
 * */
uint8_t write_header(uint8_t *buf){
	FLASH_Status st;
	uint8_t ret_val = ALL_OK;
	for(int i=0; i<HEADER_SIZE;i+=4){
		st = FLASH_ProgramWord(NEW_CORE_START_ADDRESS+i,*(uint32_t *)&buf[i]);
		if(st != FLASH_COMPLETE){
			g_vars.errno = FLASH_WRITE_ERR;
			return GENERAL_ERROR;
		}
	}

	return ret_val;
}




/**
 * Writes a core chunk to a flash.
 * @params
 * 	- buf - pointer to a data
 * 	- count - amount of data in bytes
 * 	- addr - from where to start
 *
 * @returns
 *  - ret_val - ALL_OK if success, GENERAL_ERROR if not(see errno)
 * */
uint8_t write_core_chunk(uint8_t *buf, uint16_t count, uint32_t addr){
	FLASH_Status st;
	uint8_t ret_val = ALL_OK;
	for(int i=0; i<count;i++){
		st = FLASH_ProgramByte(addr+i,buf[i]);
		if(st != FLASH_COMPLETE){
			g_vars.errno = FLASH_WRITE_ERR;
			return GENERAL_ERROR;
		}
	}

	return ret_val;
}


/**
 * This function is called when a reboot signal from a pc-updater is received.
 * This means that all done.
 * */
void upd_finished(){
	FLASH_Lock();
	NVIC_SystemReset();
}



/**
 * Erases a flash sector
 * @param
 *  - sector - number of a sector to erase
 * @returns
 * 	- ALL_OK if all ok
 * 	- GENERAL_ERROR if error; see errno
 * */
uint8_t erase_sector(uint8_t sector){

#if !defined (STM32F4XX) && !defined (STM32F40XX)
#error "This procedure is for stm32f4. Otherwise check flash memory map"
#endif
	FLASH_Status st;
	uint8_t ret_val= 0;
	switch (sector) {
		case 1:
			st = FLASH_EraseSector(FLASH_Sector_1,VoltageRange_3);
			break;
		case 2:
			st = FLASH_EraseSector(FLASH_Sector_2,VoltageRange_3);
			break;
		case 3:
			st = FLASH_EraseSector(FLASH_Sector_3,VoltageRange_3);
			break;
		case 4:
			st = FLASH_EraseSector(FLASH_Sector_4,VoltageRange_3);
			break;
		case 5:
			st = FLASH_EraseSector(FLASH_Sector_5,VoltageRange_3);
			break;
		default:
			break;
	}
	if(st != FLASH_COMPLETE){
		g_vars.errno = FLASH_ERASE_ERR;
		ret_val = GENERAL_ERROR;
	}
	return ret_val;

}

/**
 * Calculates crc32 using  stm32's hardware capabilities and writes it to flash right after the last new core byte
 * @param
 *  - core_size - size of a core in bytes
 *  @returns crc32 value or 0xFFFFFFFF if flash error
 * */
uint32_t upd_calc_crc(uint32_t core_size){
	uint32_t ret_val = 0xFFFFFFFF;
	FLASH_Status st;
	ret_val = CRC_CalcBlockCRC((uint32_t *)NEW_CORE_START_ADDRESS,core_size);
	st = FLASH_ProgramWord(NEW_CORE_START_ADDRESS+core_size,ret_val);
	if(st != FLASH_COMPLETE){
		g_vars.errno = FLASH_WRITE_ERR;
		return 0xFFFFFFFF;
	}
	return ret_val;
}


