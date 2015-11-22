
#ifndef STM32F030
#define STM32F030
#endif

#include "cortex_m0/stm32f030f4.hxx"
#include "cortex_m0/gpio.hxx"
#include "cortex_m0/timer.hxx"
#include "cortex_m0/feature.hxx"
#include "cortex_m0/event.hxx"

using mcu = stm32f030f4;
using leg_9 = mcu::leg<9>;
using leg_13 = mcu::leg<13>;
using leg_14 = mcu::leg<14>;
using leg_17 = mcu::leg<17>;
using timer_1 = mcu::timer<1>;
using timer_3 = mcu::timer<3>;

constexpr timer::ctl<timer_3,leg_14> pwm_3 {};
constexpr timer::ctl<timer_1,leg_17,leg_13> pwm_1 {};
constexpr gpio::ctl<leg_9> led {};
  
int main()
{
  led.setup();
  led.set_high();
  
  pwm_3.setup(timer::pwm1,3*_sec_,50*_dty_);
  pwm_1.setup(timer::pwm1,1*_khz_,10*_dty_);
  pwm_1.update_leg<leg_13>(timer::negative_polarity,20*_dty_);
  pwm_1.update_free_channel<4>(80*_dty_);
  
  pwm_1.start();
  pwm_3.start();
  for(;;)
  {
    auto wtf = event::wait_for(pwm_1.rise,pwm_1.cc4if);
    if ( wtf == pwm_1.rise )
    {
      led.set_high();
    }
    else if ( wtf == pwm_1.cc4if )
    {
      led.set_low();
    }
  }
}
 
#if 0
  
constexpr gpio::ctl<mcu::leg<11>> led {};
constexpr gpio::ctl<mcu::leg<12>> button {};
constexpr timer::ctl<mcu::timer<14>,mcu::leg<10>> pwm {};
constexpr timer::ctl<mcu::timer<3>> alarm {};

int main()
{
  //mcu::setup<mcu::hse<8000000>,mcu::pll<48000000>>();
  
  led.setup();
  button.setup(gpio::pull_up,gpio::input);
  pwm.setup(timer::pwm1,timer::negative_polarity,3*_hz_,50*_dty_,gpio::open_drain);
  alarm.setup(30*_ms_);

  led.set_high();

  /*
  pwm.enable_interrupt([]{
    auto wtf = event::check_for(pwm.rise,pwm.fall);
    if ( wtf == pwm.rise )
        led.set_high();
    else if ( wtf == pwm.fall )
        led.set_low();
  });
  
  alarm.enable_interrupt([]{
    static bool is_pressed = true;
    if ( button.get() == button.low && !is_pressed ) 
    {
      pwm.stop();
      is_pressed = true;
    }
    else if ( button.get() == button.high && is_pressed )
    {
      pwm.start();
      led.set_high();
      is_pressed = false;
    }
  });
  */
  
  alarm.start();


  bool foo = true;  
  int dty = 10;
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
        dty = (dty + 10)%100;
        pwm.update_channel<1>(dty*_dty_);
        pwm.start();
        led.set_high();
        foo = false;
      }
    }
  }

}

#endif

