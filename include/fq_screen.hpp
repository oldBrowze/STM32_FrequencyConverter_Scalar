#pragma once

#include "stm32f1xx.h"

#define     _FUNCTION_DELAY  10
class LED_I
{
private:
    LED_I() = delete;
	/*
	ф-и работы с микросхемой MAX7219
	*/
    static const inline uint8_t symbols[]
    {
		0b00111111,//0
		0b00000110,//1
		0b01011011,//2
		0b01001111,//3
		0b01100110,//4
		0b01101101,//5
		0b01111101,//6
		0b00000111,//7
		0b01111111,//8
		0b01101111//9
	};
public:
	static inline volatile uint32_t __ticks{0};
	static void __delay(volatile uint32_t ms);
    static inline uint8_t 
        value{0}, digit{3};
    static void init();
	/*__STATIC_FORCEINLINE*/ static void send_command(const uint8_t address, const uint8_t command);
	/*{
		CS_ACTIVATE();

		SPI1->DR = (address << 8) | command; // отслыем первым байтом адрес регистра, втором - команду для регистра
		
		while(SPI1->SR & SPI_SR_BSY);
		CS_DIACTIVATE();
	}*/
};