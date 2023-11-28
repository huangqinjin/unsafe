#include "catch.hpp"

#include <version>
#ifdef __cpp_lib_memory_resource
#include <memory_resource>
#endif
#include <unsafe/string.hpp>

#ifndef _LIBCPP_VERSION

TEST_CASE("string")
{
    const bool sso_enabled = GENERATE(true, false);
    CAPTURE(sso_enabled);
    
    {
        std::string s;
        if (!sso_enabled) s.reserve(512);
        s.push_back('x');

        unsafe::string us(s);

        if (!sso_enabled) CHECK(us.buffer() == s.data());
        else CHECK(us.buffer() == s);
        CHECK(us.length() == s.size());
        CHECK(us.allocated() == s.capacity());
    }

    {
        char arr[512] = "x";
        unsafe::string us(arr, 1, sso_enabled ? 0 : 512);
        std::string& s = us;

        if (!sso_enabled) CHECK(us.buffer() == arr);
        CHECK(us.buffer() == s.data());
        CHECK(us.length() == s.size());
        CHECK(us.allocated() == s.capacity());
    }
}

#ifdef __cpp_lib_memory_resource
TEST_CASE("pmr::string")
{
    const bool sso_enabled = GENERATE(true, false);
    CAPTURE(sso_enabled);
    
    {
        std::pmr::string s;
        if (!sso_enabled) s.reserve(512);
        s.push_back('x');

        unsafe::pmr::string us(s);

        if (!sso_enabled) CHECK(us.buffer() == s.data());
        else CHECK(us.buffer() == s);
        CHECK(us.length() == s.size());
        CHECK(us.allocated() == s.capacity());
    }

    {
        char arr[512] = "x";
        unsafe::pmr::string us(arr, 1, sso_enabled ? 0 : 512);
        std::pmr::string& s = us;

        if (!sso_enabled) CHECK(us.buffer() == arr);
        CHECK(us.buffer() == s.data());
        CHECK(us.length() == s.size());
        CHECK(us.allocated() == s.capacity());
    }
}
#endif

#endif
