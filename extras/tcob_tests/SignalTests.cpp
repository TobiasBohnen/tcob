#include "tests.hpp"

i32 const ExpValue = 45234;

static i32 Value {0};

struct SignalTest {
    void test(i32 val)
    {
        Value = val;
    }
};

void static Test(i32 val)
{
    Value = val;
}

TEST_CASE("Core.Signal.Connect")
{
    SUBCASE("Member Function")
    {
        signal<i32 const> sig0;
        SignalTest        inst;
        REQUIRE(Value != ExpValue);
        sig0.connect<&SignalTest::test>(&inst);
        sig0(ExpValue);
        REQUIRE(Value == ExpValue);
        Value = 0;
    }
    SUBCASE("Free Function")
    {
        signal<i32 const> sig0;
        REQUIRE(Value != ExpValue);
        sig0.connect(&Test);
        sig0(ExpValue);
        REQUIRE(Value == ExpValue);
        Value = 0;
    }
    SUBCASE("Lambda")
    {
        {
            signal<i32 const> sig0;
            i32               value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value](i32 val) { value = val; });
            sig0(ExpValue);
            REQUIRE(value == ExpValue);
        }
        {
            signal<i32 const> sig0;
            i32               value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value]() { value = ExpValue * 10; });
            sig0(ExpValue);
            REQUIRE(value == ExpValue * 10);
        }
    }
}

TEST_CASE("Core.Signal.Disconnect")
{
    {
        signal<> sig0;
        REQUIRE(sig0.get_slot_count() == 0);
        auto id0 = sig0.connect([]() {}).get_id();
        auto id1 = sig0.connect([]() {}).get_id();
        REQUIRE(sig0.get_slot_count() == 2);
        sig0.disconnect(id0);
        REQUIRE(sig0.get_slot_count() == 1);
        sig0.disconnect(id1);
        REQUIRE(sig0.get_slot_count() == 0);
    }
    {
        signal<> sig0;
        REQUIRE(sig0.get_slot_count() == 0);
        auto id0 = sig0.connect([]() {}).get_id();
        auto id1 = sig0.connect([]() {}).get_id();
        REQUIRE(sig0.get_slot_count() == 2);
        sig0.disconnect(id1);
        REQUIRE(sig0.get_slot_count() == 1);
        sig0.disconnect(id0);
        REQUIRE(sig0.get_slot_count() == 0);
    }
}

TEST_CASE("Core.Signal.Lifetime")
{
    SUBCASE("Scoped Connection")
    {
        signal<> sig0;
        REQUIRE(sig0.get_slot_count() == 0);
        {
            scoped_connection conn {sig0.connect([] {})};
            REQUIRE(sig0.get_slot_count() == 1);
        }
        REQUIRE(sig0.get_slot_count() == 0);
    }

    SUBCASE("Connection")
    {
        signal<> sig0;
        REQUIRE(sig0.get_slot_count() == 0);
        {
            std::function<void()> l {};
            connection            conn {sig0.connect(l)};
            REQUIRE(sig0.get_slot_count() == 1);
        }

        REQUIRE(sig0.get_slot_count() == 1);
        sig0();
        REQUIRE(sig0.get_slot_count() == 0);
    }
}

TEST_CASE("Core.Signal.Handled")
{
    struct event_base {
        bool Handled {false};
    };

    {
        signal<event_base> sig0;
        event_base         ev;
        sig0.connect([](auto&& arg) { arg.Handled = true; });
        sig0(ev);
        REQUIRE(ev.Handled);
    }
    {
        signal<event_base> sig0;
        event_base         ev;
        i32                callCount {0};
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&& arg) { callCount++; arg.Handled = true; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0(ev);
        REQUIRE(ev.Handled);
        REQUIRE(callCount == 2);
    }
}
