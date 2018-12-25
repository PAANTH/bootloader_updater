/*
 * updater.h
 *
 *  Created on: Oct 31, 2018
 *      Author: paanth
 */

#ifndef UPDATER_H_
#define UPDATER_H_
#include "inttypes.h"
uint8_t check_upd_request(uint8_t *buf, uint16_t count);
uint8_t handle_upd_packet(uint8_t *buf, uint16_t count);
void upd_finished();
#endif /* UPDATER_H_ */
