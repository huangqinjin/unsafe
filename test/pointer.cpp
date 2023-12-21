#include "catch.hpp"

#include <unsafe/pointer.hpp>

TEST_CASE("shared_ptr")
{
    struct A : std::enable_shared_from_this<A> {};
    struct B : A {};

    auto sp = std::make_shared<B>();
    CHECK(sp.use_count() == 1);
    
    B* p = nullptr;
    CHECK_NOTHROW(p = unsafe::release_from_this(sp));
    CHECK(sp.get() == p);
    CHECK(sp.use_count() == 2);

    CHECK_NOTHROW(sp = unsafe::shared_from_this(p));
    CHECK(sp.use_count() == 1);

    B b;
    sp = std::shared_ptr<B>(std::make_shared<int>(), &b);
    CHECK_THROWS_AS(unsafe::shared_from_this(&b), std::bad_weak_ptr);
    CHECK_THROWS_AS(unsafe::release_from_this(sp), std::bad_weak_ptr);
}
