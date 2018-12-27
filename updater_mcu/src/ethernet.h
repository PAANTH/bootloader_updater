/*
 * ethernet.h
 *
 *  Created on: Dec 25, 2018
 *      Author: paanth
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_
#include "stdint.h"

void eth_initialisation(void);
void lwip_pkt_handle(void);
void lwip_periodic_handle(u32 localtime);
#endif /* ETHERNET_H_ */

