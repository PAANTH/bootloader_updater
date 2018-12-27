/*
 * defs.h
 *
 *  Created on: Sep 24, 2018
 *      Author: paanth
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SYSTEMTICK_PERIOD_MS	1

typedef enum{
	ALL_OK = 0,
	FLASH_WRITE_ERR,
	FLASH_ERASE_ERR,
	UPDATER_UNREC_CMD,
	GENERAL_ERROR
}erro_t;
#endif /* DEFS_H_ */
