#include "tests.hpp"

TEST_CASE("Core.Property.Copying")
{
    prop<i32> test {10};
    REQUIRE(test == 10);
    auto test2 {test()};
    REQUIRE(test2 == 10);
    test2 = 20;
    REQUIRE(test2 == 20);
    REQUIRE(test == 10);
}

TEST_CASE("Core.Property.Event")
{
    bool      changed {false};
    prop<i32> vt;
    vt.Changed.connect([&](i32) { changed = true; });
    REQUIRE_FALSE(vt == 100);
    vt = {100};
    REQUIRE(vt == 100);
    REQUIRE(changed == true);
}

TEST_CASE("Core.Property.Validate")
{
    {
        i32 const     expected = 32;
        prop_val<i32> prop {{[&](i32 const&) { return expected; }}};
        prop.Changed.connect([&](i32 const& val) { REQUIRE(val == expected); });
        prop = 300;
        REQUIRE(expected == 32);
        REQUIRE(prop == expected);
    }
    {
        i32 const     expected = 32;
        prop_val<i32> prop {{12, [&](i32 const&) { return expected; }}};
        REQUIRE(prop == 12);
        prop.Changed.connect([&](i32 const& val) { REQUIRE(val == expected); });
        prop = 300;
        REQUIRE(expected == 32);
        REQUIRE(prop == expected);
    }
}

TEST_CASE("Core.Property.Funcs")
{
    i32          result {0};
    prop_fn<i32> prop {{12, [&]() { return result; }, [&](i32 val) { result = val; }}};
    REQUIRE(result == 12);
    REQUIRE(prop == 12);

    prop = 300;
    REQUIRE(result == 300);
    REQUIRE(prop == result);
}

TEST_CASE("Core.Property.Value")
{
    {
        prop<i32> prop;
        prop = 234;
        REQUIRE(prop == 234);
        prop = 500;
        REQUIRE(prop == 500);
    }
    {
        prop<f32> prop;
        prop = 23.4f;
        REQUIRE(prop == 23.4f);
        prop = 50.0f;
        REQUIRE(prop == 50.0f);
    }
    {
        prop<u8> prop {u8 {9}};
        REQUIRE(prop == 9);
        prop = 12;
        REQUIRE(prop == 12);
    }
    {
        prop<bool> prop {};
        REQUIRE(prop == false);
        prop = true;
        REQUIRE(prop == true);
    }
}

TEST_CASE("Core.Property.Compare")
{
    {
        prop<f32> prop {23.4f};
        REQUIRE(prop == 23.4f);
        REQUIRE(prop != 150.0f);
    }
    {
        prop<i32> prop {234};
        REQUIRE(prop > 1);
        REQUIRE(prop < 600);
    }
    {
        prop<f32> prop {23.4f};
        REQUIRE(prop > 1.0f);
        REQUIRE(prop < 150.0f);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {23.4f};
        REQUIRE(prop0 == prop1);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {12.3f};
        REQUIRE(prop0 != prop1);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {12.3f};
        REQUIRE(prop0 > prop1);
    }
    {
        prop<f32> prop0 {12.3f};
        prop<f32> prop1 {23.4f};
        REQUIRE(prop0 < prop1);
    }
    {
        prop<degree_f> prop0 {12.3f};
        REQUIRE(prop0 > degree_f {2.3f});
    }
    {
        prop<degree_f> prop1 {23.4f};
        prop<degree_f> prop0 {12.3f};
        REQUIRE(prop0 < prop1);
    }
}

TEST_CASE("Core.Property.Arithmetic")
{
    SUBCASE("+")
    {
        {
            prop<i32> prop;
            prop = 234;
            prop += 100;
            REQUIRE(prop == 334);
        }
        {
            prop<point_i> prop {};
            prop = {123, 456};
            prop += {100, 200};
            REQUIRE(prop == point_i {223, 656});
        }
        {
            prop<point_i> prop {};
            prop = {123, 456};
            prop += point_i {100, 200};
            REQUIRE(prop == point_i {223, 656});
            prop += point_i {100, 200};
            REQUIRE(prop == point_i {323, 856});
        }
        {
            prop<i32> prop;
            prop = 0;
            prop = prop + 100;
            REQUIRE(prop == 100);
            prop = 100 + prop;
            REQUIRE(prop == 200);
        }
    }
    SUBCASE("-")
    {
        {
            prop<i32> prop;
            prop = 234;
            prop -= 100;
            REQUIRE(prop == 134);
        }
        {
            prop<point_i> prop {};
            prop = {123, 456};
            prop -= {100, 200};
            REQUIRE(prop == point_i {23, 256});
        }
        {
            prop<i32> prop;
            prop = 200;
            prop = prop - 100;
            REQUIRE(prop == 100);
            prop = 100 - prop;
            REQUIRE(prop == 0);
        }
    }
    SUBCASE("*")
    {
        {
            prop<i32> prop;
            prop = 10;
            prop *= 100;
            REQUIRE(prop == 1000);
        }
        {
            prop<point_i> prop {};
            static_assert(std::is_same_v<decltype(prop.operator->()), point_i const*>);
            prop = {123, 456};
            prop *= {100, 200};
            REQUIRE(prop == point_i {12300, 91200});
        }
        {
            prop<i32> prop;
            prop = 10;
            prop = prop * 100;
            REQUIRE(prop == 1000);
            prop = 100 * prop;
            REQUIRE(prop == 100000);
        }
    }
    SUBCASE("/")
    {
        {
            prop<i32> prop;
            prop = 200;
            prop /= 100;
            REQUIRE(prop == 2);
        }
        {
            prop<point_i> prop {};
            prop = {500, 600};
            prop /= {100, 200};
            REQUIRE(prop == point_i {5, 3});
        }
        {
            prop<i32> prop;
            prop = 234;
            prop = prop / 100;
            REQUIRE(prop == 2);
            prop = 100 / prop;
            REQUIRE(prop == 50);
        }
    }
}

template <typename T>
class queue_source final {
public:
    using type              = std::remove_const_t<T>;
    using return_type       = type;
    using const_return_type = type;

    auto get() const -> const_return_type
    {
        auto front {_values.front()};
        _values.pop();
        return front;
    }

    auto set(type const& value, bool) -> bool
    {
        _values.push(value);
        return true;
    }

private:
    mutable std::queue<type> _values {};
};

TEST_CASE("Core.Property.CustomSource")
{
    prop_base<i32, queue_source<i32>> prop;
    prop = 100;
    prop = 200;
    prop = 400;

    REQUIRE(prop == 100);
    REQUIRE(prop == 200);
    REQUIRE(prop == 400);
}
