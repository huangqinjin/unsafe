//
// Copyright (c) 2023 Huang Qinjin (huangqinjin@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UNSAFE_BIND_HPP
#define UNSAFE_BIND_HPP

#include <utility>
#include <type_traits>

namespace unsafe
{
    // https://en.cppreference.com/w/cpp/types/is_function
    template<typename F> struct mem_fn_t;
    template<typename R, class C, typename... A>
    struct mem_fn_t<R(C::*)(A...)>
    {
        using ret = R;
        using cls = C;
        using obj = C&;
        using type = R(A...);
        static const bool var = false;
        static const bool noe = false;
        static const std::size_t argc = sizeof...(A);
        template<typename T> using cvr = T;
        template<typename T> struct id { using type = T; };
        template<template<typename...> class S, template<typename> class T = id>
        using args = S<typename T<A>::type...>;
        template<template<typename...> class S, template<typename> class T = id>
        using argt = typename args<S, T>::type;
        template<template<typename...> class S, template<typename> class T = id>
        static const auto argv = args<S, T>::value;
    };

    template<typename R, class C> struct mem_fn_t<R C::*> : mem_fn_t<R(C::*)()> { using type = R; };

    #if _MSC_VER < 1930 || defined(__clang__)
    // Before Visual Studio 2022, the comma is required.
    // Clang enables -Wambiguous-ellipsis by default.
    #define VARIADIC_TEMPLATE_C_VARIADIC_ELLIPSIS ,...
    #else
    #define VARIADIC_TEMPLATE_C_VARIADIC_ELLIPSIS ...
    #endif

    #define MEM_FN_T_C_Q(Q, E) \
    template<typename R, class C, typename... A> \
    struct mem_fn_t<R(C::*)(A... VARIADIC_TEMPLATE_C_VARIADIC_ELLIPSIS) Q E> : \
    mem_fn_t<R(C::*)(A...) Q E> { static const bool var = true; \
    using type = R(A... VARIADIC_TEMPLATE_C_VARIADIC_ELLIPSIS) Q E; }; \

    #define MEM_FN_T_Q(Q, E) MEM_FN_T_C_Q(Q, E) \
    template<typename R, class C, typename... A> \
    struct mem_fn_t<R(C::*)(A...) Q E> : mem_fn_t<R(C::*)(A...) E> { template<typename T> using cvr = T Q; \
    using obj = std::conditional_t<std::is_reference_v<C Q>, C Q, std::add_lvalue_reference_t<C Q>>; \
    using type = R(A...) Q E; }; \

    #define MEM_FN_T(E) MEM_FN_T_Q(&, E) MEM_FN_T_Q(&&, E) \
    MEM_FN_T_Q(const, E) MEM_FN_T_Q(const&, E) MEM_FN_T_Q(const&&, E) \
    MEM_FN_T_Q(volatile, E) MEM_FN_T_Q(volatile&, E) MEM_FN_T_Q(volatile&&, E) \
    MEM_FN_T_Q(const volatile, E) MEM_FN_T_Q(const volatile&, E) MEM_FN_T_Q(const volatile&&, E) \

    #ifdef __cpp_noexcept_function_type
    template<typename R, class C, typename... A>
    struct mem_fn_t<R(C::*)(A...) noexcept> : mem_fn_t<R(C::*)(A...)> { static const bool noe = true;
    using type = R(A...) noexcept; };
    MEM_FN_T(noexcept)
    #endif
    #define MEM_FN_T_E
    MEM_FN_T(MEM_FN_T_E)
    MEM_FN_T_C_Q(MEM_FN_T_E, MEM_FN_T_E)


    template<int i> struct tag {};
   
    template<int i, class Self>
    decltype(auto) get(Self&& self) noexcept
    {
        return get(std::forward<Self>(self), tag<i>{});
    }

    // https://en.cppreference.com/w/cpp/language/class_template
    // Explicit instantiation definitions ignore member access specifiers.
    template<int i, typename PM, PM pm>
    struct bind
    {
        template<typename Self>
        static decltype(auto) get(Self&& self)
        {
            if constexpr (std::is_member_function_pointer_v<PM>)
                return [&self](auto&&... args) -> decltype(auto) {
                    return (std::forward<Self>(self).*pm)(std::forward<decltype(args)>(args)...);
                };
            else
                return std::forward<Self>(self).*pm;
        }

        [[maybe_unused]] friend decltype(auto) get(typename mem_fn_t<PM>::cls& self, tag<i>) { return get(self); }
        [[maybe_unused]] friend decltype(auto) get(typename mem_fn_t<PM>::cls&& self, tag<i>) { return get(std::move(self)); }
        [[maybe_unused]] friend decltype(auto) get(typename mem_fn_t<PM>::cls const& self, tag<i>) { return get(self); }
    };
}

#define UNSAFE_BIND_T(C, i, m, T) namespace unsafe { \
template struct bind<i, T, &C::m>; \
decltype(auto) get(C&, tag<i>); \
decltype(auto) get(C&&, tag<i>); \
decltype(auto) get(C const&, tag<i>); \
}

#define UNSAFE_BIND(C, i, m) UNSAFE_BIND_T(C, i, m, decltype(&C::m))

#endif
