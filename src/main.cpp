#include "stm32f1xx.h"
//#include "fq_core.hpp"
#include "fq_screen.hpp"
#include "fq_rcc.hpp"

int main()
{
    RCC_Init(); //инициализация тактирования
    //FreqConverter::main_initialization();
    LED_I::init();

    LED_I::send_command(0xF, 0x0); // отключаем тест. режим
    LED_I::send_command(0x9, 0x00); // режим декодирования
    LED_I::send_command(0xB, 0x3); // количество разрядов
    LED_I::send_command(0xA, 0x7); // яркость
    LED_I::send_command(0xC, 0x1); // включаем

    LED_I::send_command(0x2, 0x2); // единица


    while(true);
    return 0;
}