/*
 * defs.h
 *
 *  Created on: Sep 24, 2018
 *      Author: paanth
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SYSTEMTICK_PERIOD_MS	1
#define TX_PACKET_SIZE 			3
#define RX_PACKET_SIZE 			512
#define UPD_ACK_PKT_SIZE		3
#define UPD_NACK_PKT_SIZE		3
#define UPD_COMMAND_SIZE		3
#define HEADER_SIZE				16
#define NEW_CORE_START_ADDRESS  0x0801000
#define NEW_CORE_SECTOR			4
#define REMOTE_RX_PORT 			50001 //dst port
#define REMOTE_TX_PORT 			51004

#define DEVICE_TX_PORT 			50002 //src port
#define DEVICE_RX_PORT 			51003

#define UPDATE_PORT_LOCAL		55555
#define UPDATE_PORT_REMOTE		55556


typedef enum{
	ALL_OK = 0,
	FLASH_WRITE_ERR,
	FLASH_ERASE_ERR,
	UPDATER_UNREC_CMD,
	GENERAL_ERROR
}erro_t;
#endif /* DEFS_H_ */
