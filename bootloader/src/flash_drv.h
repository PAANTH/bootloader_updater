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
void replace_core(void);
void erase_sector(uint8_t sector);

#endif /* FLASH_DRV_H_ */
