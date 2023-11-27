//
// Copyright (c) 2023 Huang Qinjin (huangqinjin@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UNSAFE_IOSTREAM_HPP
#define UNSAFE_IOSTREAM_HPP

#include <iostream>
#include <fstream>
#include <cstdio> // fileno
#include <cstring> // strncmp

#ifdef _WIN32
#include <io.h> // _get_osfhandle
#endif

#if defined(__GLIBCXX__)
#include <ext/stdio_sync_filebuf.h>
#endif

#include "bind.hpp"

#if defined(_LIBCPP_VERSION)
UNSAFE_BIND(std::filebuf, ~0, __file_);
UNSAFE_BIND(std::wfilebuf, ~0, __file_);
#elif defined(_MSVC_STL_UPDATE)
UNSAFE_BIND(std::filebuf, ~0, _Myfile);
UNSAFE_BIND(std::wfilebuf, ~0, _Myfile);
#endif

namespace unsafe
{
    template<class CharT, class Traits>
    FILE* filebuf_FILE(const std::basic_filebuf<CharT, Traits>* buf) noexcept
    {
#if defined(__GLIBCXX__)
        struct access : std::basic_filebuf<CharT, Traits> {
            FILE* get() { return this->_M_file.file(); }
        }; return const_cast<access*>(static_cast<const access*>(buf))->get();
#else
        return unsafe::get<~0>(*buf);
#endif
    }

    template<class CharT, class Traits>
    int filebuf_fileno(const std::basic_filebuf<CharT, Traits>* buf) noexcept
    {
#if defined(__GLIBCXX__)
        struct access : std::basic_filebuf<CharT, Traits> {
            int get() { return this->_M_file.fd(); }
        }; return const_cast<access*>(static_cast<const access*>(buf))->get();
#elif defined(_WIN32)
        return _fileno(filebuf_FILE(buf));
#else
        return fileno(filebuf_FILE(buf));
#endif
    }

    template<class CharT, class Traits>
    auto filebuf_native_handle(const std::basic_filebuf<CharT, Traits>* buf) noexcept
    {
#if defined(__cpp_lib_fstream_native_handle)
        return buf->native_handle();
#elif defined(_WIN32)
        return (void*)_get_osfhandle(filebuf_fileno(buf));
#else
        return filebuf_fileno(buf);
#endif
    }

    template<class CharT, class Traits>
    FILE* streambuf_FILE(const std::basic_streambuf<CharT, Traits>* buf) noexcept
    {
        if (auto f = dynamic_cast<const std::basic_filebuf<CharT, Traits>*>(buf))
            return filebuf_FILE(f);
#if defined(__GLIBCXX__)
        if (auto f = dynamic_cast<__gnu_cxx::stdio_sync_filebuf<CharT, Traits>*>(
            const_cast<std::basic_streambuf<CharT, Traits>*>(buf)))
            return f->file();
#elif defined(_LIBCPP_VERSION)
        // https://github.com/llvm/llvm-project/blob/llvmorg-17.0.1/libcxx/src/std_stream.h
        auto& f = *buf;
        const char* s = typeid(f).name();
        if (s && (
            std::strncmp(s, "NSt3__111__stdoutbuf", 20) == 0 ||
            std::strncmp(s, "NSt3__110__stdinbuf", 19) == 0))
        {
            return *(FILE**)(buf + 1);
        }
#endif
        return nullptr;
    }

    template<class CharT, class Traits>
    int streambuf_fileno(const std::basic_streambuf<CharT, Traits>* buf) noexcept
    {
        if (auto f = dynamic_cast<const std::basic_filebuf<CharT, Traits>*>(buf))
            return filebuf_fileno(f);
        if (auto f = streambuf_FILE(buf))
#if defined(_WIN32)
            return _fileno(f);
#else
            return fileno(f);
#endif
        return -1;
    }

    template<class CharT, class Traits>
    auto streambuf_native_handle(const std::basic_streambuf<CharT, Traits>* buf) noexcept
    {
        if (auto f = dynamic_cast<const std::basic_filebuf<CharT, Traits>*>(buf))
            return filebuf_native_handle(f);
        if (auto f = streambuf_FILE(buf))
#if defined(_WIN32)
            return (void*)_get_osfhandle(_fileno(f));
#else
            return fileno(f);
#endif
        return decltype(filebuf_native_handle<CharT, Traits>(nullptr))(-1);
    }
}

#endif
