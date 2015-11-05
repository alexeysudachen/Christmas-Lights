
#ifndef STM32F030
#define STM32F030
#endif

#include "cortex_m0/stm32f030f4.hxx"
#include "cortex_m0/gpio.hxx"
#include "cortex_m0/timer.hxx"
#include "cortex_m0/feature.hxx"
#include "cortex_m0/event.hxx"

using mcu = stm32f030f4;

constexpr gpio::ctl<mcu::leg<11>> led {};
constexpr gpio::ctl<mcu::leg<12>> button {};
constexpr timer::ctl<mcu::timer<14>,mcu::leg<10>> pwm {};
constexpr timer::ctl<mcu::timer<3>> alarm {};
  
int main()
{
  
  led.setup();
  button.setup(gpio::pull_up,gpio::input);
  pwm.setup(timer::pwm1,timer::negative_polarity,3*_hz_,50*_dty_,gpio::open_drain);
  alarm.setup(30*_ms_);
  
  led.set_high();
  alarm.start();

  bool foo = true;
  
  for(;;)
  {
    auto wtf = event::wait_for(pwm.rise,pwm.fall,alarm.rise);
    
    if ( wtf == pwm.rise )
        led.set_high();
    else if ( wtf == pwm.fall )
        led.set_low();
    
    if ( wtf == alarm.rise )
    {
      if ( button.get() == button.low && !foo ) 
      {
        pwm.stop();
        foo = true;
      }
      else if ( button.get() == button.high && foo )
      {
        pwm.start();
        foo = false;
      }
    }
  }

}

