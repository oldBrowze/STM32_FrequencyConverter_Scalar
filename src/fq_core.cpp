#include "fq_core.hpp"
/*
TIM1 - формирование синусоиды на выходе. Частота зависит от ARR и PSC.
TIM3 - обработка энкодера(таймер в режиме encoder mode)
*/

void FreqConverter::buzzer_toggle(bool turn)
{
    if((GPIOB->IDR & GPIO_IDR_IDR7) == turn)
        return;
    GPIOB->BSRR |= (turn) ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7;
}

void FreqConverter::buttons_initialize()
{
    /*
        Нужно, чтобы было включено тактирование AFIO, GPIOx
    */
    //KEY_BUTTON на энкодере0
    GPIOA->CRL |= (0b01 << GPIO_CRL_CNF3_Pos) | (0b00 << GPIO_CRL_MODE3_Pos);
                                    
    //BUTTON_REVERSE(с подтяжкой на плюс, при включенном реверсе - будет пин в лог. 0)1
    GPIOA->CRL |= (0b01 << GPIO_CRL_CNF4_Pos) | (0b00 << GPIO_CRL_MODE4_Pos);
    //GPIOA->BSRR |= GPIO_BSRR_BS4;


    //КНОПКА ОТКЛЮЧЕНИЯ FAULT/ЗВУКОВОЙ ОШИБКИ(по фронту импульса/с внешним прерыванием по изменении флага)2
    GPIOA->CRL |= (0b01 << GPIO_CRL_CNF5_Pos) | (0b00 << GPIO_CRL_MODE5_Pos);
    //GPIOA->BSRR |= GPIO_BSRR_BS5;


    //Настройка внешних прерываний
    AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI3_PA;                            //Нулевой канал EXTI подключен к порту PA0(KEY_BUTTON энкодера)
    AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PA | AFIO_EXTICR2_EXTI5_PA;
    /*
    Обработчик кнопки энкодера(A3) должен срабатывать при переходе с 1 в 0 - спад
    Обработчик кнопки реверса(A4) взводит флаг is_reverse и блокирует управление кнопками, пока двигатель не изменить скорость
    Обработчик кнопки fault(A5) должен срабатывать при переходе с 0 в 1(по фронту), спад должен быть пропущен - фронт
    */

    //EXTI->RTSR |= EXTI_RTSR_RT5;                      //Прерывание по фронту импульса(при переключении с лог. 0 на лог. 1 на кнопке FAULT)
    EXTI->FTSR |= EXTI_FTSR_FT3 | EXTI_FTSR_FT4 | EXTI_FTSR_FT5;                       //Прерывание по спаду импульса(при переключении с лог. 1 на лог. 0;)



    EXTI->PR &= EXTI_PR_PR3 | EXTI_PR_PR4 | \
                    EXTI_PR_PR5;                                       //Сбрасываем флаг прерывания 
    EXTI->IMR |= EXTI_IMR_MR3 | EXTI_IMR_MR4 | \
                    EXTI_IMR_MR5;                        //Включаем прерывание 0-го канала EXTI
    
    //Разрешаем прерывание в контроллере прерываний
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);                                   
    //NVIC_EnableIRQ(EXTI15_10_IRQn);

    FreqConverter::current_direction = (GPIOB->IDR & GPIO_IDR_IDR13) ? _DIR_RIGHT : _DIR_LEFT;
}

//Обработчики внешних прерываний

//при нажатии на кнопку энкодера происходит установка частоты на 30 Гц
extern "C" void EXTI3_IRQHandler()
{
    if(!FreqConverter::key_encoder_is_pressed)
    {
       TIM3->CNT = 30; 
       FreqConverter::key_encoder_is_pressed = true;
    }
    EXTI->PR = EXTI_PR_PR3;
}

extern "C" void EXTI4_IRQHandler()
{
    if(!FreqConverter::key_fault_is_pressed)
    {
        TIM3->CNT = 30;
        FreqConverter::key_fault_is_pressed = true;
        FreqConverter::is_fault = !FreqConverter::is_fault;
        FreqConverter::buzzer_toggle(FreqConverter::is_fault);    //выключаем буззер
    }

    EXTI->PR = EXTI_PR_PR4;
}

extern "C" void EXTI9_5_IRQHandler()
{
    //можно без проверки пина прерывания, PA5 подключен один
    if(!FreqConverter::key_reverse_is_pressed)
    {
        FreqConverter::key_reverse_is_pressed = true;
        FreqConverter::is_reverse = !FreqConverter::is_reverse;
        FreqConverter::buzzer_toggle(FreqConverter::is_reverse);    //выключаем буззер
    }
    /*
    if(FreqConverter::is_reverse)
        return;
    */
    //FreqConverter::current_frequency = TIM3->CNT; //либо реализовать через ссылку &

    EXTI->PR = EXTI_PR_PR5;
}

void FreqConverter::ADC_initialize()
{
    //порт A затактирован, АЦП затактирован
    /*
    Сделать: для 1 фазы ADC1, для второй фазы ADC2(чтобы работали параллельно)
    Для каждой фазы сделать прерывание и в нем проверку.

    */
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;                                      // т.к частота МК 72 МГц, а частота макс. АЦП - 14 МГц, то нужно использовать предделитель

    GPIOA->CRL &= ~((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) | \
                    (GPIO_CRL_MODE1 | GPIO_CRL_CNF1));                      // настраиваем порты PA0-PA1 как аналоговые


    ADC1->SMPR2 |= (0b011 << ADC_SMPR2_SMP0_Pos);                           // количество измерений
    ADC2->SMPR2 |= (0b011 << ADC_SMPR2_SMP0_Pos);


    ADC1->CR1 |= ADC_CR1_EOCIE;                                             // разрешаем прерывание от АЦП
    ADC1->CR2 |= ADC_CR2_CONT;                                              // непрерывная работа 

    ADC2->CR1 |= ADC_CR1_EOCIE;                                             // разрешаем прерывание от АЦП
    ADC2->CR2 |= ADC_CR2_CONT;                                              // непрерывная работа 

    __NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC1->SQR1 = 0;               // 1 пребразование
    //ADC1->SQR2 = 0;
    ADC1->SQR3 = 0;               // канал A0

    ADC2->SQR1 = 0;               // 1 пребразование
    //ADC1->SQR2 = 0;
    ADC2->SQR3 = 0x1;               // канал A1

    /*
    PA0 - измерение тока фазы 1
    (0b01 << ADC_JSQR_JL_Pos) - всего 2 преобразования(2 канала инжект.)
    (0x1 << ADC_JSQR_JSQ3_Pos) - канал PA1(измеряется первым), читается из JDR3, измерение тока фазы 2
    (0x2 << ADC_JSQR_JSQ4_Pos) - канал PA2(измеряется вторым), читается из JDR4, измерение температуры драйвера
    */
    ADC1->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
    ADC1->CR2 |= ADC_CR2_ADON;                                          // запускаем АЦП


    ADC2->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG;
    ADC2->CR2 |= ADC_CR2_ADON;  
    /*
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL); // ждем, пока АЦП откалибруется
    //задержка
    asm("NOP");
    asm("NOP");
    */
    ADC1->CR2 |= ADC_CR2_SWSTART; // запускаем АЦП
    ADC2->CR2 |= ADC_CR2_SWSTART; // запускаем АЦП
}

extern "C" void ADC1_2_IRQHandler()
{
    /*
    if(ADC1->SR & ADC_SR_EOC)
    {
        if((((static_cast<float>(ADC1->DR) / _ADC_VOLTAGE_STEP) - _ZERO_LEVEL_VOLATAGE) / _MVOLTS_PER_AMPER) > 0.03)
            FreqConverter::buzzer_toggle(true);
        //отключить драйвер
        ADC1->SR &= ~ADC_SR_EOC;
    }
    if(ADC1->SR & ADC_SR_JEOC)
    {
        //либо объединить условия
        if(ADC1->JDR3 > _MAX_CURRENT_PHASE) //если ток в фазе больше установленного
        //отключить драйвер
            return;
        if(ADC1->JDR4 > _MAX_TEMP_DRIVER)
        //отключить драйвер
            return;
        
        ADC1->SR &= ~ADC_SR_JEOC;
    }*/
    if(ADC1->SR & ADC_SR_EOC)
    {
        /*
        static uint8_t count = 1;
        static uint16_t _buffer = 0;
        if(++count <= 8)
        {
            _buffer += ADC1->DR;
        }
        else
        {
            FreqConverter::_ADC_VALUE = _buffer / 8;
            _buffer = 0;
            count = 0;
        }*/
        ADC1->SR &= ~ADC_SR_EOC;
    }

    if(ADC2->SR & ADC_SR_EOC)
    {
        static uint8_t count = 1;
        static uint16_t _buffer = 0;
        if(++count <= 8)
        {
            _buffer += ADC2->DR;
        }
        else
        {
            FreqConverter::_ADC_VALUE = _buffer / 8;
            _buffer = 0;
            count = 0;
        }
        ADC2->SR &= ~ADC_SR_EOC;
    }
}


void FreqConverter::timer_initialize()
{
    TIM1->PSC = get_PSC(_SIGNAL_FREQUENCY_MIN);
    TIM1->ARR = _ARR_VALUE-1;
    

    // включаем выход 1, 2, 3 каналов ножек как выход
    // устанавливаем норм. полярность(1 - высокий)
    TIM1->CCER      = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;
    TIM1->CCER      |= TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE;             

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


    TIM3->SMCR  =   TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1  | (0b100 << TIM_SMCR_TS_Pos); //Encoder mode. Как вверх, так и вниз. Триггер на инкремент
    TIM3->CCER  =   ~(TIM_CCER_CC1P | TIM_CCER_CC2P); //полярность. Активный - high
    TIM3->CCMR1 |=   TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

    TIM3->ARR   =   _SIGNAL_FREQUENCY_MAX;
    
    TIM3->DIER |=   TIM_DIER_UIE  | TIM_DIER_TIE;
    NVIC_EnableIRQ(TIM3_IRQn);

   // TIM3->EGR  = TIM_EGR_UG;
    TIM3->CR1  |=   TIM_CR1_CEN;
    TIM1->CR1  |=   TIM_CR1_CEN;

    TIM3->CNT  = 10;

    //период секунда
    TIM2->PSC = 1440-1;
    TIM2->ARR = 50000;
    //TIM2->
    TIM2->DIER = TIM_DIER_UIE;
    TIM2->CR1 = TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM2_IRQn);

}

//Обработчики прерываний

//прерывание по переполнению таймера-формирователя синусоиды
extern "C" void TIM1_UP_IRQHandler()
{
    if (TIM1->SR & TIM_SR_UIF)
    {
        static uint16_t 
            _counter_phase_U{_START_VALUE_PHASE_U},
            _counter_phase_V{_START_VALUE_PHASE_V},
            _counter_phase_W{_START_VALUE_PHASE_W};
        
        //ОБРАБОТЧИК КНОПКИ РЕВЕРСА(проверка по значению)
        if(FreqConverter::current_direction == _DIR_RIGHT)
        {
            TIM1->CCR1 = FreqConverter::phases[_counter_phase_U];       //фаза U
            TIM1->CCR3 = FreqConverter::phases[_counter_phase_W];       //фаза W
        }
        else if(FreqConverter::current_direction == _DIR_LEFT)
        {
            TIM1->CCR1 = FreqConverter::phases[_counter_phase_W];       //фаза U
            TIM1->CCR3 = FreqConverter::phases[_counter_phase_U];       //фаза W 
        }
        TIM1->CCR2 = FreqConverter::phases[_counter_phase_V];       //фаза V



        if(++_counter_phase_U == _DISCRETIZE)
            _counter_phase_U = 0;
        if(++_counter_phase_V == _DISCRETIZE)
            _counter_phase_V = 0;
        if(++_counter_phase_W == _DISCRETIZE)
            _counter_phase_W = 0;
        
        TIM1->SR = 0;
    }
}

//обработчик таймера-энкодера
extern "C" void TIM3_IRQHandler()
{
    if(FreqConverter::is_reverse == true)
    {
        TIM3->SR = 0;
        TIM3->CNT = FreqConverter::current_frequency;
        return;
    }
    if(FreqConverter::is_fault == false)
    {
        if(TIM3->SR & TIM_SR_UIF)
        {
            TIM3->CNT = (TIM3->CR1 & TIM_CR1_DIR) ? 10 : 90; 
        }
        if(TIM3->SR & TIM_SR_TIF)
        {
            TIM1->PSC = FreqConverter::get_PSC(TIM3->CNT);
        }
    }
    //проверить после того, как отлючить надписи на дисплее
    else
        TIM3->CNT = (TIM3->CR1 & TIM_CR1_DIR) ? (TIM3->CNT + 1): (TIM3->CNT - 1);
    TIM3->SR = 0;
}

extern "C" void TIM2_IRQHandler()
{
    //FreqConverter::is_fault = !FreqConverter::is_fault;
    //FreqConverter::buzzer_toggle(FreqConverter::is_fault);
    /*if(FreqConverter::is_reverse == false)
        return;
    static bool count_down = true;
    static uint16_t _freq = FreqConverter::current_frequency;

    if(count_down == true) //тормозим
    {
        if(_freq - 10 > 10)
        {
            _freq -= 10;
            TIM1->PSC = FreqConverter::get_PSC(_freq);
        }
        else
        {
            FreqConverter::current_direction = (FreqConverter::current_direction == _DIR_LEFT) ? _DIR_RIGHT : _DIR_LEFT;
            count_down = false;
        }
    }
    else  //ускоряемся
    {
        if(_freq >= FreqConverter::current_frequency)
        {
            TIM1->PSC = FreqConverter::get_PSC(FreqConverter::current_frequency);
            FreqConverter::is_reverse = false;
        }
        else
        {
            TIM1->PSC = FreqConverter::get_PSC(_freq);
            _freq += 10;
        }
    }*/
    TIM2->SR = 0;
}