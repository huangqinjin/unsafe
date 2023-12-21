//
// Copyright (c) 2023 Huang Qinjin (huangqinjin@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UNSAFE_POINTER_HPP
#define UNSAFE_POINTER_HPP

#include <type_traits>
#include <memory>

namespace unsafe
{
    template<typename T> 
    std::pair<T*&, void*&> unpack_ptr(std::shared_ptr<T> const& sp) noexcept
    {
        void*(&ps)[2] = reinterpret_cast<void*(&)[2]>(const_cast<std::shared_ptr<T>&>(sp));
        const int p = 0;
        return { reinterpret_cast<T*&>(ps[p]), ps[1 - p] };
    }

    template<class T>
    struct enable_shared_from_this
    {
        template<typename U>
        static auto test(std::enable_shared_from_this<U>*) -> void;
        static auto test(...) -> bool;
        static const bool value = std::is_void_v<decltype(test(static_cast<std::remove_const_t<T>*>(nullptr)))>;
    };

    template<class T>
    T* release_from_this(std::shared_ptr<T> sp)
    {
        static_assert(enable_shared_from_this<T>::value, "T must derive from std::enable_shared_from_this");
        if (!sp) return nullptr;

        auto ps = unpack_ptr(sp);
        if (ps.second != unpack_ptr(sp->shared_from_this()).second)
            throw std::bad_weak_ptr();

        ps.second = nullptr;
        return ps.first;
    }

    template<class T>
    std::shared_ptr<T> shared_from_this(T* p)
    {
        static_assert(enable_shared_from_this<T>::value, "T must derive from std::enable_shared_from_this");
        if (p == nullptr) return {};

        auto sp = p->shared_from_this();
        unpack_ptr(std::shared_ptr<void>{}).second = unpack_ptr(sp).second;
        return { std::move(sp), p };
    }
}

#endif
