#include "stm32f1xx.h"
#include "fq_rcc.hpp"
#include "fq_core.hpp"
//#include "fq_screen.hpp"

int main()
{
    RCC_Init();                             //инициализация тактирования
    FreqConverter::main_initialization();   //инициализация частотника
    //LED_I::init();

    LED_I::send_command(0xC, 0x1);          //выход из сна(normal mode)
    LED_I::send_command(0xA, 0xB);          //яркость 23/32
    LED_I::send_command(0xB, 0x3);          //количество разрядов (4 - 1 = 0x3)
    LED_I::send_command(0xF, 0x1);          //дисплей-тест
    
    while(true);
    return 0;
}