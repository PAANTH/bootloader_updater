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
#include "flash_drv.h"
uint32_t errno = 0;


typedef void (*pFun)(void);
pFun jump_to_app;
void check_core(void);




/**
 * @brief jump to main program
 * @param addr - main program address
 * @return none
 * */
void __jump(uint32_t addr){

	uint32_t jump_address = *(__IO uint32_t*)(addr+4);


	jump_to_app = (pFun)jump_address;
	__set_PRIMASK(1);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, APPLICATION_OFFSET);
	__set_PRIMASK(0);
	__set_MSP(*(__IO uint32_t*)addr);
	jump_to_app();

}


/**
 * @brief the function checks whether there is a new core or not;
 * if it is, replaces old core and erases new core flash sectors
 * @param none
 * @return none
 * */
void check_core(void){
	uint8_t temp=0;
	FLASH_Unlock();
	temp = read_header();
	if(temp == 1){  //bingo! new core.

		temp = replace_core();
		if(temp != 0){ //if error
			//place here a code to notify people that shit happened
			return;
		}
		//otherwise erase already copied sector
		temp = erase_sector(NEW_CORE_FLASH_ADDR,PAGES_AMOUNT_TO_ERASE);
		if(temp != 0){ //if error
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
	if(errno == 0 ){
		__jump(CURRENT_CORE_FLASH_ADDR);
	}
	//should not get here
	while(1){
		//place a code to show that an error occured
		for(int i=0;i<1000000;i++);
		GPIO_SetBits(GPIOB,GPIO_Pin_15);

		for(int i=0;i<1000000;i++);
		GPIO_ResetBits(GPIOB,GPIO_Pin_15);

	}

}






