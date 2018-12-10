/*
 * global_vars.h
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */

#ifndef GLOBAL_VARS_H_
#define GLOBAL_VARS_H_

#include "stm32f10x.h"
typedef struct{
	int8_t errno;
	uint32_t new_core_size;
}glob_vars_t;

typedef enum{
	FLASH_OK = 0,
	FLASH_RD_ERR = 1,
	FLASH_WR_ERR = 2,
	FLASH_ERASE_ERR = 3
}errno_vals_t;

void init_global_vars();

#endif /* GLOBAL_VARS_H_ */
