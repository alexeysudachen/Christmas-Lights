
#pragma once

#include "metacpp.hxx"

namespace cortex_m0
{
  inline namespace abi {
    typedef struct { unsigned int quot, rem; } r_t;     
    extern "C" __value_in_regs r_t __aeabi_uidivmod(unsigned int, unsigned int);
  }
  
  template<int pin_number, int port, int channel, typename... alternate_funcs> 
  struct leg_def
  {
    using index = mp::number<pin_number>;
    enum { gpio_port = port, gpio_channel = channel, no = pin_number };
    typedef mp::list<alternate_funcs...> af;    
  };    

  template<int tim_number, int Ahbenr_number, int Ahbenr, int Channels_count, typename Mapping> 
  struct tim_def
  {
    using index = mp::number<tim_number>;
    using mapping = Mapping;
    enum { ahbenr_number = Ahbenr_number, ahbenr = Ahbenr, channels_count = Channels_count, no = tim_number };
  };    
  
 template<int port, uint32_t t_ahbenr>
  struct port_def
  {
    using index = mp::number<port>;
    static constexpr uint32_t ahbenr = t_ahbenr;
  };

  template<int port>   decltype(GPIOA) _gpio_ctl(); // yep, GPIOA.   
  template<> constexpr decltype(GPIOA) _gpio_ctl<'A'>() { return GPIOA; }
  template<> constexpr decltype(GPIOB) _gpio_ctl<'B'>() { return GPIOB; }
  template<> constexpr decltype(GPIOC) _gpio_ctl<'C'>() { return GPIOC; }
  template<> constexpr decltype(GPIOD) _gpio_ctl<'D'>() { return GPIOD; }
  template<> constexpr decltype(GPIOE) _gpio_ctl<'E'>() { return GPIOE; }
  template<> constexpr decltype(GPIOF) _gpio_ctl<'F'>() { return GPIOF; }

  template<int port>   uint32_t _gpio_ahbenr(); 
  template<> constexpr uint32_t _gpio_ahbenr<'A'>() { return RCC_AHBENR_GPIOAEN; }
  template<> constexpr uint32_t _gpio_ahbenr<'B'>() { return RCC_AHBENR_GPIOBEN; }
  template<> constexpr uint32_t _gpio_ahbenr<'C'>() { return RCC_AHBENR_GPIOCEN; }
  template<> constexpr uint32_t _gpio_ahbenr<'D'>() { return RCC_AHBENR_GPIODEN; }
  template<> constexpr uint32_t _gpio_ahbenr<'E'>() { return RCC_AHBENR_GPIOEEN; }
  template<> constexpr uint32_t _gpio_ahbenr<'F'>() { return RCC_AHBENR_GPIOFEN; }  
  
  template<int tim_number> decltype(TIM1) _tim_ctl(); 
  template<> constexpr decltype(TIM1)  _tim_ctl<1>()  { return TIM1; }
  template<> constexpr decltype(TIM2)  _tim_ctl<2>()  { return TIM2; }
  template<> constexpr decltype(TIM3)  _tim_ctl<3>()  { return TIM3; }
  template<> constexpr decltype(TIM6)  _tim_ctl<6>()  { return TIM6; }
  template<> constexpr decltype(TIM7)  _tim_ctl<7>()  { return TIM7; }
  template<> constexpr decltype(TIM14) _tim_ctl<14>() { return TIM14; }
  template<> constexpr decltype(TIM15) _tim_ctl<15>() { return TIM15; }
  template<> constexpr decltype(TIM16) _tim_ctl<16>() { return TIM16; }
  template<> constexpr decltype(TIM17) _tim_ctl<17>() { return TIM17; }

  template<int channel_number> struct af_adc_in{};
  template<int af_no, int tim_number> struct af_tim{};
  template<int af_no, int tim_number> struct af_tim_bkin{};
  template<int af_no, int spi_number> struct af_spi_nss{};
  template<int af_no, int spi_number> struct af_spi_sck{};
  template<int af_no, int spi_number> struct af_spi_miso{};
  template<int af_no, int spi_number> struct af_spi_mosi{};
  template<int af_no, int i2c_number> struct af_i2c_scl{};
  template<int af_no, int i2c_number> struct af_i2c_sda{};
  template<int af_no> struct af_swdio {};
  template<int af_no> struct af_swclk {};
  template<int af_no> struct af_ir_out {};
  template<int af_no> struct af_eventout {};
  template<int af_no, int usart_number> struct af_usart_cts {};
  template<int af_no, int usart_number> struct af_usart_rts {};
  template<int af_no, int usart_number> struct af_usart_tx {};
  template<int af_no, int usart_number> struct af_usart_rx {};
  template<int af_no, int usart_number> struct af_usart_ck {};
  
  template<typename List, template<int...> class AF, int... opts>
  struct for_af
  {
    template<int af_no>
    constexpr static int af_no_of(AF<af_no,opts...>*) { return af_no; }
    constexpr static int af_no_of(...) { return  -1; }
    constexpr static int find() 
    {
      return af_no_of((typename List::head*)0) >= 0 
        ? af_no_of((typename List::head*)0) 
        : for_af<typename List::tail,AF,opts...>::find();
    }
  };

  template<template<int...> class AF, int... opts>
  struct for_af<mp::empty_list,AF,opts...>
  {
    constexpr static int find() { return -1; }
  };

  template<typename mCu>
  struct mcu
  {
    template<int leg_number> struct leg 
    {
      using info = typename mp::find_number<leg_number,typename mCu::legs>::value;
      static_assert(mp::find_number<leg_number,typename mCu::legs>::exists,"specified leg does not exist on the device");
    };
    template<int timer_number> struct timer 
    {
      using info = typename mp::find_number<timer_number,typename mCu::timers>::value;
      static_assert(mp::find_number<timer_number,typename mCu::timers>::exists,"specified timer does not exist on the device");
    };
    using hardware = mCu;
  };

}
