#pragma     once
#include    "stm32f1xx.h"
#include    <cmath>
#include    <array>
#include    "core_cm3.h"
// компилятор должен оптимизировать, память не должна использоваться.

constexpr auto  _F_CPU                  = 72000000u;                // частота процессора
constexpr auto  _ARR_VALUE              = 256u;
constexpr auto  _SIGNAL_FREQUENCY_MAX   = 50u;                      // максимальная частота синусоиды
constexpr auto  _SIGNAL_FREQUENCY_MIN   = 50u;                      // минимальная частота синусоиды
constexpr auto  _DISCRETIZE             = 100u;                     // дискретизация синусоиды(количество точек)
constexpr auto  _VOLTAGE_ON_PORT        = 3.3f;                     // напряжение на порту(напряжение питания)
constexpr auto  _BITNESS                = 8u;                      // разрядность таймера[~цап]
constexpr auto  _AMPLITUDE              = ((1 << 16) - 1) / 2;      // максимальное значение таймера
constexpr auto  _ADC_MAX_VALUE          = (1 << 12) - 1;            // максимальное значение в АЦП
class FreqConverter
{
    using T = uint16_t;                                         // используемый тип данных у регистра
    FreqConverter() = delete;

public:
    static inline std::array<T, _DISCRETIZE> 
        phase_A 
        {   // полуволна( T = Пи )
            128, 136, 144, 152, 160, 167, 175, 182, 189, 196, 203, 209, 215, 221, 226, 
            231, 236, 240, 243, 247, 249, 251, 253, 254, 255, 255, 255, 254, 252, 250, 
            248, 245, 242, 238, 234, 229, 224, 218, 213, 206, 200, 193, 186, 179, 171, 
            163, 156, 148, 140, 132, 123, 115, 107, 99, 92, 84, 76, 69, 62, 55, 
            49, 42, 37, 31, 26, 21, 17, 13, 10, 7, 5, 3, 1, 0, 0, 
            0, 1, 2, 4, 6, 8, 12, 15, 19, 24, 29, 34, 40, 46, 52, 
            59, 66, 73, 80, 88, 95, 103, 111, 119, 127       
        },
        phase_B //Phase_A + 120 
        {
            238, 234, 229, 224, 219, 213, 207, 200, 193, 186, 179, 172, 164, 156, 148, 
            140, 132, 124, 116, 108, 100, 92, 84, 77, 70, 62, 56, 49, 43, 37, 
            32, 26, 22, 17, 14, 10, 7, 5, 3, 1, 0, 0, 0, 1, 2, 
            3, 6, 8, 11, 15, 19, 23, 28, 33, 39, 45, 51, 58, 65, 72, 
            80, 87, 95, 103, 111, 119, 127, 135, 143, 151, 159, 167, 174, 182, 189, 
            196, 203, 209, 215, 221, 226, 231, 236, 240, 243, 246, 249, 251, 253, 254, 
            255, 255, 255, 254, 252, 251, 248, 245, 242, 238
        },
        phase_C
        {
            18, 14, 10, 7, 5, 3, 1, 0, 0, 0, 1, 2, 3, 5, 8, 
            11, 15, 19, 23, 28, 33, 39, 45, 51, 58, 65, 72, 79, 87, 94, 
            102, 110, 118, 126, 134, 143, 151, 158, 166, 174, 181, 189, 195, 202, 209, 
            215, 220, 226, 231, 235, 239, 243, 246, 249, 251, 253, 254, 255, 255, 255, 
            254, 253, 251, 248, 246, 242, 238, 234, 230, 225, 219, 213, 207, 201, 194, 
            187, 180, 172, 164, 157, 149, 141, 133, 125, 116, 108, 101, 93, 85, 77, 
            70, 63, 56, 50, 43, 37, 32, 27, 22, 18
        };
    static void timer_initialize();
    static void array_initialize();
    static void ADC_initialize();
    static void main_initialization(); // все методы инициализации вызываются здесь.
    static inline uint8_t get_frequency()
    {
        return ((ADC1->DR / static_cast<float>(_ADC_MAX_VALUE)) * _SIGNAL_FREQUENCY_MAX);
    }
};


