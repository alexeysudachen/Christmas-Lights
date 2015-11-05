
#pragma once

#include "metacpp.hxx"
#include "cortex_m.hxx"

namespace gpio
{
  using namespace cortex_m0;
  
  inline namespace share
  {
    enum open_drain_t { open_drain };
  }

  enum pull_up_t { pull_up };
  enum pull_down_t { pull_down };
  enum input_t { input };
  enum output_t { output };
  enum low_speed_t { low_speed };
  enum medium_speed_t { medium_speed };
  enum high_speed_t { high_speed };
    
  template<typename leg> 
  struct ctl
  {
    template<typename... Opts> 
    static void _setup()
    {
      using opts_list = mp::list<Opts...>;
      using leg_info = typename leg::info;
      constexpr auto leg_ctl = _gpio_ctl<leg_info::gpio_port>();
      
      static_assert(mp::in<output_t,opts_list>::value + mp::in<input_t,opts_list>::value < 2,"please specify only output for read/write leg_ctl");
      constexpr bool read_only = !mp::in<output_t,opts_list>::exists && mp::in<input_t,opts_list>::exists;
      
      RCC->AHBENR |= _gpio_ahbenr<leg_info::gpio_port>();
      
      if ( mp::in<open_drain_t,opts_list>::exists ) 
        leg_ctl->OTYPER |= 1<<leg_info::gpio_channel;
      else
        leg_ctl->OTYPER &= ~(1<<leg_info::gpio_channel);

      if ( !mp::in<medium_speed_t,opts_list>::exists&&!mp::in<low_speed_t,opts_list>::exists )
        leg_ctl->OSPEEDR |= uint32_t(3)<<leg_info::gpio_channel*2;
      else if ( mp::in<medium_speed_t,opts_list>::exists )
        leg_ctl->OSPEEDR = uint32_t(1)<<leg_info::gpio_channel*2 | leg_ctl->OSPEEDR&~(uint32_t(3)<<leg_info::gpio_channel*2);
      else
        leg_ctl->OSPEEDR &= ~(uint32_t(3)<<leg_info::gpio_channel*2);
      
      if ( !mp::in<pull_up_t,opts_list>::exists&&!mp::in<pull_down_t,opts_list>::exists )
        leg_ctl->PUPDR &= ~(uint32_t(3)<<leg_info::gpio_channel*2);
      else if ( mp::in<pull_up_t,opts_list>::exists ) 
        leg_ctl->PUPDR &= uint32_t(1)<<leg_info::gpio_channel*2 | leg_ctl->PUPDR&~(uint32_t(3)<<leg_info::gpio_channel*2);
      else 
        leg_ctl->PUPDR &= uint32_t(2)<<leg_info::gpio_channel*2 | leg_ctl->PUPDR&~(uint32_t(3)<<leg_info::gpio_channel*2);
      
      if ( read_only )
        leg_ctl->MODER |= uint32_t(1)<<leg_info::gpio_channel*2;
      else
        leg_ctl->MODER &= ~(uint32_t(3)<<leg_info::gpio_channel*2);
    }

    template<typename... Opts> 
    __forceinline void setup(Opts... opts) const { _setup<Opts...>(); }
    
    void set_high() const
    {
      using info = typename leg::info;
      constexpr auto ctl = _gpio_ctl<info::gpio_port>();
      ctl->ODR |= (uint16_t(1)<<info::gpio_channel); 
    }
    
    void set_low() const
    {
      using info = typename leg::info;
      constexpr auto ctl = _gpio_ctl<info::gpio_port>();
      ctl->ODR &= ~(uint16_t(1)<<info::gpio_channel); 
    }

    bool get() const
    {
      using info = typename leg::info;
      constexpr auto ctl = _gpio_ctl<info::gpio_port>();
      return (ctl->IDR&(1<<info::gpio_channel) == 0)?false:true;;
    }
    
    static constexpr bool high = true;
    static constexpr bool low  = false;
  };
}
