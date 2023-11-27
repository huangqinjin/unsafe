#include "catch.hpp"

#include <unsafe/bind.hpp>

namespace
{
    class B
    {
        int x = 1234;
        int f() { return x; }
        int f() const { return -x; }
    };

    class D : public B
    {
        int y = 4321;
        int g() & { return y; }
        int g() && { return -y; } 
    };
}

UNSAFE_BIND(B, 0, x)
UNSAFE_BIND_T(B, 1, f, int (B::*)())
UNSAFE_BIND_T(B, 2, f, int (B::*)() const)
UNSAFE_BIND(D, 0, y)
UNSAFE_BIND_T(D, 1, g, int (D::*)() &)
UNSAFE_BIND_T(D, 2, g, int (D::*)() &&)

TEST_CASE("bind")
{
    D d;
    B& b = d;

    static_assert(std::is_same_v<decltype(unsafe::get<0>((B&)b)), int&>);
    static_assert(std::is_same_v<decltype(unsafe::get<0>((B&&)b)), int&&>);
    static_assert(std::is_same_v<decltype(unsafe::get<0>((const B&)b)), const int&>);

    CHECK(unsafe::get<0>(b) == 1234);
    CHECK(unsafe::get<1>(b)() == 1234);
    CHECK(unsafe::get<2>(std::as_const(b))() == -1234);

    CHECK(unsafe::get<0>(d) == 4321);
    CHECK(unsafe::get<1>(d)() == 4321);
    CHECK(unsafe::get<2>(std::move(d))() == -4321);
}
