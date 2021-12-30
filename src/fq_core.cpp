#include "fq_core.hpp"
#include "fq_screen.hpp"
/*
TIM1 - формирование синусоиды на выходе. Частота зависит от ARR и PSC.
TIM3 - обработка энкодера(тайймер в режиме encoder mode)
*/
void FreqConverter::main_initialization()
{
    //ADC_initialize();
    timer_initialize();
    //button_initialize();
}

void FreqConverter::button_initialize()
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
    GPIOA->CRL |= (0x02 << GPIO_CRL_CNF0_Pos); //Вход Pull Up/Pull Down
    GPIOA->ODR |= (1 << 0); //Подтяжка вверх


    AFIO->EXTICR[0] &= ~(AFIO_EXTICR1_EXTI0); //Нулевой канал EXTI подключен к порту PA0
    
    //EXTI->RTSR |= EXTI_RTSR_TR0; //Прерывание по нарастанию импульса
    EXTI->FTSR |= EXTI_FTSR_TR0; //Прерывание по спаду импульса
    
    EXTI->PR = EXTI_PR_PR0;      //Сбрасываем флаг прерывания 
                                //перед включением самого прерывания
    EXTI->IMR |= EXTI_IMR_MR0;   //Включаем прерывание 0-го канала EXTI
    
    NVIC_EnableIRQ(EXTI0_IRQn);  //Разрешаем прерывание в контроллере прерываний
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
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN; // включаем тактирование портов А и В
    

    TIM1->PSC = get_PSC(_SIGNAL_FREQUENCY_MIN >> 2);
    TIM1->ARR = _ARR_VALUE-1;
    
    TIM1->CCER      = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;
    TIM1->CCER      |= TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE;             
    // включаем выход 1, 2, 3 каналов ножек как выход
    // устанавливаем норм. полярность(1 - высокий)
    TIM1->BDTR      = TIM_BDTR_MOE;                                                    // разрешили работу выводов


    TIM1->CCMR1     |= (0b110 << TIM_CCMR1_OC1M_Pos) | (0b110 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;   // установили работу каналов 1,2 как PWM1
    TIM1->CCMR2     |= (0b110 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;                                   // установили работу канала 3 как PWM1
    TIM1->CCMR1     |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

    TIM1->CR1       &= ~TIM_CR1_DIR;
    TIM1->CR1       &= ~TIM_CR1_CMS;
    TIM1->DIER      = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_UP_IRQn);


    // установим выводы каналов как alternative push-pull выходы
    GPIOA->CRH &=   ~(GPIO_CRH_MODE8 | GPIO_CRH_MODE9 | GPIO_CRH_MODE10); //mode 01 - alernative push-pull
    GPIOA->CRH |=   GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE10_1;

    GPIOA->CRH &=   ~(GPIO_CRH_CNF8 | GPIO_CRH_CNF9 | GPIO_CRH_CNF10);
    GPIOA->CRH |=   (GPIO_CRH_CNF8_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_1);//cnf 10 - mode output

    GPIOB->CRH &=   ~(GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15); //mode 01 - alernative push-pull
    GPIOB->CRH |=   GPIO_CRH_MODE13_1 | GPIO_CRH_MODE14_1 | GPIO_CRH_MODE15_1;

    GPIOB->CRH &=   ~(GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15);
    GPIOB->CRH |=   (GPIO_CRH_CNF13_1 | GPIO_CRH_CNF14_1 | GPIO_CRH_CNF15_1);//cnf 10 - mode output

    //Ноги энкодера для таймера TIM3
    GPIOA->CRL |=   (0b01 << GPIO_CRL_CNF6_Pos) | (0b01 << GPIO_CRL_CNF7_Pos);
    GPIOA->CRL |=   (0b00 << GPIO_CRL_MODE6_Pos) | (0b00 << GPIO_CRL_MODE7_Pos);


    TIM3->SMCR  =   TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1; //Encoder mode. Как вверх, так и вниз. Триггер на инкремент
    TIM3->CCER  =   ~(TIM_CCER_CC1P | TIM_CCER_CC2P); //полярность. Активный - high
    TIM3->CCMR1 |=   TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

    TIM3->ARR   =   _SIGNAL_FREQUENCY_MAX + 2; //
    
    TIM3->DIER |=   TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM3->CR1  |=   TIM_CR1_CEN;
    TIM1->CR1  |=   TIM_CR1_CEN;

    TIM3->CNT  = _SIGNAL_FREQUENCY_MIN >> 2;
}

extern "C" void TIM1_UP_IRQHandler()
{
    if (TIM1->SR & TIM_SR_UIF)
    {
        static uint8_t 
            _counter_phase_U{0},
            _counter_phase_V{100},
            _counter_phase_W{200};
        
        TIM1->CCR1 = FreqConverter::phases[_counter_phase_U];
        TIM1->CCR2 = FreqConverter::phases[_counter_phase_V];
        TIM1->CCR3 = FreqConverter::phases[_counter_phase_W];

        if(++_counter_phase_U == _DISCRETIZE)
            _counter_phase_U = 0;
        
        if(++_counter_phase_V == _DISCRETIZE)
            _counter_phase_V = 100;
        if(++_counter_phase_W == _DISCRETIZE)
            _counter_phase_W = 200;
        
        TIM1->SR = 0;
    }
}

extern "C" void TIM3_IRQHandler()
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        if(TIM3->CNT > _SIGNAL_FREQUENCY_MAX)
            TIM3->CNT = _SIGNAL_FREQUENCY_MAX;
        if(TIM3->CNT < _SIGNAL_FREQUENCY_MIN)
            TIM3->CNT = _SIGNAL_FREQUENCY_MIN;

        TIM1->PSC = FreqConverter::get_PSC(TIM3->CNT >> 2);
    }
}


//unused
/*
extern "C" void ADC1_2_IRQHandler()
{
    if(ADC1->SR & ADC_SR_EOC)
    {
        //TIM2->ARR = (_F_CPU / (FreqConverter::get_frequency() * _DISCRETIZE) - 1);
        //LED_I::value = FreqConverter::get_frequency();
    }
    ADC1->SR = 0x0;
}
*/