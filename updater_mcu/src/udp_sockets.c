#include "udp_sockets.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <string.h>
#include <stdio.h>

#include "defs.h"
#include "updater.h"
//-----------------------------------------------------------------------------------//
struct pbuf *p_tx;
struct udp_pcb *upcb_tx;
struct pbuf *p_upd_tx;
struct udp_pcb *upcb_upd;
//-----------------------------------------------------------------------------------//


void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void udp_upd_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void send_updater_ack(void);
void send_updater_nack(void);
//-----------------------------------------------------------------------------------//

uint8_t rx_buff[RX_PACKET_SIZE];
uint8_t rx_upd_buff[RX_PACKET_SIZE];
extern uint8_t g_upd;
extern glob_vars_t g_vars;
//-----------------------------------------------------------------------------------//

void UDP_ReceiveConfiguration()
{
	struct ip_addr local_ip_addr;
	IP4_ADDR(&local_ip_addr, 192,168,1,2);

	struct ip_addr remote_ip_addr;
	IP4_ADDR(&remote_ip_addr, 192,168,1,3);

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
	IP4_ADDR(&local_ip_addr, 192,168,1,2);

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

void UDP_Configuration()
{
	UDP_TransmitConfiguration();
	UDP_ReceiveConfiguration();
	UDP_update_configuration();

}


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

//------------------------------------------------------------------------------------







