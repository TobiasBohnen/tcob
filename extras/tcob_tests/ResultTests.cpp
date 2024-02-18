#include "tests.hpp"

#include <optional>

enum class error {
    Ok,
    Error1,
    Error2
};

TEST_CASE("Core.Result.ValueOr")
{
    int const testValue {100};
    int const orValue {200};

    {
        auto const value = result<int, error> {testValue}.value_or(orValue);
        REQUIRE(value == testValue);
    }
    {
        auto const value = result<int, error> {error::Error1}.value_or(orValue);
        REQUIRE(value == orValue);
    }
}

TEST_CASE("Core.Result.AndThen")
{
    int const testValue {100};

    auto andThen = [](int val) { return result<int, error> {val * 2}; };

    {
        auto const value = result<int, error> {testValue}.and_then(andThen);
        REQUIRE(*value == testValue * 2);
    }
    {
        auto const value  = result<int, error> {testValue};
        auto const value1 = value.and_then(andThen);
        REQUIRE(*value1 == testValue * 2);
    }
    {
        auto const value  = result<int, error> {testValue};
        auto const value1 = value.and_then(andThen);
        REQUIRE(*value1 == testValue * 2);
    }
    {
        auto const value = result<int, error> {testValue}
                               .and_then(andThen)
                               .and_then(andThen)
                               .and_then(andThen);
        REQUIRE(*value == testValue * 2 * 2 * 2);
    }
    {
        auto const value = result<int, error> {error::Error1}.and_then(andThen);
        REQUIRE(*value == int {});
    }
}

TEST_CASE("Core.Result.OrElse")
{
    int const testValue {100};
    int const error1Value {200};
    int const error2Value {250};

    auto orElse = [&](error err) {
        switch (err) {
        case error::Error1:
            return result<int, error> {error1Value};
        case error::Error2:
            return result<int, error> {error2Value};
        case error::Ok:
            break;
        }
        return result<int, error> {err};
    };

    {
        auto const value = result<int, error> {testValue}.or_else(orElse);
        REQUIRE(*value == testValue);
    }
    {
        auto const value  = result<int, error> {testValue};
        auto const value1 = value.or_else(orElse);
        REQUIRE(*value1 == testValue);
    }
    {
        auto const value = result<int, error> {error::Error1}.or_else(orElse);
        REQUIRE(*value == error1Value);
    }
    {
        auto const value = result<int, error> {error::Error2}.or_else(orElse);
        REQUIRE(*value == error2Value);
    }
}

TEST_CASE("Core.Result.Transform")
{
    int const  testValue {100};
    auto const xform = [](int val) -> f32 { return static_cast<f32>(val) * 2.0f; };

    {
        auto const value = result<int, error> {testValue}.transform(xform);
        REQUIRE(*value == testValue * 2.0f);
    }
    {
        auto const value  = result<int, error> {testValue};
        auto const value1 = value.transform(xform);
        REQUIRE(*value1 == testValue * 2.0f);
    }
    {
        auto const value  = result<int, error> {testValue};
        auto const value1 = value.transform(xform);
        REQUIRE(*value1 == testValue * 2.0f);
    }
    {
        auto const value = result<int, error> {error::Error1}.transform(xform);
        REQUIRE(*value == f32 {});
    }
    {
        auto const value = result<int, error> {error::Error2}.transform(xform);
        REQUIRE(*value == f32 {});
    }
}

TEST_CASE("Core.Result.MemberAccess")
{
    result<point_f, error> const value {{1.2f, 3.4f}};
    REQUIRE(value->X == 1.2f);
    REQUIRE(value->Y == 3.4f);
}
