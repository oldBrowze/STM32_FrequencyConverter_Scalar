#include "fq_screen.hpp"

void LED_I::__delay(volatile uint32_t ms)
{
    uint32_t _temp = __ticks;
    while((__ticks - _temp) < ms);
}

extern "C" void SysTick_Handler()
{
    ++LED_I::__ticks;
}

void LED_I::init()
{
    //тактирование порта B уже включено таймером
    //RCC->APB2ENR = RCC_APB2ENR_IOPBEN;
    SysTick_Config(72000000 / 1000);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN; // запускаем тактирование SPI, PB
    
    SPI1->CR1 |= (1 << SPI_CR1_DFF_Pos)         // 16-битная передача
                | (1 << SPI_CR1_SSM_Pos)        // программный SS
                | (1 << SPI_CR1_SSI_Pos)        // программный SS
                | (1 << SPI_CR1_BIDIMODE_Pos)   // transmit-only
                | (1 << SPI_CR1_BIDIOE_Pos)
                | (0 << SPI_CR1_LSBFIRST_Pos)   // старший бит вперед
                | (0b111 << SPI_CR1_BR_Pos)     // F_CPU/64
                | (1 << SPI_CR1_MSTR_Pos)       // master mode
                | (0 << SPI_CR1_CPHA_Pos)       // SPI 0:0
                | (0 << SPI_CR1_CPOL_Pos);
    SPI1->CR1 |= (1 << SPI_CR1_SPE_Pos);        // запуск SPI

    //SCK
    GPIOA->CRL |= (0b10 << GPIO_CRL_CNF5_Pos) | (0b11 << GPIO_CRL_MODE5_Pos);
  
    //CS
    GPIOB->CRH |= (0b00 <<GPIO_CRH_CNF11_Pos) | (0b11 << GPIO_CRH_MODE11_Pos);
    GPIOB->BSRR |= GPIO_BSRR_BS11;

    //MOSI
    GPIOA->CRL |= (0b10 << GPIO_CRL_CNF7_Pos) | (0b11 << GPIO_CRL_MODE7_Pos);
    while(SPI1->SR & SPI_SR_MODF);
}

void LED_I::send_command(const uint8_t address, const uint8_t command)
{
    GPIOB->BRR |= GPIO_BRR_BR11;

    SPI1->DR = (address << 8) | command; // отслыем первым байтом адрес регистра, втором - команду для регистра

    while (!(SPI1->SR & SPI_SR_TXE));
    while(SPI1->SR & SPI_SR_BSY);

    GPIOB->BSRR |= GPIO_BSRR_BS11;

    __delay(1);
}
