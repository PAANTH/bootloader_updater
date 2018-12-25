/*
 * sys_init.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */


#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "defs.h"

/**
 * @brief all system init;
 * Called from main()
 * */
void sys_init(void){

	//led init led
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef g;
	g.GPIO_Mode = GPIO_Mode_Out_PP;
	g.GPIO_Pin = GPIO_Pin_15;
	g.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&g);

}
