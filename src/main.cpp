#include "stm32f1xx.h"
//#include "fq_screen.hpp"
#include "fq_rcc.hpp"
#include "fq_core.hpp"

int main()
{
    RCC_Init(); //инициализация тактирования
    FreqConverter::main_initialization();

    while(true)
    {
        TIM1->PSC = FreqConverter::get_PSC(TIM2->CNT);
    }
    return 0;
}