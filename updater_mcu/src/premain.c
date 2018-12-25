#include "premain.h"

#define CORE_CLK 168000000 //Hz   was originally 120MHz??
#define PLL_M 16
#define PLL_N 336//240
#define PLL_P 2
#define PLL_Q 7

#include "stm32f4xx.h"

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
    FpuConfig();
    ResetRcc();
    ClockConfig(HSE_PLL);
}



void FpuConfig(void){
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));// FPU is on, set CP10 and CP11 Full Access
}



void ResetRcc(void){

    RCC->CR |= (uint32_t)0x00000001;// Set HSION bit

    RCC->CFGR = 0x00000000;// Reset CFGR register

    RCC->CR &= (uint32_t)0xFEF6FFFF;// Reset HSEON, CSSON and PLLON bits

    RCC->PLLCFGR = 0x24003010;// Reset PLLCFGR register

    RCC->CR &= (uint32_t)0xFFFBFFFF;// Reset HSEBYP bit

    RCC->CIR = 0x00000000;// Disable all interrupts
}



void ClockConfig(ClockSource clkSource){
    switch (clkSource){
    case HSI:
        HseConfig();
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
        HseConfig();
        break;
    }
}



void HsiConfig(void){
    systemCoreClock = HSI_VALUE;
}



void HseConfig(void){
    uint32_t timeoutCounter = 0;
    uint32_t hseStatus = 0;

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);// Enable HSE

    while((hseStatus == 0) && (timeoutCounter != HSE_STARTUP_TIMEOUT)){// wait until HSE is started
        hseStatus = RCC->CR & RCC_CR_HSERDY;// Get HSE status
        timeoutCounter++;
    }

    if(hseStatus !=0){

        FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_0WS; // Configure Flash prefetch, Instruction cache, Data cache and wait state

        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));// Clear mask
        RCC->CFGR |= RCC_CFGR_SW_HSE;// Set HSE as clk source

        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_HSE);// whait until HSE is not selected as clk source

        systemCoreClock = HSE_VALUE;
    }
    else{
        systemCoreClock = HSI_VALUE;
    }
}



void HsiPllConfig(void){
	//...
}



void HsePllConfig(void){
    uint32_t timeoutCounter = 0;
    uint32_t hseStatus = 0;

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);// Enable HSE

    while((hseStatus == 0) && (timeoutCounter != HSE_STARTUP_TIMEOUT)){// wait until HSE is started
        hseStatus = (RCC->CR & RCC_CR_HSERDY);// Get HSE status
        timeoutCounter++;
}

    if(hseStatus !=0){
		    
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;// Select regulator voltage output Scale 1 mode
        PWR->CR |= PWR_CR_VOS;

        // HCLK = SYSCLK/1, PCLK2 = HCLK/2, PCLK1 = HCLK/4.
        RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV4);

        // Configure the main PLL
        RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

        RCC->CR |= RCC_CR_PLLON;// Enable the main PLL

        while((RCC->CR & RCC_CR_PLLRDY) == 0);// Wait till the main PLL is ready

        FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;//Configure Flash prefetch, Instruction cache, Data cache and wait state

        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));// Select the main PLL as system clock source
        RCC->CFGR |= RCC_CFGR_SW_PLL;

        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);// Wait till the main PLL is used as system clock source

        systemCoreClock = CORE_CLK;
    }
    else{
        systemCoreClock = HSI_VALUE;
    }

}







