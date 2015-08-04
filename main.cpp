
#ifndef STM32F030
#define STM32F030
#endif

#include "cortex_m0.hxx"

using mcu = cortex_m0::stm32f030f4;

constexpr mcu::timer<14,10> pwm{};
constexpr mcu::timer<3> alarm{};
constexpr mcu::gpio<11> led{};
constexpr mcu::gpio<12> button{};

int main()
{
  mcu::setup_gpio(led);
  mcu::setup_gpio(button,mcu::pull_up,mcu::input);
  mcu::setup_pwm(pwm,mcu::negative_polarity,3*_hz_,50*_dty_);
  mcu::setup_alarm(alarm,30*_ms_);

  mcu::turn_on(led);
  mcu::start(alarm);

  bool foo = true;
  
  for(;;)
  {
    auto wtf = mcu::wait_for_next(pwm,alarm);
    if ( wtf == pwm )
    {
      mcu::put(led,mcu::is_on_duty(pwm));
    }
    else if ( wtf == alarm )
    {
      if ( mcu::get(button) == mcu::low && !foo ) 
      {
        mcu::stop(pwm);
        foo = true;
      }
      else if ( mcu::get(button) == mcu::high && foo )
      {
        mcu::start(pwm);
        foo = false;
      }
    }
  }

}

