/*
 * ethernet.h
 *
 *  Created on: Dec 25, 2018
 *      Author: paanth
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_

void eth_initialisation(void);
void ARPTableUpdate(void);
void EthernetPacketsClear(void);
void UDP_PacketsTransmit(void);
void INTRPT_ETH_disable(void);
void INTRPT_ETH_enable(void);

#endif /* ETHERNET_H_ */

