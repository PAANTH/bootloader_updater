/*
 * sys_init.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */


#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "defs.h"
//TODO Check led pin
void sys_init(void){

	//led init led
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef g;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_Pin = GPIO_Pin_15;
	g.GPIO_PuPd = GPIO_PuPd_UP;
	g.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&g);

}
