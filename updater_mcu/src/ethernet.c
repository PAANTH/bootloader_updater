#include "ethernet.h"


#include "stm32f10x.h"
#include "stm32_eth.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "stm32f10x_tim.h" //arp
#include "udp_sockets.h"
#include "netconf.h"
#include "etharp.h"



//-----------------------------------------------------------------------------------

#define PHYAddress 0x00

//#define MDIO      GPIO_Pin_2
//#define PPS_OUT   GPIO_Pin_5
//#define TX_EN     GPIO_Pin_11
//#define TXD0      GPIO_Pin_12
//#define TXD1 	  GPIO_Pin_13
//#define MDC       GPIO_Pin_1
//#define CRS       GPIO_Pin_0
//#define REF_CLK   GPIO_Pin_1
//#define CRS_DV    GPIO_Pin_7
//#define RX_ER	  GPIO_Pin_10
//#define RXD0      GPIO_Pin_4
//#define RXD1      GPIO_Pin_5
//#define HW_RST    GPIO_Pin_8

//-----------------------------------------------------------------------------------

uint8_t flag_TransmitEnable = 0;

void ETH_Configuration();
void ARPUpdate_Configuration();
void ETHTimerTX_Configuration();

//-----------------------------------------------------------------------------------

void eth_initialisation()
{
	EthernetPacketsClear();

	ETH_Configuration();

	LwIP_Init();

	//ARPUpdate_Configuration();
	//ETHTimerTX_Configuration();

	UDP_Configuration();
}

//-----------------------------------------------------------------------------------

void ETH_Configuration()
{
	//gpio config
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOC, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_7; // interrupt /ref clk /mdio /crs_dv
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13; //rx_er / tx_en /tx_d0 / tx_d1
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5; //mdc /rxd0 /rxd1
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //hw reset
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);

	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

	//mac config

	GPIO_ResetBits(GPIOA, GPIO_Pin_8); //hw reset


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);



	GPIO_SetBits(GPIOA, GPIO_Pin_8);
	ETH_DeInit();

	ETH_SoftwareReset();
	while(ETH_GetSoftwareResetStatus() == SET);

	ETH_InitTypeDef ETH_InitStructure;
	ETH_StructInit(&ETH_InitStructure);

	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;

	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

//	u32 t1 = 0;//test phy
	ETH_Init(&ETH_InitStructure, PHYAddress);
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);

	//nvic
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
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

void ARPUpdate_Configuration()
{
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
//
//	TIM_TimeBaseInitTypeDef TIM7_InitStructure;
//	TIM_TimeBaseStructInit(&TIM7_InitStructure);
//	TIM7_InitStructure.TIM_Prescaler = 60000;
//	TIM7_InitStructure.TIM_Period = 0xFFFFFFFF;
//	TIM_TimeBaseInit(TIM7, &TIM7_InitStructure);
//
//	TIM_Cmd(TIM7, ENABLE);
}

//---------------------------

int arp_time = 0;
void ARPTableUpdate()
{
	arp_time--;
	if (arp_time==0) {
		etharp_tmr();
		arp_time = 10;
	}
}

//---------------------------

void EthernetPacketsClear()
{
//nothing for now
}

//---------------------------

void ETHTimerTX_Configuration()
{
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
//
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
//	TIM_TimeBaseInitStructure.TIM_Prescaler = 60000;
//	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFFFFFF;
//	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);

}
//---------------------------

int flag_t = 0;
void UDP_PacketsTransmit()
{
//	int i;
//	u8 crc = 0;
//	for(i=0;i<25;i++){
//		crc = crc + EthPacket_TX[i];
//	}
//	EthPacket_TX[25] = crc;
//	UDP_DataTransmit();
//	if (flag_t) {
//		P1_Set(0);
//		flag_t = 0;
//	}else {
//		P1_Set(1);
//		flag_t = 1;
//	}
}









