
#pragma once

#include "metacpp.hxx"

namespace event {
  
  template<typename...> struct wtf;
  template<typename... Sources> const wtf<Sources...> wait_for_next(Sources... sources);
  template<typename... Sources> const wtf<Sources...> wait_for(Sources... sources);

  template<> struct wtf<> 
  { 
    bool operator == (wtf<>) const;
    bool occured() const { return value != 0; }
  protected:
    unsigned value; 
    wtf() : value(0) {}
    bool _update(unsigned val) { value = val; return occured(); }
    void clear() { value = 0; }
  };

  template<typename First, typename... Next>
  struct wtf<First,Next...> : wtf<Next...>
  {
    template<typename T> static T source_of(void(T));
    using Source = decltype(source_of(First()));
    using wtf<Next...>::operator ==;
    using wtf<>::occured;
    __forceinline bool operator ==(First) const
    {
      return (wtf<>::value & (1<<mp::length_of<Next...>::value)) != 0;
    }    
    friend const wtf<First,Next...> wait_for<First,Next...>(First,Next...);
    friend const wtf<First,Next...> wait_for_next<First,Next...>(First,Next...);
  protected:
    wtf() : wtf<Next...>() {}
    __forceinline bool _update(unsigned val)
    {
      return wtf<Next...>::_update((Source::check_and_clear()?1<<mp::length_of<Next...>::value:0)|val);
    }
    __declspec(noinline) bool update(...) /*elipses means __attribute__((noinline))*/ { return _update(0); }
    void clear()
    {
      Source::clear();
      wtf<Next...>::clear();
    }
  };
      
  template<typename... Sources>
  __forceinline const wtf<Sources...> wait_for(Sources... sources)
  {
    wtf<Sources...> e{};
    while ( !e.update() ) __nop(); // wait for event
    return e;
  }

  template<typename... Sources>
  __forceinline const wtf<Sources...> wait_for_next(Sources... sources)
  {
    wtf<Sources...> e{};
    e.clear(); // clear sources
    while ( !e.update() ) __nop(); // wait for event
    return e;
  }
  
}
