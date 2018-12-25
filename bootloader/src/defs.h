/*
 * defs.h
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SYSTEMTICK_PERIOD_MS 		1
#define TX_PACKET_SIZE 				80
#define RX_PACKET_SIZE 				12
#define NEW_CORE_SECTOR 			4
#define CURRENT_CORE_SECTOR 		1
#define NEW_CORE_FLASH_ADDR     	0x08010000
#define CURRENT_CORE_FLASH_ADDR 	0x08004000
#define CORE_HEADER_SIZE 			16
#define APPLICATION_OFFSET  		0x4000

typedef enum{
	FLASH_OK = 0,
	FLASH_RD_ERR = 1,
	FLASH_WR_ERR = 2,
	FLASH_ERASE_ERR = 3
}errno_vals_t;
#endif /* DEFS_H_ */

/**
*ss
*/