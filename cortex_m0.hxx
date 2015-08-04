
#pragma once
#include "metacpp.hxx"
#include "feature.hxx"

#ifdef STM32F030
#include <stm32f0xx.h>
#else
#error unknown MCU
#endif

#include <stdint.h>
#include "feature.hxx"
#include "metacpp.hxx"

namespace cortex_m0
{
	template<int pin_number, int port, int channel, typename... alternate_funcs> 
	struct pin_def
	{
		using index = mp::number<pin_number>;
		enum { gpio_port = port, gpio_channel = channel, no = pin_number };
		typedef mp::list<alternate_funcs...,mp::E> af;		
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

	template<int port> decltype(GPIOA) _gpio_ctl(); // yep, GPIOA. 	
	template<> constexpr decltype(GPIOA) _gpio_ctl<'A'>() { return GPIOA; }
	template<> constexpr decltype(GPIOB) _gpio_ctl<'B'>() { return GPIOB; }
	template<> constexpr decltype(GPIOC) _gpio_ctl<'C'>() { return GPIOC; }
	template<> constexpr decltype(GPIOD) _gpio_ctl<'D'>() { return GPIOD; }
	template<> constexpr decltype(GPIOE) _gpio_ctl<'E'>() { return GPIOE; }
	template<> constexpr decltype(GPIOF) _gpio_ctl<'F'>() { return GPIOF; }

	template<int port> uint32_t _gpio_ahbenr(); 
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
			
	template<typename mcu_def>
	struct mcu
	{
		template<int pin_number> struct gpio{};
		template<int tim_number, int... pin_numbers> struct timer {};

		enum pull_up_t { pull_up };
		enum pull_down_t { pull_down };
		enum open_drain_t { open_drain };
		enum input_t { input };
		enum output_t { output };
		enum low_speed_t { low_speed };
		enum medium_speed_t { medium_speed };
		enum high_speed_t { high_speed };
		enum negative_polarity_t { negative_polarity };

		static constexpr bool high = true;
		static constexpr bool low  = false;
		
		/* ----- */
		
		template<int pin_number> 
		using pin_info = typename mp::find_number<pin_number,typename mcu_def::pins>::value;
		
		template<int tim_number> 
		using tim_info = typename mp::find_number<tim_number,typename mcu_def::timers>::value;
				
		/* ----- */
		
		template<int pin_number, typename... Opts>
		static void setup_gpio()
		{
			using opts_list = mp::list<Opts...,mp::E>;
			using pin_info = pin_info<pin_number>;
			constexpr auto pin = _gpio_ctl<pin_info::gpio_port>();
			
			static_assert(mp::in<output_t,opts_list>::value + mp::in<input_t,opts_list>::value < 2,"please specify only output for read/write pin");
			constexpr bool read_only = !mp::in<output_t,opts_list>::exists && mp::in<input_t,opts_list>::exists;
			
			RCC->AHBENR |= _gpio_ahbenr<pin_info::gpio_port>();
			
			if ( mp::in<open_drain_t,opts_list>::exists ) 
				pin->OTYPER |= 1<<pin_info::gpio_channel;
			else
				pin->OTYPER &= ~(1<<pin_info::gpio_channel);

			if ( !mp::in<medium_speed_t,opts_list>::exists&&!mp::in<low_speed_t,opts_list>::exists )
				pin->OSPEEDR |= uint32_t(3)<<pin_info::gpio_channel*2;
			else if ( mp::in<medium_speed_t,opts_list>::exists )
				pin->OSPEEDR = uint32_t(1)<<pin_info::gpio_channel*2 | pin->OSPEEDR&~(uint32_t(3)<<pin_info::gpio_channel*2);
			else
				pin->OSPEEDR &= ~(uint32_t(3)<<pin_info::gpio_channel*2);
			
			if ( !mp::in<pull_up_t,opts_list>::exists&&!mp::in<pull_down_t,opts_list>::exists )
				pin->PUPDR &= ~(uint32_t(3)<<pin_info::gpio_channel*2);
			else if ( mp::in<pull_up_t,opts_list>::exists ) 
				pin->PUPDR &= uint32_t(1)<<pin_info::gpio_channel*2 | pin->PUPDR&~(uint32_t(3)<<pin_info::gpio_channel*2);
			else 
				pin->PUPDR &= uint32_t(2)<<pin_info::gpio_channel*2 | pin->PUPDR&~(uint32_t(3)<<pin_info::gpio_channel*2);
			
			if ( read_only )
				pin->MODER |= uint32_t(1)<<pin_info::gpio_channel*2;
			else
				pin->MODER &= ~(uint32_t(3)<<pin_info::gpio_channel*2);
		}
		
		template<int pin_number, typename... Opts> __forceinline
		static void setup_gpio(gpio<pin_number>, Opts... opts)
		{
			setup_gpio<pin_number,Opts...>();
		}
		
		template<int pin_number>
		static void turn_on() 
		{ 
			using pin_info = pin_info<pin_number>;
			constexpr auto pin = _gpio_ctl<pin_info::gpio_port>();
			pin->ODR |= (uint16_t(1)<<pin_info::gpio_channel); 
		}

		template<int pin_number> __forceinline
		static void turn_on(gpio<pin_number>) { turn_on<pin_number>(); }

		template<int pin_number>
		static void turn_off() 
		{ 
			using pin_info = pin_info<pin_number>;
			constexpr auto pin = _gpio_ctl<pin_info::gpio_port>();
			pin->ODR &= ~(uint16_t(1)<<pin_info::gpio_channel); 
		}

		template<int pin_number> __forceinline
		static void turn_off(gpio<pin_number>) { turn_off<pin_number>(); }

		template<int pin_number>
		static bool get()
		{
			using pin_info = pin_info<pin_number>;
			constexpr auto pin = _gpio_ctl<pin_info::gpio_port>();
			return (pin->IDR&(1<<pin_info::gpio_channel) == 0)?false:true;;
		}

		template<int pin_number> __forceinline
		static bool get(gpio<pin_number>) { return get<pin_number>(); }

		template<int pin_number> __forceinline
		static void put(gpio<pin_number>,bool value)
		{
			if ( value == high )
				turn_on<pin_number>();
			else
				turn_off<pin_number>();
		}

		template<typename Item,typename tim_number>
		struct enable_timer_pin
		{
			static void apply()
			{
				constexpr int pin_number = Item::value;
				using pin_info = pin_info<pin_number>;
				constexpr auto pin = _gpio_ctl<pin_info::gpio_port>();
			
				RCC->AHBENR |= _gpio_ahbenr<pin_info::gpio_port>();
			
				pin->OTYPER &= ~(1<<pin_info::gpio_channel);
				pin->OSPEEDR |= uint32_t(3)<<pin_info::gpio_channel*2; // high speed
				pin->MODER &= ~(uint32_t(2)<<pin_info::gpio_channel*2); // alternate function
				
				constexpr int pin_af = for_af<typename pin_info::af,af_tim,tim_number::value>::find();
				static_assert(pin_af >= 0,"specified pin des not connected to timer");
				
				constexpr int af_r = pin_info::gpio_channel < 8 ? 0 : 1;
				constexpr int af_shift = (pin_info::gpio_channel%8)*4;
				pin->AFR[af_r] = pin->AFR[af_r] & ~(uint32_t(0xf)<<af_shift) | uint32_t(pin_af)<<af_shift;
			}
		};
		
		template<int tim_number, typename pin_numbers>
		static void enable_timer()
		{			
			mp::for_each<pin_numbers,enable_timer_pin,mp::number<tim_number>>::apply();
			
			using tim_info = tim_info<tim_number>;
			static_assert(!mp::the_same<tim_info,mp::E>::is_true,"mcu does not have specified timer");
			
			constexpr auto tim = _tim_ctl<tim_number>();
			
			if ( tim_info::ahbenr_number == 1 )
				RCC->APB1ENR |= tim_info::ahbenr;
			else
				RCC->APB2ENR |= tim_info::ahbenr;

			tim->PSC = 0;
			tim->ARR = 0;
			tim->CCR1 = 0;
			tim->CCMR1 = 0;
			tim->CCER = 0;
			tim->BDTR = 0;
			tim->CR1 = 0;
			tim->EGR = 0;
		}
				
		template<int tim_number, int... pin_numbers, typename... Opts>
		static void setup_pwm(timer<tim_number,pin_numbers...>, Opts... opts)
		{
			enable_timer<tim_number,mp::number_list<pin_numbers...>>();
		}

		template<int tim_number, int... pin_numbers>
		static void update_pwm_duty(timer<tim_number,pin_numbers...>, feature::duty dty)
		{
		}

		template<int tim_number, int... pin_numbers, typename... Opts>
		static void setup_alarm(timer<tim_number,pin_numbers...>, Opts... opts)
		{
		}
		
		template<int tim_number, int... pin_numbers, typename... Opts>
		static void start(timer<tim_number,pin_numbers...>, Opts... opts)
		{
		}
		
		template<int tim_number, int... pin_numbers>
		static void stop(timer<tim_number,pin_numbers...>)
		{
		}
		
		template<int tim_numbers, int... pin_numbers>
		static bool is_signaled()
		{
			return false;
		}
		
		template<int tim_number, int... pin_numbers> __forceinline
		static bool is_signaled(timer<tim_number,pin_numbers...>) 
		{ return is_signaled<tim_number,pin_numbers...>(); }
		
		template<int tim_numbers, int... pin_numbers>
		static bool is_on_duty()
		{
			return false;
		}

		template<int tim_number, int... pin_numbers> __forceinline
		static bool is_off_duty(timer<tim_number,pin_numbers...>) 
		{ return !is_on_duty<tim_number,pin_numbers...>(); }
		
		template<int tim_number, int... pin_numbers> __forceinline
		static bool is_on_duty(timer<tim_number,pin_numbers...>) 
		{ return is_on_duty<tim_number,pin_numbers...>(); }

		struct wtf
		{
			int value;
			
			template<int tim_number, int... pin_numbers>
			bool operator ==(timer<tim_number,pin_numbers...>)
			{
				return 0;
			}
		};

		template<typename... Sources>
		static wtf wait_for_next(Sources... sources)
		{
			return wtf{0};
		}
		
	};
	
	struct stm32f030f4 : mcu<stm32f030f4>
	{
		
		template<int pin_number, int tim_channel>
		struct maps_to
		{
			using index = mp::number<pin_number>;
			static constexpr int channel = tim_channel;
		};
	
		typedef mp::list<
			tim_def<1, 2, RCC_APB2ENR_TIM1EN, 4, mp::list<maps_to<13,-1>,maps_to<14,-3>,maps_to<17,2>,maps_to<17,3>> >,
		  tim_def<3, 1, RCC_APB1ENR_TIM3EN, 4, mp::list<maps_to<12,1>,maps_to<13,2>,maps_to<14,4>> >,
			tim_def<14,1, RCC_APB1ENR_TIM14EN,1, mp::list<maps_to<10,1>,maps_to<13,1>,maps_to<14,1>> >,
			tim_def<16,2, RCC_APB2ENR_TIM16EN,1, mp::list<maps_to<12,1>> >,
			tim_def<17,2, RCC_APB2ENR_TIM17EN,1, mp::list<maps_to<13,1>> >
		> timers;
		
		
		typedef mp::list<
			pin_def<2, 'F',0 >,
			pin_def<3, 'F',1 >,
			pin_def<6, 'A',0, af_adc_in<0>, af_usart_cts<1,1> >,
			pin_def<7, 'A',1, af_adc_in<1>, af_usart_rts<1,1>, af_eventout<0> >,
			pin_def<8, 'A',2, af_adc_in<2>, af_usart_tx<1,1> >,
			pin_def<9, 'A',3, af_adc_in<3>, af_usart_rx<1,1> >,
			pin_def<10,'A',4, af_adc_in<4>, af_usart_ck<1,1>, af_tim<4,14 /*1*/>, af_spi_nss<0,1> >,
		  pin_def<11,'A',5, af_adc_in<5>, af_spi_sck<0,1> >,
		  pin_def<12,'A',6, af_adc_in<6>, af_spi_miso<0,1>, af_tim<1,3 /*1*/>, af_tim<5,16 /*1*/>,  af_eventout<6>, af_tim_bkin<2,1> >,
		  pin_def<13,'A',7, af_adc_in<7>, af_spi_mosi<0,1>, af_tim<1,3 /*2*/>, af_tim<2,1 /*-1*/>, af_tim<4,14 /*1*/>, af_tim<5,17 /*1*/>, af_eventout<6> >,
		  pin_def<14,'B',1, af_adc_in<9>, af_tim<0,14 /*1*/>, af_tim<1,3 /*4*/>, af_tim<2,1 /*-3*/> >,
		  pin_def<17,'A',9,               af_usart_tx<1,1>, af_tim<2,1 /*2*/>, af_i2c_scl<4,1> >,
		  pin_def<18,'A',10,              af_usart_rx<1,1>, af_tim<2,1 /*3*/>, af_i2c_sda<4,1>, af_tim_bkin<0,17> >,
		  pin_def<19,'A',13,					    af_swdio<0>, af_ir_out<1> >,
		  pin_def<20,'A',14,              af_swclk<0>, af_usart_tx<1,1> >
		> pins;
	};
}
