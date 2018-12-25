#include "premain.h"
#include "stm32f10x.h"

#define CORE_CLK 72000000 //Hz
#define PLL_M 10
#define PLL_N 300
#define PLL_P 2
#define PLL_Q 15



uint32_t systemCoreClock;

typedef enum{
	HSI,
	HSE,
	HSE_PLL,
	HSI_PLL
}ClockSource;


void FpuConfig(void);
void ResetRcc(void);
void ClockConfig(ClockSource clkSource);
void HsiConfig(void);
void HseConfig(void);
void HsiPllConfig(void);
void HsePllConfig(void);



void premain(void){
 
    ResetRcc();
    ClockConfig(HSE_PLL);
}







void ResetRcc(void){

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
    
    RCC->CFGR &= (uint32_t)0xF0FF0000;
    
    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;
    
    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;
    
    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;
    
    
    /* Reset PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEBFFFFFF;
    
    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x00FF0000;
    
    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;
}





void ClockConfig(ClockSource clkSource){
    switch (clkSource){
    case HSI:
        HsiConfig(); 
        break;
    case HSE:
        HseConfig(); 
        break;
    case HSI_PLL:
        HsiPllConfig(); 
        break;
    case HSE_PLL:
        HsePllConfig(); 
        break;
    default:
        HsiConfig();
        break;
    }
}



void HsiConfig(void){
    systemCoreClock = HSI_Value;
}



void HseConfig(void){
    uint32_t timeoutCounter = 0;
    uint32_t hseStatus = 0;

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);// Enable HSE

    while((hseStatus == 0) && (timeoutCounter != HSEStartUp_TimeOut)){// wait until HSE is started
        hseStatus = RCC->CR & RCC_CR_HSERDY;// Get HSE status
        timeoutCounter++;
    }

    if(hseStatus !=0){
        // Configure Flash prefetch, half-cycle access  and wait state
        FLASH->ACR = FLASH_ACR_PRFTBE |  FLASH_ACR_HLFCYA | FLASH_ACR_LATENCY_2; 
        
        //Set HSE as clock source
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));// Clear mask
        RCC->CFGR |= RCC_CFGR_SW_HSE;
        
        // wait until HSE is not selected as clk source
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSE);

        systemCoreClock = HSE_Value;
    }
    else{
        systemCoreClock = HSI_Value;
    }
}



void HsiPllConfig(void){
	//...
}



void HsePllConfig(void){
    uint32_t timeoutCounter = 0;
    uint32_t hseStatus = 0;

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);// Enable HSE

    while((hseStatus == 0) && (timeoutCounter != HSEStartUp_TimeOut)){// wait until HSE is started
        hseStatus = (RCC->CR & RCC_CR_HSERDY);// Get HSE status
        timeoutCounter++;
    }

    if(hseStatus !=0){
		    
        // Configure Flash prefetch, half-cycle access  and wait state
        FLASH->ACR = FLASH_ACR_PRFTBE |FLASH_ACR_LATENCY_2; //FLASH_ACR_HLFCYA |

        // HCLK = SYSCLK/1, PCLK2 = HCLK/1, PCLK1 = HCLK/2. (AHB-72; APB2-72; APB1-36MHz)
        RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2);

        // Configure the main PLL
        /* Configure PLLs ------------------------------------------------------*/
        /* Must use PLL2MUL for 25MHz HSE or can not achieve 72MHz See Clocks in RefMan*/
        /* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
        /* PREDIV1 configuration: PREDIV1CLK = PLL2 / 5 = 8 MHz */
        /* SYSCLC = PREDIV1CLK * PLLMUL = 8*9=72 MHz */
        RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |   //RESET
                                  RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
        
        RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
                                 RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);
       
        /* Enable PLL2 */
        RCC->CR |= RCC_CR_PLL2ON;
        /* Wait till PLL2 is ready */
        while((RCC->CR & RCC_CR_PLL2RDY) == 0);
        
        
       
        /* PLL configuration: PLLCLK = PREDIV1_CLK * 9 = 72 MHz */ 
        RCC->CFGR &= (uint32_t)~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL); //RESET
        RCC->CFGR |= (uint32_t)( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL9); 
        
        
       
      
        
        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;
        while((RCC->CR &RCC_CR_PLLRDY) == 0);

        /*USB 48MHz clk is by default if 72MHz syclk; else see RCC->CR-> OTGFSPRE*/
//////
       
        
        //Set HSE_PLL as clock source
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));// Clear mask
        RCC->CFGR |= RCC_CFGR_SW_PLL;
        
        // wait untill HSE_PLL is not selected as clk source
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);

        systemCoreClock = CORE_CLK;
        
       
    }
    else{
        systemCoreClock = HSI_Value;
    }
    

}







