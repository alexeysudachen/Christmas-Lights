
#pragma once

#ifndef STM32F030
#define STM32F030
#endif

#include <stm32f0xx.h>
#include "metacpp.hxx"
#include "cortex_m.hxx"

namespace cortex_m0
{  
  
  struct stm32f030f4 : mcu<stm32f030f4>
  {  

    template<int leg_number, int tim_channel>
    struct maps_to
    {
      using index = mp::number<leg_number>;
      static constexpr int channel = tim_channel;
    };

    typedef mp::list<
      tim_def<1, 2, RCC_APB2ENR_TIM1EN, 4, mp::list<maps_to<13,-1>,maps_to<14,-3>,maps_to<17,2>,maps_to<18,3>> >,
      tim_def<3, 1, RCC_APB1ENR_TIM3EN, 4, mp::list<maps_to<12,1>,maps_to<13,2>,maps_to<14,4>> >,
      tim_def<14,1, RCC_APB1ENR_TIM14EN,1, mp::list<maps_to<10,1>,maps_to<13,1>,maps_to<14,1>> >,
      tim_def<16,2, RCC_APB2ENR_TIM16EN,1, mp::list<maps_to<12,1>> >,
      tim_def<17,2, RCC_APB2ENR_TIM17EN,1, mp::list<maps_to<13,1>> >
    > timers;

    typedef mp::list<
      leg_def<2, 'F',0 >,
      leg_def<3, 'F',1 >,
      leg_def<6, 'A',0, af_adc_in<0>, af_usart_cts<1,1> >,
      leg_def<7, 'A',1, af_adc_in<1>, af_usart_rts<1,1>, af_eventout<0> >,
      leg_def<8, 'A',2, af_adc_in<2>, af_usart_tx<1,1> >,
      leg_def<9, 'A',3, af_adc_in<3>, af_usart_rx<1,1> >,
      leg_def<10,'A',4, af_adc_in<4>, af_usart_ck<1,1>, af_tim<4,14 /*1*/>, af_spi_nss<0,1> >,
      leg_def<11,'A',5, af_adc_in<5>, af_spi_sck<0,1> >,
      leg_def<12,'A',6, af_adc_in<6>, af_spi_miso<0,1>, af_tim<1,3 /*1*/>, af_tim<5,16 /*1*/>,  af_eventout<6>, af_tim_bkin<2,1> >,
      leg_def<13,'A',7, af_adc_in<7>, af_spi_mosi<0,1>, af_tim<1,3 /*2*/>, af_tim<2,1 /*-1*/>, af_tim<4,14 /*1*/>, af_tim<5,17 /*1*/>, af_eventout<6> >,
      leg_def<14,'B',1, af_adc_in<9>, af_tim<0,14 /*1*/>, af_tim<1,3 /*4*/>, af_tim<2,1 /*-3*/> >,
      leg_def<17,'A',9,               af_usart_tx<1,1>, af_tim<2,1 /*2*/>, af_i2c_scl<4,1> >,
      leg_def<18,'A',10,              af_usart_rx<1,1>, af_tim<2,1 /*3*/>, af_i2c_sda<4,1>, af_tim_bkin<0,17> >,
      leg_def<19,'A',13,              af_swdio<0>, af_ir_out<1> >,
      leg_def<20,'A',14,              af_swclk<0>, af_usart_tx<1,1> >
    > legs;
  
  };

}

using cortex_m0::stm32f030f4;
