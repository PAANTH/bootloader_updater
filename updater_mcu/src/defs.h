/*
 * defs.h
 *
 *  Created on: Sep 24, 2018
 *      Author: paanth
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SYSTEMTICK_PERIOD_MS	1


#define UPD_COMMAND_SIZE		3
#define HEADER_SIZE				16
#define NEW_CORE_START_ADDRESS  0x0801000
#define NEW_CORE_SECTOR			4


typedef enum{
	ALL_OK = 0,
	FLASH_WRITE_ERR,
	FLASH_ERASE_ERR,
	UPDATER_UNREC_CMD,
	GENERAL_ERROR
}erro_t;
#endif /* DEFS_H_ */
