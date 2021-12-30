#pragma     once
#include    "stm32f1xx.h"
#include    <cmath>
#include    <array>
#include    "core_cm3.h"

// компилятор должен оптимизировать, память не должна использоваться С++17
constexpr auto  _BITNESS                = 8u;                       // разрядность цап
constexpr auto  _F_CPU                  = 72000000u;                // частота процессора
constexpr auto  _ARR_VALUE              = (1 << _BITNESS);          // значение ARR(максимальное значение - 1 опорной таблицы значений)
constexpr auto  _SIGNAL_FREQUENCY_MAX   = 100u << 2;                // максимальная частота синусоиды(умноженная на 4 - шаг энкодера = 4)
constexpr auto  _SIGNAL_FREQUENCY_MIN   = 30u << 2;                 // минимальная частота синусоиды(умноженная на 4 - шаг энкодера = 4)
constexpr auto  _DISCRETIZE             = 100u;                     // дискретизация синусоиды(количество точек)

//настройки АЦП
constexpr auto  _VOLTAGE_ON_PORT        = 3.3f;                     // напряжение на порту(напряжение питания)
constexpr auto  _AMPLITUDE              = ((1 << 16) - 1) / 2;      // максимальное значение таймера
constexpr auto  _ADC_MAX_VALUE          = (1 << 12) - 1;            // максимальное значение в АЦП

class FreqConverter
{
    using T = uint16_t;                                             // используемый тип данных у регистра
    FreqConverter() = delete;                                       // коструктор по умолчанию удален. Класс без объектов
public:
    static inline std::array<T, _DISCRETIZE> phases//(по 150 точек на каждую полуволну)
    {
        128, 136, 144, 152, 160, 167, 175, 182, 189, 196, 203, 209, 215, 221, 226, 
        231, 236, 240, 243, 247, 249, 251, 253, 254, 255, 255, 255, 254, 252, 250, 
        248, 245, 242, 238, 234, 229, 224, 218, 213, 206, 200, 193, 186, 179, 171, 
        163, 156, 148, 140, 132, 123, 115, 107, 99, 92, 84, 76, 69, 62, 55, 
        49, 42, 37, 31, 26, 21, 17, 13, 10, 7, 5, 3, 1, 0, 0, 
        0, 1, 2, 4, 6, 8, 12, 15, 19, 24, 29, 34, 40, 46, 52, 
        59, 66, 73, 80, 88, 95, 103, 111, 119, 127
/*
128, 	132, 	136, 	139, 	143, 	147, 	151, 	155, 	159, 	163, 
166, 	170, 	174, 	177, 	181, 	184, 	187, 	191, 	194, 	197, 
200, 	202, 	205, 	208, 	210, 	213, 	215, 	217, 	219, 	221, 
223, 	224, 	226, 	227, 	229, 	230, 	231, 	232, 	233, 	234, 
235, 	235, 	236, 	236, 	237, 	237, 	237, 	238, 	238, 	238, 
238, 	238, 	238, 	238, 	238, 	237, 	237, 	237, 	237, 	237, 
236, 	236, 	236, 	236, 	235, 	235, 	235, 	235, 	234, 	234, 
234, 	234, 	234, 	234, 	234, 	234, 	234, 	234, 	234, 	234, 
234, 	234, 	234, 	235, 	235, 	235, 	235, 	236, 	236, 	236, 
236, 	237, 	237, 	237, 	237, 	237, 	238, 	238, 	238, 	238, 
238, 	238, 	238, 	238, 	237, 	237, 	237, 	236, 	236, 	235, 
235, 	234, 	233, 	232, 	231, 	230, 	229, 	227, 	226, 	224, 
223, 	221, 	219, 	217, 	215, 	213, 	210, 	208, 	205, 	202, 
200, 	197, 	194, 	191, 	187, 	184, 	181, 	177, 	174, 	170, 
166, 	163, 	159, 	155, 	151, 	147, 	143, 	139, 	136, 	132, 
128, 	123, 	119, 	116, 	112, 	108, 	104, 	100, 	96, 	92, 
89, 	85, 	81, 	78, 	74, 	71, 	68, 	64, 	61, 	58, 
55, 	53, 	50, 	47, 	45, 	43, 	40, 	38, 	36, 	34, 
32, 	31, 	29, 	28, 	26, 	25, 	24, 	23, 	22, 	21, 
20, 	20, 	19, 	19, 	18, 	18, 	18, 	17, 	17, 	17, 
17, 	17, 	17, 	17, 	17, 	18, 	18, 	18, 	18, 	18, 
19, 	19, 	19, 	19, 	20, 	20, 	20, 	20, 	21, 	21, 
21, 	21, 	21, 	21, 	21, 	21, 	21, 	21, 	21, 	21, 
21, 	21, 	21, 	20, 	20, 	20, 	20, 	19, 	19, 	19, 
19, 	18, 	18, 	18, 	18, 	18, 	17, 	17, 	17, 	17, 
17, 	17, 	17, 	17, 	18, 	18, 	18, 	19, 	19, 	20, 
20, 	21, 	22, 	23, 	24, 	25, 	26, 	28, 	29, 	31, 
32, 	34, 	36, 	38, 	40, 	42, 	45, 	47, 	50, 	53, 
55, 	58, 	61, 	64, 	68, 	71, 	74, 	78, 	81, 	85, 
89, 	92, 	96, 	100, 	104, 	108, 	112, 	116, 	119, 	123
*/
    };
    static void timer_initialize();                                 // инициализация таймеров(TIM1 & TIM3)   
    static void ADC_initialize();                                   // инициализация АЦП для оцифровки значений шунтов на фазах
    static void main_initialization();                              // все методы инициализации вызываются здесь

    static void button_initialize();                                // инициализация кнопок   
    constexpr static inline uint32_t get_PSC(const uint8_t &value)  // пересчет PSC для таймера TIM1(для изменения частоты синусоиды. Частота не всегда равна аргументу)
    {
        return ((F_CPU / value / _ARR_VALUE / _DISCRETIZE) - 1);
    }
};


