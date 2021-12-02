#pragma once
#include "stm32f1xx.h"


//класс для работы с микросхемой MAX7219
class LED_I
{
private:
    LED_I() = delete;
public:
	static inline volatile uint32_t __ticks{0};

	//функция задержки
	static void __delay(volatile uint32_t);

	//функция инициализации
    static void init();

	//функция отправки команды через протокол SPI микросхеме
	/*__STATIC_FORCEINLINE*/ static void send_command(const uint8_t, const uint8_t);
};