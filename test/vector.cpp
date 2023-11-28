#include "catch.hpp"

#include <version>
#ifdef __cpp_lib_memory_resource
#include <memory_resource>
#endif
#include <unsafe/vector.hpp>

TEST_CASE("vector")
{
    {
        std::vector<int> v;
        v.reserve(16);
        v.push_back(1);

        unsafe::vector uv(v);

        CHECK(uv.start() == v.data());
        CHECK(uv.finish() == v.data() + v.size());
        CHECK(uv.storage() == v.data() + v.capacity());
    }

    {
        int arr[10];
        unsafe::vector uv(arr, 1, 10);
        std::vector<int>& v = uv;

        CHECK(uv.start() == v.data());
        CHECK(uv.finish() == v.data() + v.size());
        CHECK(uv.storage() == v.data() + v.capacity());        
    }
}

#ifdef __cpp_lib_memory_resource
TEST_CASE("pmr::vector")
{
    {
        std::pmr::vector<int> v;
        v.reserve(16);
        v.push_back(1);

        unsafe::vector uv(v);

        CHECK(uv.start() == v.data());
        CHECK(uv.finish() == v.data() + v.size());
        CHECK(uv.storage() == v.data() + v.capacity());
    }

    {
        int arr[10];
        unsafe::pmr::vector<int> uv(arr, 1, 10);
        std::pmr::vector<int>& v = uv;

        CHECK(uv.start() == v.data());
        CHECK(uv.finish() == v.data() + v.size());
        CHECK(uv.storage() == v.data() + v.capacity());
    }
}
#endif
