/*
 * ethernet.c
 *
 *  Created on: Dec 25, 2018
 *      Author: paanth
 */
#include "ethernet.h"
#include "stm32f10x.h"
#include "stm32_eth.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
//#include "udp_sockets.h"
//#include "netconf.h"
#include "etharp.h"


#define PHYAddress 0x00

void eth_config(void);



/**
 * @brief the function is called on system initialization to init eth/udp
 * @param none
 * @return none
 * */
void eth_initialisation(void){

	eth_config();
	LwIP_Init();
	UDP_Configuration();
}


/**
 * @brief setup pins and mac on mcu
 * @param none
 * @return none
 * */
void eth_config(void)
{
//gpio config
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
							| RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);


	GPIO_InitTypeDef g;
	g.GPIO_Speed = GPIO_Speed_50MHz;
	g.GPIO_Mode = GPIO_Mode_AF_PP;
	g.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_8; //RMII_MDIO, MCO
	GPIO_Init(GPIOA,&g);

	g.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13; //RMII_TX_EN, RMII_TXD0, RMII_TXD1
	GPIO_Init(GPIOB, &g);

	g.GPIO_Pin = GPIO_Pin_1; //RMII_MDC
	GPIO_Init(GPIOC,&g);

	g.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	g.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_7; //RMII_REF_CLK, RMII_CRS_DV(DP83848 sig. "receive data valid")
	GPIO_Init(GPIOA,&g);

	g.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; //RMII_RXD0, RMII_RXD1
	GPIO_Init(GPIOC,&g);
//TODO why need PC8?
	g.GPIO_Mode = GPIO_Mode_Out_PP;
	g.GPIO_Pin = GPIO_Pin_8; //??
	GPIO_Init(GPIOC,&g);

//nvic
	NVIC_InitTypeDef n;
	//TODO check if it work without this line; should
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x00);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	n.NVIC_IRQChannel = ETH_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 2;
	n.NVIC_IRQChannelSubPriority = 0;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);

//eth clk
	ETH_InitTypeDef e;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx,ENABLE);
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);


//mac config

//dma config




}


void ETH_IRQHandler()
{
	while(ETH_GetRxPktSize() != 0)
  {
    LwIP_Pkt_Handle();
  }

  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}


//---------------------------



int arp_time = 0;
void arp_tbl_upd(void)
{
	arp_time--;
	if (arp_time==0) {
		etharp_tmr();
		arp_time = 10;
	}
}












