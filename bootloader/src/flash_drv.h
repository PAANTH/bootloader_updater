/*
 * flash_drv.h
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */

#ifndef FLASH_DRV_H_
#define FLASH_DRV_H_

#include "inttypes.h"
uint8_t read_header(void);
uint8_t replace_core(void);
uint8_t erase_sector(uint32_t start_page_addr, uint8_t pages_amount);

#endif /* FLASH_DRV_H_ */
