#pragma once
#include "stm32f1xx.h"
#include <array>

//класс для работы с микросхемой MAX7219
class LED_I
{
private:
    LED_I() = delete;
public:
	static constexpr std::array<uint8_t, 11> numbers_of_digit
	{
		0b00111111,	//0
		0b00000110,	//1
		0b01011011,	//2
		0b01001111,	//3
		0b01100110,	//4
		0b01101101,	//5
		0b01111101,	//6
		0b00000111,	//7
		0b01111111,	//8
		0b01101111,	//9
		0b10000000	//dp
	};
	static inline volatile uint32_t __ticks{0};

	//функция задержки
	static void __delay(volatile uint32_t);

	//функция инициализации
    static void init();

	//функция отправки команды через протокол SPI микросхеме
	/*__STATIC_FORCEINLINE*/ static void send_command(const uint8_t &);
	/*__STATIC_FORCEINLINE*/ static void set_digit(const uint8_t &);
};