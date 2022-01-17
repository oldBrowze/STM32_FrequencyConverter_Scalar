#include "stm32f1xx.h"
#include "fq_rcc.hpp"
#include "fq_core.hpp"
//#include "fq_screen.hpp"

int main()
{
    RCC_Init();                             //инициализация тактирования
    FreqConverter::main_initialization();   //инициализация частотника

    while(true);
    return 0;
}