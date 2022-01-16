#include "fq_screen.hpp"

/*
void LED_I::__delay(volatile uint32_t ms)
{
    uint32_t _temp = __ticks;
    while((__ticks - _temp) < ms);
}

extern "C" void SysTick_Handler()
{
    ++LED_I::__ticks;
}
*/
void LED_I::init()
{
    //тактирование порта B уже включено таймером
    //RCC->APB2ENR = RCC_APB2ENR_IOPBEN;
    SysTick_Config(72000000 / 700);
    AFIO->MAPR = AFIO_MAPR_SPI1_REMAP | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN; // запускаем тактирование SPI, PB
    

    SPI1->CR1 = (0 << SPI_CR1_DFF_Pos)         // 16-битная передача
                | (1 << SPI_CR1_SSM_Pos)        // программный SS
                | (1 << SPI_CR1_SSI_Pos)        // программный SS
                | (1 << SPI_CR1_BIDIMODE_Pos)   // transmit-only
                | (1 << SPI_CR1_BIDIOE_Pos)
                | (0 << SPI_CR1_LSBFIRST_Pos)   // старший бит вперед
                | SPI_CR1_BR                    // F_CPU/64
                | (1 << SPI_CR1_MSTR_Pos)       // master mode
                | (0 << SPI_CR1_CPHA_Pos)       // SPI 0:0
                | (0 << SPI_CR1_CPOL_Pos);
    //SPI1->CR2 |= SPI_CR2_SSOE;
    SPI1->CR1 |= (1 << SPI_CR1_SPE_Pos);        // запуск SPI

    //SCK
    GPIOB->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);    // обнуление
    GPIOB->CRL |= (0b10 << GPIO_CRL_CNF3_Pos) | (0b11 << GPIO_CRL_MODE3_Pos);
  
    //CS

    //74hc595 сегментный
    GPIOB->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_MODE11);    // обнуление
    GPIOB->CRH |= (0b00 <<GPIO_CRH_CNF11_Pos) | (0b11 << GPIO_CRH_MODE11_Pos);

    //74hc595 разрядный
    GPIOB->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);    // обнуление
    GPIOB->CRH |= (0b00 <<GPIO_CRH_CNF10_Pos) | (0b11 << GPIO_CRH_MODE10_Pos);

    //MOSI
    GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);    // обнуление
    GPIOB->CRL |= (0b10 << GPIO_CRL_CNF5_Pos) | (0b11 << GPIO_CRL_MODE5_Pos);

    while(SPI1->SR & SPI_SR_MODF);
    GPIOB->BRR = GPIO_BRR_BR11 | GPIO_BRR_BR10;
}

void LED_I::send_command(const uint8_t &command, bool in_segment)
{
    SPI1->DR = command;

    while (!(SPI1->SR & SPI_SR_TXE));
    while(SPI1->SR & SPI_SR_BSY);

    GPIOB->BSRR |= (in_segment) ? GPIO_BSRR_BS11 : GPIO_BSRR_BS10;
    GPIOB->BRR |= (in_segment) ? GPIO_BRR_BR11 : GPIO_BRR_BR10;
}
/*
void LED_I::update_value(const uint8_t &value)
{
    LED_I::send_command(0x4, false);                                                //4 разряд
    LED_I::send_command(numbers_of_digit[11]);                                      //Буква F

    if(value < 10)
    {
        LED_I::send_command(0x3, false);                                            //3 разряд
        LED_I::send_command(numbers_of_digit[value % 10] | numbers_of_digit[10]);   //Цифра + точка

        LED_I::send_command(0x2, false);                                            //2 разряд
        LED_I::send_command(numbers_of_digit[0]);                                   //цифра 0

        LED_I::send_command(0x1, false);                                            //1 разряд
        LED_I::send_command(numbers_of_digit[0]);                                   //цифра 0
    }
    else if(value < 100)
    {
        LED_I::send_command(0x3, false);                                            //3 разряд
        LED_I::send_command(numbers_of_digit[(99 % 100 / 10)]);                    //Цифра

        LED_I::send_command(0x2, false);                                            //2 разряд
        LED_I::send_command(numbers_of_digit[value % 10] | numbers_of_digit[10]);   //Цифра + точка

        LED_I::send_command(0x1, false);                                            //1 разряд
        LED_I::send_command(numbers_of_digit[0]);                                   //цифра 0
    }
    else
    {
        LED_I::send_command(0x3, false);                                            //3 разряд
        LED_I::send_command(numbers_of_digit[value / 1000]);                        //Цифра

        LED_I::send_command(0x2, false);                                            //2 разряд
        LED_I::send_command(numbers_of_digit[value % 100 / 10]);                    //Цифра 

        LED_I::send_command(0x1, false);                                            //1 разряд
        LED_I::send_command(numbers_of_digit[value % 10] | numbers_of_digit[10]);   //цифра + точка
    }
}
*/
uint8_t LED_I::get_value(const uint8_t &value, const uint8_t &digit)
{
    switch(digit)
    {
        case 0b0001: return value % 10;
        case 0b0010: return (value < 10) ? 0 : value % 100 / 10;
        case 0b0100: return (value < 100) ? 0 : value % 1000 / 100;
        case 0b1000: return 11;
        default: return 0;
    }
}

extern "C" void SysTick_Handler()
{
    static uint8_t current_digit{1};
    static auto& value = TIM3->CNT;
    
    LED_I::send_command(current_digit, false); // разряд
    LED_I::send_command(LED_I::numbers_of_digit[LED_I::get_value(value, current_digit)]);

    current_digit = (current_digit > 4) ? 1 : current_digit << 1;
    
   /*
   switch(current_digit)
   {
        case 4:
            LED_I::send_command(0x8, false);                                                
            LED_I::send_command(LED_I::numbers_of_digit[11]);         
            break;
        case 3:
            if(value < 10)
            {
                LED_I::send_command(0x4, false);                                            //3 разряд
                LED_I::send_command(LED_I::numbers_of_digit[value % 10] | LED_I::numbers_of_digit[10]);   //Цифра + точка
            }
            else if(value < 100)
            {
                LED_I::send_command(0x4, false);                                            //3 разряд
                LED_I::send_command(LED_I::numbers_of_digit[(99 % 100 / 10)]);                    //Цифра
            }
            else
            {
                LED_I::send_command(0x4, false);                                            //3 разряд
                LED_I::send_command(LED_I::numbers_of_digit[value / 1000]);                        //Цифра
            }
            break;
        case 2:
            if(value < 10)
            {
                LED_I::send_command(0x2, false);                                            //2 разряд
                LED_I::send_command(LED_I::numbers_of_digit[0]);                                   //цифра 0
            }
            else if(value < 100)
            {
                LED_I::send_command(0x2, false);                                            //2 разряд
                LED_I::send_command(LED_I::numbers_of_digit[value % 10] | LED_I::numbers_of_digit[10]);   //Цифра + точка
            }
            else
            {
                LED_I::send_command(0x2, false);                                            //2 разряд
                LED_I::send_command(LED_I::numbers_of_digit[value % 100 / 10]);                    //Цифра 
            }
            break;
        case 1:
            if(value < 10)
            {
                LED_I::send_command(0x1, false);                                            //1 разряд
                LED_I::send_command(LED_I::numbers_of_digit[0]);                                   //цифра 0
            }
            else if(value < 100)
            {
                LED_I::send_command(0x1, false);                                            //1 разряд
                LED_I::send_command(LED_I::numbers_of_digit[0]);                                   //цифра 0
            }
            else
            {
                LED_I::send_command(0x1, false);                                            //1 разряд
                LED_I::send_command(LED_I::numbers_of_digit[value % 10] | LED_I::numbers_of_digit[10]);   //цифра + точка
            }
            break;
   }
   current_digit = (current_digit > 4) ? 1 : current_digit + 1;
   */
}