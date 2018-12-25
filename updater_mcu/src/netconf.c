#include "netconf.h"

#include "debug.h"

#include "mem.h"
#include "memp.h"
#include "udp.h"
#include "etharp.h"
#include "ethernetif.h"

#include <stdio.h>

//----------------------------
#include "defs.h"

struct netif netif;
__IO u32 ARPTimer = 0;


void ARPUpdate_Configuration();

//---------------------------

void LwIP_Init(void)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	u8 macaddress[6]={0x00,0x22,0x11,0x22,0x11,0x22};

	mem_init(); //Initializes the dynamic memory heap defined by MEM_SIZE

	memp_init(); //Initializes the memory pools defined by MEMP_NUM_x

	IP4_ADDR(&ipaddr, 192, 168, 1,2);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 1, 1);

	Set_MAC_Address(macaddress);

	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);


	netif_set_default(&netif); //Registers the default network interface

	netif_set_up(&netif); //When the netif is fully configured this function must be called

	ARPUpdate_Configuration();
}

//---------------------------

void LwIP_Pkt_Handle(void)
{
	ethernetif_input(&netif); //Read a received packet from the Ethernet buffers and send it to the lwIP for handling
}

//---------------------------

void LwIP_Periodic_Handle(__IO u32 localtime)
{

	/* ARP periodic process */
	if (localtime - ARPTimer >= ARP_TMR_INTERVAL)
	{
		ARPTimer =  localtime;
		etharp_tmr();
	}
}























