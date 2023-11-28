//
// Copyright (c) 2023 Huang Qinjin (huangqinjin@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UNSAFE_VECTOR_HPP
#define UNSAFE_VECTOR_HPP

#include <vector>
#include <cstring> // memcpy, memset

namespace unsafe
{
    template<typename T, class A = std::allocator<T>>
    union vector
    {
        std::vector<T, A> v;

#if defined(_MSVC_STL_UPDATE)
        // https://github.com/microsoft/STL/blob/vs-2022-17.9/stl/inc/vector#L2206
        // https://github.com/microsoft/STL/blob/vs-2022-17.9/stl/inc/vector#L386
        template<class V> struct traits;
        template<class V> struct traits<std::_Vector_iterator<V>> { using type = V; };

        auto& raw() noexcept
        {
            using _Alty = std::_Rebind_alloc_t<A, T>;
            using _Scary_val = typename traits<typename std::vector<T, A>::iterator>::type;

            auto& _Mypair = reinterpret_cast<std::_Compressed_pair<_Alty, _Scary_val>&>(v);
            static_assert(sizeof(_Mypair) == sizeof(std::vector<T, A>));
            return _Mypair._Myval2;
        }

        T* start() noexcept { return raw()._Myfirst; }
        T* finish() noexcept { return raw()._Mylast; }
        T* storage() noexcept { return raw()._Myend; }

        vector(T* data, std::size_t size, std::size_t capacity = 0) noexcept
        {
            std::memset(this, 0, sizeof *this);

            raw()._Myfirst = data;
            raw()._Mylast = data + size;
            raw()._Myend = data + (capacity < size ? size : capacity);
        }
#elif defined(__GLIBCXX__)
        // https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.2.0/libstdc++-v3/include/bits/stl_vector.h#L371
        auto& raw() noexcept
        {
            using _Base = std::_Vector_base<T, A>;
            static_assert(std::is_base_of_v<_Base, std::vector<T, A>>);
            return ((_Base&)v)._M_impl;
        }

        T* start() noexcept { return raw()._M_start; }
        T* finish() noexcept { return raw()._M_finish; }
        T* storage() noexcept { return raw()._M_end_of_storage; }

        vector(T* data, std::size_t size, std::size_t capacity = 0) noexcept
        {
            std::memset(this, 0, sizeof *this);

            raw()._M_start = data;
            raw()._M_finish = data + size;
            raw()._M_end_of_storage = data + (capacity < size ? size : capacity);
        }
#elif defined(_LIBCPP_VERSION)
        // https://github.com/llvm/llvm-project/blob/llvmorg-17.0.1/libcxx/include/vector#L741-L744
        auto& raw() noexcept
        {
            return reinterpret_cast<T*(&)[3]>(v);
        }

        T* start() noexcept { return raw()[0]; }
        T* finish() noexcept { return raw()[1]; }
        T* storage() noexcept { return raw()[2]; }

        vector(T* data, std::size_t size, std::size_t capacity = 0) noexcept
        {
            std::memset(this, 0, sizeof *this);

            raw()[0] = data;
            raw()[1] = data + size;
            raw()[2] = data + (capacity < size ? size : capacity);
        }
#else
        T* start() noexcept;
        T* finish() noexcept;
        T* storage() noexcept;
        vector(T* data, std::size_t size, std::size_t capacity = 0) noexcept;
#endif
 
        ~vector() {}
        vector() noexcept : vector(nullptr, 0, 0) {}
        vector(std::vector<T, A>& v) noexcept : vector(v.data(), v.size(), v.capacity()) {}
        operator std::vector<T, A>&() noexcept { return v; }
    };

#ifdef __cpp_lib_memory_resource
    namespace pmr
    {
        template<typename T>
        using vector = unsafe::vector<T, std::pmr::polymorphic_allocator<T>>;
    }
#endif
}

#endif
