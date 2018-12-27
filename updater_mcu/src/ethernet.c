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
#include "mem.h"
#include "memp.h"
#include "udp.h"
#include "ethernetif.h"
#include "etharp.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <string.h>
#include <stdio.h>
#include "defs.h"


#define PHYAddress 0x00
#define LOCAL_FIRST_OCTET	192
#define LOCAL_SECOND_OCTET	168
#define LOCAL_THIRD_OCTET	1
#define LOCAL_FOURTH_OCTET	3

#define REMOTE_FIRST_OCTET	192
#define REMOTE_SECOND_OCTET	168
#define REMOTE_THIRD_OCTET	1
#define REMOTE_FOURTH_OCTET	4

#define DEFAULT_GATEWAY_FIRST_OCTET		192
#define DEFAULT_GATEWAY_SECOND_OCTET	168
#define DEFAULT_GATEWAY_THIRD_OCTET		1
#define DEFAULT_GATEWAY_FOURTH_OCTET	1


struct pbuf *p_tx;
struct udp_pcb *upcb_tx;
struct pbuf *p_upd_tx;
struct udp_pcb *upcb_upd;

uint8_t rx_buff[RX_PACKET_SIZE];
uint8_t rx_upd_buff[RX_PACKET_SIZE];
extern uint8_t g_upd;


void eth_config(void);
void lwip_init(void);
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void udp_upd_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void send_updater_ack(void);
void send_updater_nack(void);
void udp_configuration(void);
void udp_receive_configuration(void);




/**
 * @brief the function is called on system initialization to init eth/udp
 * @param none
 * @return none
 * */
void eth_initialisation(void){

	eth_config();
	lwip_init();
	udp_configuration();
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


	RCC_PLL3Config(RCC_PLL3Mul_10); //PLL3 clkout is 50MHz
	RCC_PLL3Cmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLL3RDY));
	RCC_MCOConfig(RCC_MCO_PLL3CLK);

	ETH_DeInit();
	ETH_SoftwareReset();
	while(ETH_GetSoftwareResetStatus() == SET);

//mac config

	ETH_StructInit(&e);
	e.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
	e.ETH_Speed = ETH_Speed_100M;
	e.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
	e.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;


////dma config
	e.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	e.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	e.ETH_FixedBurst = ETH_FixedBurst_Enable;
	e.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	e.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	e.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	ETH_Init(&e,PHYAddress);
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R,ENABLE);
}




/**
 * @brief initialization of lwip
 * @param none
 * @return none
 * */
void lwip_init(void)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	u8 macaddress[6]={0x00,0x22,0x11,0x22,0x11,0x22};

	mem_init(); //Initializes the dynamic memory heap defined by MEM_SIZE

	memp_init(); //Initializes the memory pools defined by MEMP_NUM_x

	IP4_ADDR(&ipaddr, LOCAL_FIRST_OCTET, LOCAL_SECOND_OCTET, LOCAL_THIRD_OCTET,LOCAL_FOURTH_OCTET);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, DEFAULT_GATEWAY_FIRST_OCTET, DEFAULT_GATEWAY_SECOND_OCTET, DEFAULT_GATEWAY_THIRD_OCTET, DEFAULT_GATEWAY_FOURTH_OCTET);

	Set_MAC_Address(macaddress);

	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);


	netif_set_default(&netif); //Registers the default network interface

	netif_set_up(&netif); //When the netif is fully configured this function must be called


}


/**
 * @brief Read a received packet from the Ethernet buffers and send it to the lwIP for handling.
 * @param none
 * @return none
 * */
void lwip_pkt_handle(void)
{
	ethernetif_input(&netif);
}


/**
 * @brief arp update periodic process
 * @param localtime - local time, based on systick counter
 * @return none
 * */
void lwip_periodic_handle(__IO u32 localtime)
{

	/* ARP periodic process */
	if (localtime - ARPTimer >= ARP_TMR_INTERVAL)
	{
		ARPTimer =  localtime;
		etharp_tmr();
	}
}


/**************UDP CONFIGURATION************************/

/**
 * @brief the function configures an udp stuff
 * @param none
 * @return none
 * */
void udp_configuration(void){

	udp_transmit_configuration();
	udp_receive_configuration();
	udp_update_configuration();

}

/**
 * @brief the function configures udp receive
 * @param none
 * @return none
 * */
void udp_receive_configuration(void){

	struct ip_addr local_ip_addr;
	IP4_ADDR(&local_ip_addr, LOCAL_FIRST_OCTET, LOCAL_SECOND_OCTET, LOCAL_THIRD_OCTET,LOCAL_FOURTH_OCTET);

	struct ip_addr remote_ip_addr;
	IP4_ADDR(&remote_ip_addr, REMOTE_FIRST_OCTET, REMOTE_SECOND_OCTET, REMOTE_THIRD_OCTET,REMOTE_FOURTH_OCTET);

	struct udp_pcb *upcb;
	err_t err;



	upcb = udp_new();
	if (upcb)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		err = udp_bind(upcb, &local_ip_addr, DEVICE_RX_PORT);//

		udp_connect(upcb, &remote_ip_addr, REMOTE_TX_PORT);

		if(err == ERR_OK)
		{
			/* Set a receive callback for the upcb */
			udp_recv(upcb, udp_receive_callback, NULL);
		}
	}





}
//--------------------------------------------------
void UDP_update_configuration()
{
	struct ip_addr local_ip_addr;
	IP4_ADDR(&local_ip_addr, LOCAL_FIRST_OCTET, LOCAL_SECOND_OCTET, LOCAL_THIRD_OCTET,LOCAL_FOURTH_OCTET);

	struct ip_addr remote_ip_addr;
	IP4_ADDR(&remote_ip_addr, 192,168,1,3);


	/*updater*/

	err_t upd_err;



	if (upcb_upd)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		upd_err = udp_bind(upcb_upd, &local_ip_addr, UPDATE_PORT_LOCAL);

		udp_connect(upcb_upd, &remote_ip_addr, UPDATE_PORT_LOCAL);

		if(upd_err == ERR_OK)
		{
			/* Set a receive callback for the upcb */
			udp_recv(upcb_upd, udp_upd_receive_callback, NULL);
		}
	}


}
//-------------------------



void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{


	memcpy(rx_buff, p->payload , p->len);
	//udp_disconnect(upcb);
	while( 0 == pbuf_free(p));
	g_vars.flag_udp_rec = 1;

}



void udp_upd_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{

	if(g_upd){
		uint8_t result=0;
		if(p->len<= sizeof(rx_upd_buff)){
			memcpy(rx_upd_buff, p->payload , p->len);
			result = handle_upd_packet(rx_buff,p->len);
			if(result == ALL_OK){
				send_updater_ack();
			}
			else{
				send_updater_nack();
			}
		}
	}
	else{
		if(p->len<= sizeof(rx_upd_buff)){
			memcpy(rx_upd_buff, p->payload , p->len);
			check_upd_request(rx_buff,p->len); //ret_val not used
		}

	}

	while( 0 == pbuf_free(p));

}


//-----------------------------------------------------------------------------------

void UDP_TransmitConfiguration()
{
	struct ip_addr local_ip_addr;
	IP4_ADDR(&local_ip_addr, 192,168,1,2);

	struct ip_addr remote_ip_addr;
	IP4_ADDR(&remote_ip_addr, 192,168,1,3);

	err_t err;

	/* Create a new UDP control block  */
	upcb_tx = udp_new();

	if (upcb_tx)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		err = udp_bind(upcb_tx, &local_ip_addr, DEVICE_TX_PORT);//

		if(err == ERR_OK) {
			udp_connect(upcb_tx, &remote_ip_addr, REMOTE_RX_PORT);
		}
	}



}

//-------------------------
uint8_t tx_buf[80];
void UDP_DataTransmit()
{
	memset(tx_buf,0xFD,sizeof(tx_buf));
	p_tx = pbuf_alloc(PBUF_TRANSPORT, TX_PACKET_SIZE , PBUF_RAM);
	memcpy(p_tx->payload, tx_buf,TX_PACKET_SIZE);
	udp_send(upcb_tx, p_tx);
	while( 0 == pbuf_free(p_tx));
}

//-----------------------------------------------------------------------------------




//------------------------------------------------------------------------------------
/**
 * Updater's ACK(0xAA) and NACK(0xBB) 3 bytes
 * */

void send_updater_ack(void){

	p_upd_tx = pbuf_alloc(PBUF_TRANSPORT, UPD_ACK_PKT_SIZE , PBUF_RAM);
	memset(p_upd_tx->payload, 0xAA,UPD_ACK_PKT_SIZE);
	udp_send(upcb_upd, p_upd_tx);
	while( 0 == pbuf_free(p_upd_tx));
}

void send_updater_nack(void){

	p_upd_tx = pbuf_alloc(PBUF_TRANSPORT, UPD_NACK_PKT_SIZE , PBUF_RAM);
	memset(p_upd_tx->payload, 0xBB,UPD_NACK_PKT_SIZE);
	udp_send(upcb_upd, p_upd_tx);
	while( 0 == pbuf_free(p_upd_tx));
}


