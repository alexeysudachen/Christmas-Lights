
#pragma once

#include <stdint.h>

namespace feature
{
  struct frequency
  {
    struct _Hz  {};
    struct _kHz {};
    struct _mHz {};
    uint32_t value;
    explicit frequency(uint32_t val) : value(val) {}
  };

  inline frequency operator*(uint32_t value,frequency::_Hz)
  {
    return frequency(value);
  }

  inline frequency operator*(uint32_t value,frequency::_kHz)
  {
    return frequency(value*1000);
  }

  inline frequency operator*(uint32_t value,frequency::_mHz)
  {
    return frequency(value*1000*1000);
  }

  struct duty
  {
    struct _Dty {};
    uint8_t value;
    explicit duty(uint8_t val): value(val) {}
  };

  inline duty operator*(uint8_t value,duty::_Dty)
  {
    return duty(value);
  }
	
  struct timeout
  {
    struct _Mks {};
    struct _Ms  {};
    struct _Sec {};
    uint32_t value; // in micro seconds ~ 4000sec max
    explicit timeout(uint32_t val) : value(val) {}
  };

  inline timeout operator*(uint32_t value,timeout::_Mks)
  {
    return timeout(value);
  }

  inline timeout operator*(uint32_t value,timeout::_Ms)
  {
    return timeout(value*1000);
  }

  inline timeout operator*(uint32_t value,timeout::_Sec)
  {
    return timeout(value*1000*1000);
  }
	
}

static constexpr feature::duty::_Dty       _dty_{};
static constexpr feature::frequency::_Hz   _hz_{};
static constexpr feature::frequency::_kHz  _khz_{};
static constexpr feature::frequency::_mHz  _mhz_{};
static constexpr feature::timeout::_Mks    _mks_{};
static constexpr feature::timeout::_Ms     _ms_{};
static constexpr feature::timeout::_Sec    _sec_{};
	
