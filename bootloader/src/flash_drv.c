/*
 * flash_drv.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */

//TODO read it and correct
#include "flash_drv.h"
#include "defs.h"
#include "string.h"
#include "stm32f10x_flash.h"

/**Checks if a new version of core exists.
 * return  - 0 if no header or error read
 * 			 1 if new  core detected
 * Header struct:
 * ver.ddmmyyyyssss
 * where v. is a const;
 * ddmmyyyy - date of build
 * ssss - 4 bytes for size of a core(in bytes)
 */
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
		g_vars.new_core_size = *(uint32_t*)&buf[12];
	}
	return ret_val;
}


///Copies a newly discovered version of a core to a place of an old one
void replace_core(void){

	uint32_t addr_cpy_from =NEW_CORE_FLASH_ADDR+CORE_HEADER_SIZE;
	uint32_t addr_cpy_to = CURRENT_CORE_FLASH_ADDR;
	uint8_t carrier_byte=0;
	FLASH_Status st;
	//clear space for a new version
	erase_sector(CURRENT_CORE_SECTOR);
	erase_sector(CURRENT_CORE_SECTOR+1);
	erase_sector(CURRENT_CORE_SECTOR+2);
	if(g_vars.errno != FLASH_OK){
		return;
	}

	for(int i=0; i<g_vars.new_core_size;i++){
		carrier_byte = *(uint8_t*)addr_cpy_from;
		st=FLASH_ProgramByte(addr_cpy_to,carrier_byte);
		if(st != FLASH_COMPLETE){
			g_vars.errno = FLASH_WR_ERR;
			return;
		}
		addr_cpy_from++;
		addr_cpy_to++;
	}

}

/**Erases a flash sector with core
 * param uint8_t sector - sector number to erase
 *
*/
void erase_sector(uint8_t sector){

#if !defined (STM32F4XX) && !defined (STM32F40XX)
#error "This procedure is for stm32f4. Otherwise check flash memory map"
#endif
	FLASH_Status st;
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
	}


}


