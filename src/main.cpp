#include "stm32f1xx.h"
#include "fq_rcc.hpp"
#include "fq_core.hpp"
//#include "fq_screen.hpp"

int main()
{
    RCC_Init();                             //инициализация тактирования
    //FreqConverter::main_initialization();   //инициализация частотника
    LED_I::init();
    /*
    LED_I::send_command(0xC, 0x1);          //выход из сна(normal mode)
    LED_I::send_command(0xA, 0xB);          //яркость 23/32
    LED_I::send_command(0xB, 0x3);          //количество разрядов (4 - 1 = 0x3)
    LED_I::send_command(0xF, 0x1);          //дисплей-тест
    */
    /*
    max7219_send_to_all(REG_SHUTDOWN,    0x01); // restart
    max7219_send_to_all(REG_DECODE_MODE, 0x00); // use normal mode
    max7219_send_to_all(REG_SCAN_LIMIT,  0x07); // use all lines
    */
    LED_I::send_command(0x0C, 0x1);         // выход из сна
    LED_I::send_command(0x09, 0x00);        // режим декодирования
    LED_I::send_command(0x0B, 0x03);        // 4 разряда
    LED_I::send_command(0x0A, 0x0B);        //яркость 23/32


    LED_I::send_command(0x01, 0x01);
    LED_I::send_command(0x02, 0x02);
    LED_I::send_command(0x03, 0x04);
    LED_I::send_command(0x04, 0x08);
    LED_I::send_command(0x05, 0x0F);
    LED_I::send_command(0x06, 0x10);
    LED_I::send_command(0x07, 0x12);
    LED_I::send_command(0x08, 0x14);
    while(true);
    return 0;
}