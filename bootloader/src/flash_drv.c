/*
 * flash_drv.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */


#include "flash_drv.h"
#include "defs.h"
#include "string.h"
#include "stm32f10x_flash.h"

#define STM32F107_LAST_PAGE_ADDR 	0x0803F800
#define STM32F107_PAGE_SIZE			0x800
extern uint32_t errno;
uint32_t new_core_size = 0;

/**
 * @brief  checks if a new version of core exists.
 * Header struct:
 * ver.ddmmyyyyssss
 * where ver. is a const;
 * ddmmyyyy - date of build
 * ssss - 4 bytes for size of a core(in bytes).
 * @param - none
 * @return  - 0 if no header; 1 if new  core detected
 * */
uint8_t read_header(void){
	uint8_t ret_val=0;
	uint8_t buf[16];
	uint32_t addr=0;

	memset(buf,0,sizeof(buf));
	addr = NEW_CORE_FLASH_ADDR;
	for(int i;i<16;i++, addr++){
		buf[i] = *(volatile uint8_t*)addr;
	}
	if((buf[0] == 'v')&&(buf[1] == 'e')&&(buf[2] == 'r')){
		ret_val = 1;
		new_core_size = *(uint32_t*)&buf[12];
	}
	return ret_val;
}


/**
 * @brief copies a newly discovered version of a core to a place of an old one.
 * @param none
 * @return	0-if all ok; 1-if error
 * */
uint8_t replace_core(void){

	uint32_t addr_cpy_from =NEW_CORE_FLASH_ADDR+CORE_HEADER_SIZE;
	uint32_t addr_cpy_to = CURRENT_CORE_FLASH_ADDR;
	uint32_t carrier_word=0;
	uint8_t func_ret_val = 0;
	FLASH_Status st;
	//clear space for a new version
	func_ret_val = erase_sector(CURRENT_CORE_FLASH_ADDR,8);
	if(func_ret_val){
		errno = FLASH_ERASE_ERR;
		return 1;
	}

	//start copying
	for(int i=0; i<new_core_size;i++){
		carrier_word = *(uint32_t*)addr_cpy_from;
		st=FLASH_ProgramWord(addr_cpy_to,carrier_word);
		if(st != FLASH_COMPLETE){
			errno = FLASH_WR_ERR;
			return 1;
		}
		addr_cpy_from+=4;
		addr_cpy_to+=4;
	}
	return 0;
}

/**
 * @brief erases a flash sector with core
 * @param start_page_addr - address of a first page to erase
 * @param amount of pages to erase
 * @return ret_val-0 if all ok;
 * 1 if amount of pages is too high
 * 2 if flash erase error
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


