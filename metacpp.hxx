
#pragma once

#include <stdint.h>

namespace mp
{
  template<bool B, class T = void>
  struct enable_if { typedef T type; };
   
  template<class T>
  struct enable_if<false, T> {};

  struct E {};
    
  template<uint64_t Value>
  struct number
  {
    static const uint64_t value = Value;
  };
    
  template<typename Head, typename... Tail> struct list
  {
    using head = Head;
    using tail = list<Tail...>;
    static const int length = tail::length + 1;
  };  
  template<> struct list<E> 
  {
    using head = E;
    using tail = list<E>;
    static const int length = 0;
  };  
  template<typename Head> struct list<Head> 
  {
    using head = Head;
    using tail = list<E>;
    static const int length = tail::length + 1;
  };
  using empty_list = list<E>;
  
  template<int... numbers> struct number_list;  
  template<int first, int... numbers> struct number_list<first, numbers...>
  {
    using head = number<first>;
    using tail = number_list<numbers...>;
  };
  template<int first> struct number_list<first>
  {
    using head = number<first>;
    using tail = empty_list;
  };
  template<> struct number_list<> : empty_list {};
  
  template<typename A, typename B>
  struct the_same
  {
    static constexpr bool is_true = false;
  };
    
  template<typename A>
  struct the_same<A,A>
  {
    static constexpr bool is_true = true;
  };

  template<typename A, typename B>
  struct the_same_index
  {
    static constexpr bool is_true = the_same<A,typename B::index>::is_true;
  };
    
  template<typename A>
  struct the_same_index<A,E>
  {
    static constexpr bool is_true = false;
  };

  template<bool, template<typename...> class Pred, typename What, typename Head, typename Tail>
  struct recursive_compare
  {
    using value = typename recursive_compare<Pred<What,typename Tail::head>::is_true,Pred,What,typename Tail::head,typename Tail::tail>::value;
    static constexpr bool exists = recursive_compare<Pred<What,typename Tail::head>::is_true,Pred,What,typename Tail::head,typename Tail::tail>::exists;
  };
  
  template<template<typename...> class Pred, typename What, typename Head, typename Tail>
  struct recursive_compare<true,Pred,What,Head,Tail>
  {
    using value = Head;
    static constexpr bool exists = true;
  };
    
  template<template<typename...> class Pred, typename What>
  struct recursive_compare<false,Pred,What,E,empty_list>
  {
    using value = E;
    static constexpr bool exists = false;
  };

  template<typename What, template<typename...> class Pred, typename List>
  struct list_find_p
  {
    using value = typename recursive_compare<Pred<What,typename List::head>::is_true,Pred,What,typename List::head,typename List::tail>::value;
    static constexpr bool exists = recursive_compare<Pred<What,typename List::head>::is_true,Pred,What,typename List::head,typename List::tail>::exists;
  };
  
  template<typename What, typename List>
  struct in
  {
    static constexpr bool exists = recursive_compare<the_same<What,typename List::head>::is_true,the_same,What,typename List::head,typename List::tail>::exists;
    static constexpr int value = exists?1:0;
  };
  
  template<typename Index, typename List>
  struct find
  {
    using value = typename recursive_compare<the_same_index<Index,typename List::head>::is_true,the_same_index,Index,typename List::head,typename List::tail>::value;
    static constexpr bool exists = recursive_compare<the_same_index<Index,typename List::head>::is_true,the_same_index,Index,typename List::head,typename List::tail>::exists;
  };
  
  template<uint64_t index, typename List>
  struct find_number
  {
    using value = typename find<number<index>,List>::value;
    static constexpr bool exists = find<number<index>,List>::exists;
  };
  
  template<typename List,template<typename...> class Operator,typename Arg>
  struct for_each
  {
    template<typename... Opts>
    static void apply(Opts... opts) 
    { 
      Operator<typename List::head,Arg>::apply(opts...);
      for_each<typename List::tail,Operator,Arg>::apply(opts...);
    }
  };
  
  template<template<typename...> class Operator,typename Arg>
  struct for_each<empty_list,Operator,Arg>
  {
    template<typename... Opts>
    static void apply(Opts... opts) {}
  };
}
