
#pragma once

#include "metacpp.hxx"
#include "cortex_m.hxx"
#include "feature.hxx"
#include "gpio.hxx"

namespace timer
{
  using namespace cortex_m0;
  using namespace gpio::share;
  
  enum negative_polarity_t { negative_polarity };
  enum positive_polarity_t { positive_polarity };
  enum pwm1_t { pwm, pwm1 };
  enum pwm2_t { pwm2 };
  enum counter_t { counter };
  enum pulse_t { pulse };
   
  namespace {
    template<int no> // share body in header
    __declspec(noinline) uint16_t split_ticks(uint32_t& ticks)
    {
      uint32_t ps = 1;
      do
        for ( int i = 0; i < feature::prime::PRIMES_8bit_COUNT && ticks > 0x0ffff; )
        {
          uint8_t p = feature::prime::get_8bit(i);
          auto r = __aeabi_uidivmod(ticks,p);
          if ( r.rem != 0 ) ++i;
          else
          {
            ps = ps*p;
            ticks = r.quot;
          }
        }
      while ( ticks > 0x0ffff && --ticks );
      return (uint16_t)ps;
    }
  }
  
  template<typename Timer, typename... Legs> 
  struct ctl
  {
    using tim_info = typename Timer::info;
    using legs_list = mp::list<Legs...>;
    
    template<typename Leg,typename opts_list>
    struct LegSetupOperator
    {
      __forceinline static void apply()
      {
        using leg_info = typename Leg::info;
        constexpr auto leg_ctl = _gpio_ctl<leg_info::gpio_port>();

        RCC->AHBENR |= _gpio_ahbenr<leg_info::gpio_port>();
        leg_ctl->MODER = leg_ctl->MODER & ~(uint32_t(3)<<leg_info::gpio_channel*2)
                            | (uint32_t(2)<<leg_info::gpio_channel*2); // alternate function

        if ( mp::in<open_drain_t,opts_list>::exists ) 
          leg_ctl->OTYPER |= 1<<leg_info::gpio_channel;
        else
          leg_ctl->OTYPER &= ~(1<<leg_info::gpio_channel);
        
        leg_ctl->OSPEEDR |= uint32_t(3)<<leg_info::gpio_channel*2; // high speed
        
        constexpr int leg_af = for_af<typename leg_info::af,af_tim,tim_info::no>::find();
        static_assert(leg_af >= 0,"specified leg des not connected to timer");
        
        constexpr int af_r = leg_info::gpio_channel < 8 ? 0 : 1;
        constexpr int af_shift = (leg_info::gpio_channel%8)*4;
        leg_ctl->AFR[af_r] = leg_ctl->AFR[af_r] & ~(uint32_t(0xf)<<af_shift) | (uint32_t(leg_af)<<af_shift);        
      }
    };
     
    struct enable_channel{};
    struct disable_channel{};
    
    template<int channel>
    struct ChannelOperator1
    {
      static constexpr int abs_channel = channel < 0 ? -channel : channel;
      static constexpr int shift = abs_channel&1 ? 0 : 8;
      
      __forceinline static volatile uint16_t& ccmr() 
      { 
        constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
        return abs_channel > 2 ? tim_ctl->CCMR2 : tim_ctl->CCMR1; 
      };
      
      __forceinline static void set_mode(int val, bool enable_preload)
      {
        uint16_t mask = (7 << (4+shift)) | (1 << (3+shift));
        val = ((val&7) << (4+shift)) | ((enable_preload?1:0) << (3+shift));
        volatile uint16_t& CCMR = ccmr();
        CCMR = (CCMR & ~mask)|val;
      }
      
      static constexpr uint16_t polarity_bit() { return channel > 0 ? 1<<((channel-1)*4+1) : 1<<((abs_channel-1)*4+3); }
      static constexpr uint16_t enable_bit() { return channel > 0 ? 1<<((channel-1)*4+0) : 1<<((abs_channel-1)*4+2); }
      
      __forceinline static void apply(pwm1_t) { set_mode(6,true); }
      __forceinline static void apply(pwm2_t) { set_mode(7,true); }
      __forceinline static void apply(pulse_t){ set_mode(3,true); }
      __forceinline static void apply(negative_polarity_t) { _tim_ctl<tim_info::no>()->CCER |= polarity_bit(); }
      __forceinline static void apply(positive_polarity_t) { _tim_ctl<tim_info::no>()->CCER &= ~polarity_bit(); }
      __forceinline static void apply(enable_channel) { _tim_ctl<tim_info::no>()->CCER |= enable_bit(); }
      __forceinline static void apply(disable_channel) { _tim_ctl<tim_info::no>()->CCER &= ~enable_bit(); }
           
      __forceinline static void apply(feature::duty dty)
      { 
        constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
        uint16_t counter = tim_ctl->ARR;
        uint16_t compare = uint16_t(uint32_t(counter)*dty.value/100);
        static_assert(abs_channel<5 && abs_channel>0, "impossible channel index");
        switch(abs_channel)
        {
          case 1: tim_ctl->CCR1 = compare; break;
          case 2: tim_ctl->CCR2 = compare; break;
          case 3: tim_ctl->CCR3 = compare; break;
          case 4: tim_ctl->CCR4 = compare; break;
        }
      }
    };
    
    template<typename Leg>
    using ChannelOperator = ChannelOperator1<mp::find<typename Leg::info::index,typename tim_info::mapping>::value::channel>;

    struct OptSetupOperator
    {
      template<typename T> __forceinline static 
        typename mp::enable_if<
          mp::the_same<T,pwm1_t>::is_true
        ||mp::the_same<T,pwm2_t>::is_true
        ||mp::the_same<T,pulse_t>::is_true
        ||mp::the_same<T,negative_polarity_t>::is_true
        ,void>::type
      apply(T q)
      { 
        mp::for_each<legs_list,ChannelOperator>::apply(q); 
      }
      
     template<typename T> __forceinline static 
        typename mp::enable_if<
          mp::the_same<T,open_drain_t>::is_true
        ,void>::type
      apply(T q){}
        
      __forceinline static void apply(feature::frequency fq) // Hz
      {
        uint32_t ticks = SystemCoreClock*2/fq.value;
        ticks = (ticks>>1) + (ticks&1); // rouding to near.
        set_counter_ticks(ticks);
      }
      
      __forceinline static void apply(feature::timeout mks) // micro seconds
      {
        uint32_t ticks = uint32_t(uint64_t(mks.value)*SystemCoreClock*2/1000000);
        ticks = (ticks>>1) + (ticks&1); // rouding to near.
        set_counter_ticks(ticks);
      }

      // nothing, it requires correct counter set up before.
      __forceinline static void apply(feature::duty dty) {};
    };
    
    static void set_counter_ticks(uint32_t ticks)
    {
      constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
      tim_ctl->PSC = split_ticks<0>(ticks)-1;
      tim_ctl->ARR = (uint16_t)ticks;
    }
        
    template<typename... Opts>
    __declspec(noinline) void setup(Opts... opts) const
    {
      using opts_list = mp::list<Opts...>;
      
      // binding specified legs to timer
      mp::for_each<legs_list,LegSetupOperator,opts_list>::apply();
      
      // enable timer      
      if ( tim_info::ahbenr_number == 1 )
        RCC->APB1ENR |= tim_info::ahbenr;
      else
        RCC->APB2ENR |= tim_info::ahbenr;
      
      constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
      
      // ARR is not buffered, counter is edge-aligned, counter will not stop, 
      //   upcounting, any update source, UEV event is enabled, counter is disabled.
      tim_ctl->CR1 = 0;
      tim_ctl->CR2 = 0;
      tim_ctl->DIER = 0;
      tim_ctl->SR = 0;
      tim_ctl->CCER = 0;
      tim_ctl->CCMR1 = 0;
      if ( tim_info::channels_count > 2 )
        tim_ctl->CCMR2 = 0;
      
      // setting common timer parameters
      mp::for_each_arg<OptSetupOperator>::apply(opts...);
            
      // setting up pwm duty based on already configured frequency
      if ( mp::in<feature::duty,opts_list>::exists )
      {
        if ( !mp::in<pwm1_t,opts_list>::exists 
          && !mp::in<pwm2_t,opts_list>::exists )
        {
          OptSetupOperator::apply(pwm1);
        }
        feature::duty dty = mp::arg_fetch<feature::duty>::value(opts...);
        mp::for_each<legs_list,ChannelOperator>::apply(dty);
      }
      
      if ( tim_info::no == 1 )
      {
        tim_ctl->BDTR = uint16_t(1) << 14; // AOE
      }
      mp::for_each<legs_list,ChannelOperator>::apply(enable_channel());
    }
    
    __declspec(noinline) void start() const
    {
      constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
      tim_ctl->DIER&= ~TIM_DIER_UIE; // disable update interrupt
      tim_ctl->EGR |= TIM_EGR_UG;    // update counter
      tim_ctl->SR   =0;              // clear status flags
      tim_ctl->DIER|= TIM_DIER_UIE;  // enable update interrupt
      tim_ctl->CR1 |= TIM_CR1_CEN;   // start counter
    }
    
    // be aware, PWM keeps duty state. If PWM was on duty when stopped, it will be on duty after stop.
    void stop() const
    {
      constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
      tim_ctl->CR1 &= ~TIM_CR1_CEN;  // stop counter
    }
    
    // completely turn off the timer. Requires setup before next usage
    void shutdown() const
    {
      stop();
      if ( tim_info::ahbenr_number == 1 )
        RCC->APB1ENR &= ~tim_info::ahbenr;
      else
        RCC->APB2ENR &= ~tim_info::ahbenr;      
    }
    
    // resume after stop.
    void resume() const
    {
      constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
      tim_ctl->CR1 |= TIM_CR1_CEN;  // resume counter
    }

    // will updated yet only on next cycle.
    void update_duty(feature::duty dty) const
    {
      mp::for_each<legs_list,ChannelOperator>::apply(dty);
    }
   
    template<typename Channel, typename Leg>
    struct is_binded_to_channel
    {
      static constexpr int channel = mp::find<typename Leg::info::index,typename tim_info::mapping>::value::channel;
      static constexpr bool is_true = (int)Channel::value == channel || (int)Channel::value == -channel;
    };
      
    template<typename Channel>
    struct is_binded_to_channel<Channel,mp::E>
    {
      static constexpr bool is_true = false;
    };
            
    template<int channel_no> using in_binded_legs = mp::list_find_p<mp::number<channel_no>,is_binded_to_channel,legs_list>;     

    template<int channel_no, typename... Opts> __declspec(noinline)
    typename mp::enable_if<in_binded_legs<channel_no>::exists == true,void>::type 
    /*void*/ update_channel(Opts... opts) const
    {
      using Leg = typename in_binded_legs<channel_no>::value;
      mp::for_each_arg< ChannelOperator<Leg> >::apply(opts...); 
    }
    
    template<int channel_no, typename... Opts> __declspec(noinline)
      typename mp::enable_if<
        in_binded_legs<channel_no>::exists == false 
        && channel_no >= 1 
        && channel_no <= tim_info::channels_count,
        void>::type 
    /*void*/ update_free_channel(Opts... opts) const
    {
      using Leg = typename in_binded_legs<channel_no>::value;
      mp::for_each_arg< ChannelOperator1<channel_no> >::apply(opts...); 
    }

    template<typename Leg, typename... Opts> __declspec(noinline)
    typename mp::enable_if<mp::in<Leg,legs_list>::exists,void>::type 
    /*void*/ update_leg(Opts... opts) const
    {
      mp::for_each_arg< ChannelOperator<Leg> >::apply(opts...); 
    }

    enum { 
      REASON_COMPARE  = 0, // PWM is low, CNT matches the CCRx (any)
      REASON_COMPARE1 = 1, // PWM is low, CNT matches the CCR1 
      REASON_COMPARE2 = 2, // PWM is low, CNT matches the CCR2 
      REASON_COMPARE3 = 3, // PWM is low, CNT matches the CCR3 
      REASON_COMPARE4 = 4, // PWM is low, CNT matches the CCR4 
      REASON_OVERFLOW  = 5, // PWM is high,CNT updated
    };
     
    template<int by_reason>
    struct event_source
    {
      using timinfo = typename Timer::info;
      enum { reason = by_reason };
      __forceinline static void clear()
      {
        constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
        switch(by_reason)
        {
        case REASON_OVERFLOW:
          tim_ctl->SR &= ~TIM_SR_UIF; break;
        case REASON_COMPARE:
          tim_ctl->SR &= ~(TIM_SR_CC1IF|TIM_SR_CC2IF|TIM_SR_CC3IF|TIM_SR_CC4IF); break;
        case REASON_COMPARE1:
          tim_ctl->SR &= ~TIM_SR_CC1IF; break;
        case REASON_COMPARE2:
          tim_ctl->SR &= ~TIM_SR_CC2IF; break;
        case REASON_COMPARE3:
          tim_ctl->SR &= ~TIM_SR_CC3IF; break;
        case REASON_COMPARE4:
          tim_ctl->SR &= ~TIM_SR_CC4IF; break;
        }
      }
      __forceinline static bool occured()
      {
        constexpr auto tim_ctl = _tim_ctl<tim_info::no>();
        switch(by_reason)
        {
        case REASON_OVERFLOW:
          return (tim_ctl->SR&TIM_SR_UIF) != 0;
        case REASON_COMPARE:
          return (tim_ctl->SR&(TIM_SR_CC1IF|TIM_SR_CC2IF|TIM_SR_CC3IF|TIM_SR_CC4IF)) != 0;
        case REASON_COMPARE1:
          return (tim_ctl->SR&TIM_SR_CC1IF) != 0;
        case REASON_COMPARE2:
          return (tim_ctl->SR&TIM_SR_CC2IF) != 0;
        case REASON_COMPARE3:
          return (tim_ctl->SR&TIM_SR_CC3IF) != 0;
        case REASON_COMPARE4:
          return (tim_ctl->SR&TIM_SR_CC4IF) != 0;
        }
        return false;
      }
      __forceinline static bool check_and_clear()
      {
        if ( occured() )
        {
          clear();
          return true;
        }
        return false;
      }
    };
        
    // event source descriptors
    static void fall(event_source<REASON_COMPARE>){}
    static void cc1if(event_source<REASON_COMPARE1>){}
    static void cc2if(event_source<REASON_COMPARE2>){}
    static void cc3if(event_source<REASON_COMPARE3>){}
    static void cc4if(event_source<REASON_COMPARE4>){}
    
    // event will be mached on end of cycle. So it skips first PWM on duty event.
    // but it's ok, because PWM will be on duty immediately on timer start.
    static void rise(event_source<REASON_OVERFLOW>){}  
  };
}
