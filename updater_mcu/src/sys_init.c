/*
 * sys_init.c
 *
 *  Created on: Dec 25, 2018
 *      Author: paanth
 */
#include "sys_init.h"
#include "stm32f10x.h"
#include "premain.h"
/**
 * @brief the function is called from main in the very beginning
 * */
void system_init(void){

	premain();
	SysTick_Config(72000); //1000 ticks period or 1ms
}
