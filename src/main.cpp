#include "stm32f1xx.h"
#include "fq_rcc.hpp"
#include "fq_core.hpp"
//#include "fq_screen.hpp"

int main()
{
    RCC_Init();                             //инициализация тактирования
    //FreqConverter::main_initialization();   //инициализация частотника

    LED_I::init();

    //LED_I::send_command(LED_I::numbers_of_digit[5]);    //вывод цифры пять
    
    for(uint8_t index = 0; index <= 10; index++)
    {
        LED_I::send_command(LED_I::numbers_of_digit[index]);
        LED_I::__delay(500);
    }
    
    while(true);
    return 0;
}