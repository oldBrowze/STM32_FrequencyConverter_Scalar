#include "fq_core.hpp"
#include "fq_screen.hpp"
/*
TIM1 - формирование синусоиды на выходе. Частота зависит от ARR и PSC.
SysTick - загрузка значений в регистр сравнения TIM1, формирует частоту синусоиды
*/
void FreqConverter::main_initialization()
{
    ADC_initialize();
    timer_initialize();
}

void FreqConverter::ADC_initialize()
{
    //порт A затактирован.
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6; // т.к частота МК 72 МГц, а частота макс. АЦП - 14 МГц, то нужно использовать предделитель
    GPIOA->CRL &= ~ (GPIO_CRL_MODE1 | GPIO_CRL_CNF1);   // настраиваем порт PA1 как аналоговый

    ADC1->SMPR2 = (0b001 << ADC_SMPR2_SMP1_Pos);
    ADC1->CR1 = ADC_CR1_EOCIE; // разрешаем прерывание от АЦП
    ADC1->CR2 = ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG | ADC_CR2_CONT; // работа от внешнего события, 

    __NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC1->SQR1 = 0x0;
    ADC1->SQR2 = 0x0;
    ADC1->SQR3 = 1;

   
    ADC1->CR2 |= ADC_CR2_ADON; // запускаем АЦП
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL); // ждем, пока АЦП откалибруется
    ADC1->CR2 |= ADC_CR2_SWSTART; // запускаем АЦП
}

void FreqConverter::timer_initialize()
{
   // RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // включаем TIM1 (тактирование от APB2 - 72 MHz)
    //RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_IOPAEN; // включаем тактирование портов А и В
    

    TIM1->PSC = (F_CPU / _SIGNAL_FREQUENCY_MIN / _ARR_VALUE / _DISCRETIZE) - 1;
    TIM1->ARR = _ARR_VALUE-1;
    
    TIM1->CCER      = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P;                   
    // включаем выход 1, 2, 3 каналов ножек как выход
    // устанавливаем норм. полярность(1 - высокий)
    TIM1->BDTR      = TIM_BDTR_MOE;                                                    // разрешили работу выводов
    TIM1->CCMR1     = (0b110 << TIM_CCMR1_OC1M_Pos) /*| (0b110 << TIM_CCMR1_OC2M_Pos)*/ | TIM_CCMR1_OC1PE;   // установили работу каналов 1,2 как PWM1
    //TIM1->CCMR2     |= (0b110 << TIM_CCMR2_OC3M_Pos);                                   // установили работу канала 3 как PWM1
    //TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    TIM1->CR1       &= ~TIM_CR1_DIR;
    TIM1->CR1       &= ~TIM_CR1_CMS;
    TIM1->DIER      = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_UP_IRQn);


    // установим выводы каналов как alternative push-pull выходы
    GPIOA->CRH &=   ~(GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10); //mode 01 - alernative push-pull
    GPIOA->CRH |=   GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE10_1;

    GPIOA->CRH &=   ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10);
    GPIOA->CRH |=   (GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1);//cnf 10 - mode output

    TIM1->CR1  |=   TIM_CR1_CEN;
/*
    TIM2->ARR = ((_F_CPU / (1 << _BITNESS)) - 1);
    TIM2->PSC = 0;

    TIM2->DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;
*/
}

extern "C" void TIM1_UP_IRQHandler()
{
    if (TIM1->SR & TIM_SR_UIF)
    {
        static uint16_t 
            _counter_phase{0};
        TIM1->CCR1 = FreqConverter::phase_A[_counter_phase];
        //TIM1->CCR2 = FreqConverter::phase_B[_counter_phase];
        //TIM1->CCR3 = FreqConverter::phase_C[_counter_phase];

        if(++_counter_phase == _DISCRETIZE)
            _counter_phase = 0;
        TIM1->SR = 0;
    }
}
/*
extern "C" void TIM2_IRQHandler()
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        static uint16_t 
            _counter_phase{0};
        TIM1->CCR1 = FreqConverter::phase_A[_counter_phase];
        //TIM1->CCR2 = FreqConverter::phase_B[_counter_phase];
        //TIM1->CCR3 = FreqConverter::phase_C[_counter_phase];

        if(++_counter_phase == _DISCRETIZE)
            _counter_phase = 0;
        TIM2->SR &= ~TIM_SR_UIF;
    }
}
*/
//unused
extern "C" void ADC1_2_IRQHandler()
{
    if(ADC1->SR & ADC_SR_EOC)
    {
        //TIM2->ARR = (_F_CPU / (FreqConverter::get_frequency() * _DISCRETIZE) - 1);
        LED_I::value = FreqConverter::get_frequency();
    }
    ADC1->SR = 0x0;
}