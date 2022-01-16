#pragma once
#include "stm32f1xx.h"
#include <array>

//класс для работы с микросхемой MAX7219
class LED_I
{
private:
    LED_I() = delete;
public:
	//массив бинарных значений для семисегментного индикатора
	static constexpr std::array<uint8_t, 12> numbers_of_digit
	{
	//dp_g_f_e_d_c_b_a

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
		0b10000000,	//dp
		0b01110001	//F
	};
	//функция инициализации
    static void init();

	/*__STATIC_FORCEINLINE*/ static void send_command(const uint8_t &, bool = true);		// посылает команду на микросхема
	/*__STATIC_FORCEINLINE*/ static void update_value(const uint8_t &);						// обновляет значение на инидкаторе
	static uint8_t get_value(const uint8_t &, const uint8_t &);
};