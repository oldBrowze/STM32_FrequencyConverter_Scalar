#pragma once
#include <stm32f1xx.h>

__attribute__((always_inline)) static inline void RCC_Init() // устанавливаем на 72 МГц
{
    RCC->CR |= (1 << RCC_CR_HSEON_Pos); //запускаем HSE-генератор
    while(!(RCC->CR & (1 << RCC_CR_HSERDY_Pos))); //пока HSE генератор не запустился, сидим в цикле.

    RCC->CFGR |= (1 << RCC_CFGR_PLLSRC_Pos) | (0b0111 << RCC_CFGR_PLLMULL_Pos); // запустили тактирование HSE от PLL .. PLL = 0111 => умножение на 9


    while(RCC->CR & (1 << RCC_CR_PLLRDY_Pos)); //пока тактирование от PLL не началось, сидим в цикле.
    
    //так как частота ядра 48 MHz <= frequency <= 72 MHz, нужно установить 2 цикла ожидания FLASH
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    //т.к шина APB1 не может работать на частоте выше, чем 36 МГц, установим делитель на 2. Остальные шины(APB2, AHP) оставляем без делителей
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_HPRE_DIV1;

    RCC->CR |= RCC_CR_PLLON; // включаем PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL; // переключаемся на PLL.
    while (!((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)) //пока не перкключились на PLL, сидим в цикле

    RCC->CR &= ~RCC_CR_HSION; // отключаемся от HSI

    //тактирование нужных шин
    RCC->APB2ENR = RCC_APB2ENR_AFIOEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_TIM1EN | \
                    RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR = RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;
}