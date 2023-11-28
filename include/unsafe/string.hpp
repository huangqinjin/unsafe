//
// Copyright (c) 2023 Huang Qinjin (huangqinjin@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UNSAFE_STRING_HPP
#define UNSAFE_STRING_HPP

#include <string>
#include <cstring> // memcpy, memset

namespace unsafe
{
    template<typename C, class T = std::char_traits<C>, class A = std::allocator<C>>
    union basic_string
    {
        std::basic_string<C, T, A> s;

#if defined(_MSVC_STL_UPDATE)
        // https://github.com/microsoft/STL/blob/vs-2022-17.9/stl/inc/xstring#L4897
        // https://github.com/microsoft/STL/blob/vs-2022-17.9/stl/inc/xstring#L2202
        template<typename V> struct traits;
        template<typename V> struct traits<std::_String_iterator<V>> { using type = V; };

        auto& raw() noexcept
        {
            using _Alty = std::_Rebind_alloc_t<A, C>;
            using _Scary_val = typename traits<typename std::basic_string<C, T, A>::iterator>::type;

            auto& _Mypair = reinterpret_cast<std::_Compressed_pair<_Alty, _Scary_val>&>(s);
            static_assert(sizeof(_Mypair) == sizeof(std::basic_string<C, T, A>));
            return _Mypair._Myval2;
        }

        C* buffer() noexcept { return raw()._Myptr(); }
        std::size_t length() noexcept { return raw()._Mysize; }
        std::size_t allocated() noexcept { return raw()._Myres; }

        basic_string(C* data, std::size_t size, std::size_t capacity = 0) noexcept
        {
            std::memset(this, 0, sizeof *this);

            raw()._Bx._Ptr = data;
            raw()._Mysize = size;
            raw()._Myres = capacity < size ? size : capacity;

            capacity = std::size(raw()._Bx._Buf) - 1;
            if (raw()._Myres <= capacity) // see _Myptr()
            {
                raw()._Myres = capacity;
                std::memcpy(raw()._Bx._Buf, data, sizeof(C) * (size + 1));
            }
        }
#elif defined(__GLIBCXX__)
        // https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.2.0/libstdc++-v3/include/bits/basic_string.h#L181-L208
        auto& raw() noexcept
        {
            struct S
            {
                struct _Alloc_hider : A { C* _M_p; } _M_dataplus;
                std::size_t _M_string_length;
                std::size_t _M_allocated_capacity;
            };
            return reinterpret_cast<S&>(s);
        }

        C* buffer() noexcept { return raw()._M_dataplus._M_p; }
        std::size_t length() noexcept { return raw()._M_string_length; }
        std::size_t allocated() noexcept { return raw()._M_allocated_capacity; }

        basic_string(C* data, std::size_t size, std::size_t capacity = 0) noexcept
        {
            std::memset(this, 0, sizeof * this);

            raw()._M_dataplus._M_p = data;
            raw()._M_string_length = size;
            raw()._M_allocated_capacity = capacity < size ? size : capacity;
        }
//#elif defined(_LIBCPP_VERSION)
#else
        C* buffer() noexcept;
        std::size_t length() noexcept;
        std::size_t allocated() noexcept;
        basic_string(C* data, std::size_t size, std::size_t capacity = 0) noexcept;
#endif

        ~basic_string() {}
        basic_string() noexcept : basic_string(nullptr, 0, 0) {}
        basic_string(std::basic_string<C, T, A>& s) noexcept : basic_string(s.data(), s.size(), s.capacity()) {}
        operator std::basic_string<C, T, A>&() noexcept { return s; }
    };

    using string = basic_string<char>;
    using wstring = basic_string<wchar_t>;
    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;
#ifdef __cpp_char8_t
    using u8string = basic_string<char8_t>;
#endif

#ifdef __cpp_lib_memory_resource
    namespace pmr
    {
        template<typename C, class T = std::char_traits<C>>
        using basic_string = unsafe::basic_string<C, T, std::pmr::polymorphic_allocator<C>>;

        using string = basic_string<char>;
        using wstring = basic_string<wchar_t>;
        using u16string = basic_string<char16_t>;
        using u32string = basic_string<char32_t>;
#ifdef __cpp_char8_t
        using u8string = basic_string<char8_t>;
#endif
    }
#endif
}

#endif
