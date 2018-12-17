/*
 * main.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */
/* Includes */
#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "defs.h"
#include "sys_init.h"

uint32_t errno = 0;
/* Private function prototypes */
typedef void (*pFun)(void);
pFun jump_to_app;

void check_core(void);




/* Private functions */

void __jump(uint32_t addr){

	uint32_t jump_address = *(__IO uint32_t*)(addr+4);


	jump_to_app = (pFun)jump_address;
	__set_PRIMASK(1);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APPLICATION_OFFSET);
	__set_PRIMASK(0);
	__set_MSP(*(__IO uint32_t*)addr);
	jump_to_app();

}



void check_core(){
	uint8_t temp=0;
	FLASH_Unlock();
	temp = read_header();
	if(temp == 0){ //no core or an error occurred
		if(g_vars.errno != 0){ //if error
			//place here a code to notify people that shit happened
			return;
		}


		//all ok, just no new core

	}
	else{ //bingo! new core.
		replace_core();
		if(g_vars.errno != 0){ //if error
			//place here a code to notify people that shit happened
			return;
		}
		//otherwise erase already copied sector
		erase_sector(NEW_CORE_SECTOR);
		if(g_vars.errno != 0){ //if error
			//place here a code to notify people that shit happened
			return;
		}
	}
	FLASH_Lock();
}

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	sys_init();
	check_core();
	if(g_vars.errno == 0 ){
		__jump(CURRENT_CORE_FLASH_ADDR);
	}
	//should not get here
	while(1){
		//place a code to show that an error occured
		for(int i=0;i<1000000;i++){
			GPIO_ToggleBits(GPIOD,GPIO_Pin_15);
		}
	}

}






