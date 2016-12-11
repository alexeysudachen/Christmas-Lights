
#ifndef STM32F030
#define STM32F030
#endif

#include "cortex_m0/stm32f030f4.hxx"
#include "cortex_m0/gpio.hxx"
#include "cortex_m0/timer.hxx"
#include "cortex_m0/feature.hxx"
#include "cortex_m0/event.hxx"

using mcu = stm32f030f4;
using leg_2 = mcu::leg<2>;
using leg_6 = mcu::leg<6>;
using leg_8 = mcu::leg<8>;
using leg_11 = mcu::leg<11>;
using timer_1 = mcu::timer<1>;

constexpr timer::ctl<timer_1> pwm {};
constexpr gpio::ctl<leg_6>    led_1 {};
constexpr gpio::ctl<leg_8>    led_2 {};
constexpr gpio::ctl<leg_2>    trigger {};
constexpr gpio::ctl<leg_11>   environ {};
  
static uint32_t max_dty_width = 0;
static uint32_t dty_width = 0;
static uint32_t dty_gap = 0;
  
inline int clamp_percent(int percent)
{
  return percent > 100 ? 100 : percent < 0 ? 0 : percent;
}
  
void calculate_dty_width_and_start_pwm()
{
  
  pwm.setup(timer::pwm1,100*_hz_);
  uint32_t acc = pwm.get_max_counter();
  max_dty_width = acc-acc/10;
  
  dty_gap = acc/2-acc/10;
  //pwm.update_free_channel<3>((dty_gap-3*acc/10)*_tik_);
  pwm.update_free_channel<3>((1+acc/20)*_tik_);
  max_dty_width-= dty_gap;
  dty_width = max_dty_width;
}
  
void update_btns(int percent)
{
  dty_width = max_dty_width*clamp_percent(percent)/100;
}
  
template<int channel>
void update_channel_btns(int percent)
{
  uint32_t width = dty_width*clamp_percent(percent)/100;
  pwm.update_free_channel<channel>((dty_gap+max_dty_width-width)*_tik_);
}

template<int channel>
int next_random_100()
{
  static unsigned store = channel;
  store = (store * 214013 + 2531011)&0x0ffff;
  return (100*store) / (32768+1);
}

template<int channel>
void update_led()
{
  static int counter = 0;
  static int counter_2 = 0;

  if ( counter + counter_2 == 0 )
  {
    counter_2 = next_random_100<channel>();
    if ( counter_2 >= 100 ) counter_2 = 99;
    if ( counter_2 < 10 ) counter_2 = 10;
    counter = counter_2;
  }
  
  if ( counter ) 
  {
    int foo = 100-counter;
    int percent = foo*foo*100/10000;
    update_channel_btns<channel>(percent);
    --counter;
  }
  else
  {
    int foo = counter_2;
    int percent = foo*foo*100/10000;
    update_channel_btns<channel>(percent);
    --counter_2;
  }
}

int main()
{
  bool the_night_time = false;
  bool triggered = true;
  
  led_1.setup(gpio::open_drain);
  led_2.setup(gpio::open_drain);
  trigger.setup(gpio::input);
  environ.setup(gpio::input,gpio::pull_down);
    
  calculate_dty_width_and_start_pwm();
  update_channel_btns<1>(10);
  update_channel_btns<2>(99);
  
  led_1.set_high();
  led_2.set_high();

  for(;;)
  {
    if ( trigger.get() != triggered )
    {
      triggered = !triggered;
      
      if ( !triggered )
      {        
        pwm.start();
      
        if ( environ.get() != the_night_time )
        {
          the_night_time = !the_night_time;
          update_btns(the_night_time?20:100);
        }      
      }
    }
    else if ( event::check_and_clear(pwm.cc3if) )
    {
      led_1.set_low();
      led_2.set_low();
      update_led<1>();
      update_led<2>();
    }
    else
    {
      if ( event::check_and_clear(pwm.cc1if) )
        led_1.set_high();
      if ( event::check_and_clear(pwm.cc2if) )
        led_2.set_high();
    }
  }
}
