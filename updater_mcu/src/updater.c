/*
 * updater.c
 *	A complementary part of bootloader-updater-pc bunch for stm32f407.
 *  Created on: Oct 31, 2018
 *      Author: paanth
 */
#include "updater.h"
#include "defs.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "stm32f10x_crc.h"
#include "stm32f10x_tim.h"

#define UPD_COMMAND_SIZE		3
#define HEADER_SIZE				16
#define NEW_CORE_START_ADDRESS  0x08010000


uint8_t write_header(uint8_t *buf);
uint8_t write_core_chunk(uint8_t *buf, uint16_t count, uint32_t addr);
uint8_t erase_sector(uint8_t sector);
uint32_t upd_calc_crc(uint32_t core_size);

//flag that core update is in process. Once set, must never be reset
uint8_t g_upd=0;
uint32_t errno=0;

/**
 * @brief checks if in received buffer is 3 bytes and they are "upd".
 * This means that a new version of a core is ready to be transmitted.
 * @param buf - pointer to buffer
 * @param counter - amount of bytes (for request must be 3)
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
 * @brief handles a chunk of data. Decides whether it is a header or a core chunk. Writes to flash
 * @param buf - pointer to buffer
 * @param counter - amount of bytes (for header must be of HEADER_SIZE)
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
			uint8_t pages_amount = total_core_size/2048; //2Kb page size
			if(pages_amount < 1){
				pages_amount = 1;
			}
			ret_val=erase_sector(NEW_CORE_START_ADDRESS,pages_amount);
		}
		else if((buf[0] == 'c') && (buf[1] == 'r') && (buf[2] == 'c')){ //if calc crc32 command "crc"
			crc_val = upd_calc_crc(total_core_size);
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
 * @brief writes a received header to a flash at NEW_CORE_START_ADDRESS
 * @param buf - a pointer to a buffer with header
 * @return ret_val - ALL_OK if success, GENERAL_ERROR if not(see errno)
 * */
uint8_t write_header(uint8_t *buf){
	FLASH_Status st;
	uint8_t ret_val = ALL_OK;
	for(int i=0; i<HEADER_SIZE;i+=4){
		st = FLASH_ProgramWord(NEW_CORE_START_ADDRESS+i,*(uint32_t *)&buf[i]);
		if(st != FLASH_COMPLETE){
			errno = FLASH_WRITE_ERR;
			return GENERAL_ERROR;
		}
	}

	return ret_val;
}




/**
 * @brief writes a core chunk to a flash.
 * @param buf - pointer to a data
 * @param count - amount of data in bytes
 * @param addr - from where to start
 * @return ret_val - ALL_OK if success, GENERAL_ERROR if not(see errno)
 * */
uint8_t write_core_chunk(uint8_t *buf, uint16_t count, uint32_t addr){
	FLASH_Status st;
	uint8_t ret_val = ALL_OK;
	for(int i=0; i<count*4;i+=4){
		st = FLASH_ProgramWord(addr+i,(uint32_t *)&buf[i]);
		if(st != FLASH_COMPLETE){
			errno = FLASH_WRITE_ERR;
			return GENERAL_ERROR;
		}
	}

	return ret_val;
}


/**
 * @brief this function is called when a reboot signal from a pc-updater is received.
 * This means that all done.
 * @param none
 * @return none
 * */
void upd_finished(void){
	FLASH_Lock();
	NVIC_SystemReset();
}



/**
 * @brief erases a flash sector with core
 * @param start_page_addr - address of a first page to erase
 * @param amount of pages to erase
 * @return ret_val
 * - 0 if all ok
 * - 1 if amount of pages is too high
 * - 2 if flash erase error
*/
uint8_t erase_sector(uint32_t start_page_addr, uint8_t pages_amount){

	FLASH_Status st;
	uint32_t addr = start_page_addr;

	//assert
	uint32_t last_addr = start_page_addr+pages_amount*STM32F107_PAGE_SIZE;
	if(last_addr > STM32F107_LAST_PAGE_ADDR){
		return 1;
	}

	for(uint8_t i=0;i<pages_amount;i++){
		st = FLASH_ErasePage(addr);
		if(st != FLASH_COMPLETE){
				return 2;
		}
		addr+=STM32F107_PAGE_SIZE;
	}
	return 0;

}


/**
 * @brief calculates crc32 using  stm32's hardware capabilities and writes it to flash right after the last new core byte
 * @param core_size - size of a core in bytes
 * @return crc32 value or 0xFFFFFFFF if flash error
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


