#include "WrapperTestsClass.hpp"
#include "tests.hpp"

#include <numeric>

using namespace tcob::scripting;
using namespace tcob::scripting::squirrel;

class SquirrelWrapperTests : public squirrel::script {
public:
    SquirrelWrapperTests()
        : global(get_root_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(SquirrelWrapperTests, "Script.SquirrelWrapper.Type")
{
    auto wrapper = create_wrapper<TestScriptClass>("TSC");

    wrapper->wrap_method<&TestScriptClass::foo>("foo");
    wrapper->wrap_method<&TestScriptClass::abstractMethod>("abstract");
    wrapper->wrap_method<&TestScriptClass::virtualMethod>("virtual");
    wrapper->wrap_method<&TestScriptClass::baseMethod>("basem");
    wrapper->wrap_property<&TestScriptClass::FieldValue>("field");
    wrapper->wrap_property<&TestScriptClass::get_value, &TestScriptClass::set_value>("value");

    auto f1 = resolve_overload<i32, f32>(&TestScriptClass::overload);
    auto f2 = resolve_overload<f32, i32>(&TestScriptClass::overload);
    auto f3 = resolve_overload<std::vector<f32> const&>(&TestScriptClass::overload);
    wrapper->wrap_overload("overload", f1, f2, f3);

    SUBCASE("pointer ping pong")
    {
        op_test_class f {1};

        global["foo"]       = &f;
        op_test_class* fptr = global["foo"];
        REQUIRE(&f == fptr);

        global["foo1"]       = &f;
        op_test_class* fptr1 = global["foo1"];
        REQUIRE(&f == fptr1);
    }
    SUBCASE("functions")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        auto res       = run<i32>("return wrap.foo(\"test\", 4, true)");
        // auto res       = run("wrap:bar(true, 'test', 4)");
        REQUIRE_FALSE(res.has_error());
        REQUIRE(res);
        REQUIRE(res.value() == 4 * 4);
    }
    SUBCASE("properties")
    {
        {
            TestScriptClass t;
            global["wrap"] = &t;
            t.set_value(420);
            t.FieldValue = 100;

            auto res = run<i32>("return wrap.field");
            REQUIRE(res);
            REQUIRE(res.value() == t.FieldValue);

            res = run<i32>("return wrap.value");
            REQUIRE(res);
            REQUIRE(res.value() == t.get_value());
        }

        {
            TestScriptClass t;
            global["wrap"] = &t;

            REQUIRE_FALSE(t.FieldValue == 100);
            REQUIRE_FALSE(t.get_value() == 420);
            auto res = run("wrap.field = 100; wrap.value = 420");

            REQUIRE(res);
            REQUIRE(t.FieldValue == 100);
            REQUIRE(t.get_value() == 420);
        }
    }
    SUBCASE("overloads")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        f32 x          = *run<f32>("return wrap.overload([0.2,0.4])");
        REQUIRE(x == t.overload(std::vector<f32> {0.2f, 0.4f}));

        x = *run<f32>("return wrap.overload(4, 2.0)");
        REQUIRE(x == t.overload(4, 2.0f));

        x = *run<f32>("return wrap.overload(2.0, 12)");
        REQUIRE(x == t.overload(2.0f, 12));
    }
    SUBCASE("release")
    {
        auto* t1        = new TestScriptClass; // NOLINT
        global["wrap1"] = script_owned_ptr {t1};
        REQUIRE(TestScriptClass::ObjCount == 1);

        global["wrap1"] = nullptr;
        REQUIRE(TestScriptClass::ObjCount == 0);
    }
    SUBCASE("inheritance")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        i32 x          = *run<i32>("return wrap.virtual()");
        REQUIRE(x == 84);
        i32 y = *run<i32>("return wrap.abstract()");
        REQUIRE(y == 42);
        i32 z = *run<i32>("return wrap.basem()");
        REQUIRE(z == 21);
    }
}

TEST_CASE_FIXTURE(SquirrelWrapperTests, "Script.SquirrelWrapper.DefaultMetamethods")
{
    auto wrapper = create_wrapper<op_test_class>("ops");
    SUBCASE("LessOrEqualThan")
    {
        op_test_class t1 {4000};
        global["wrap1"] = &t1;
        op_test_class t2 {4000};
        global["wrap2"] = &t2;

        REQUIRE(*run<bool>("return wrap1 <= wrap2"));
        REQUIRE(*run<bool>("return wrap1 >= wrap2"));
        t1.value = 200;
        REQUIRE(*run<bool>("return wrap1 <= wrap2"));
        REQUIRE_FALSE(*run<bool>("return wrap1 >= wrap2"));
    }
    SUBCASE("LessThan")
    {
        op_test_class t1 {2000};
        global["wrap1"] = &t1;
        op_test_class t2 {4000};
        global["wrap2"] = &t2;

        REQUIRE(*run<bool>("return wrap1 < wrap2"));
        REQUIRE_FALSE(*run<bool>("return wrap1 > wrap2"));
        t1.value = 8000;
        REQUIRE_FALSE(*run<bool>("return wrap1 < wrap2"));
        REQUIRE(*run<bool>("return wrap1 > wrap2"));
    }
    SUBCASE("Equal")
    {
        op_test_class t1 {4000};
        global["wrap1"] = &t1;
        op_test_class t2 {4000};
        global["wrap2"] = &t2;

        REQUIRE(*run<bool>("return (wrap1 <=> wrap2) == 0"));
        REQUIRE_FALSE(*run<bool>("return (wrap1 <=> wrap2) != 0"));
        t1.value = 8000;
        REQUIRE_FALSE(*run<bool>("return (wrap1 <=> wrap2) == 0"));
        REQUIRE(*run<bool>("return (wrap1 <=> wrap2) != 0"));
    }
    SUBCASE("Add,Subtract,Divide,Multiply")
    {
        op_test_class t1 {4000};
        global["wrap1"] = &t1;
        op_test_class t2 {2000};
        global["wrap2"] = &t2;

        op_test_class* b = *run<op_test_class*>("wrap3 <- wrap1 + wrap2; return wrap3");
        REQUIRE(b->value == t1.value + t2.value);

        b = *run<op_test_class*>("wrap3 <- wrap1 - wrap2; return wrap3");
        REQUIRE(b->value == t1.value - t2.value);

        b = *run<op_test_class*>("wrap3 <- wrap1 / wrap2; return wrap3");
        REQUIRE(b->value == t1.value / t2.value);

        b = *run<op_test_class*>("wrap3 <- wrap1 * wrap2; return wrap3");
        REQUIRE(b->value == t1.value * t2.value);
    }
    SUBCASE("UnaryMinus")
    {
        op_test_class t1 {4000};
        global["wrap1"] = &t1;

        op_test_class* b = *run<op_test_class*>("wrap3 <- -wrap1; return wrap3");
        REQUIRE(b->value == -4000);
    }
}

TEST_CASE_FIXTURE(SquirrelWrapperTests, "Script.SquirrelWrapper.Metamethods")
{
    auto wrapper = create_wrapper<TestScriptClass>("TSCB");
    wrapper->wrap_metamethod(
        metamethod::Add, [](TestScriptClass* instance1, i32 x) { return instance1->get_value() + x; });
    wrapper->wrap_metamethod(
        metamethod::Subtract, [](TestScriptClass* instance1, i32 x) { return instance1->get_value() - x; });
    wrapper->wrap_metamethod(
        metamethod::Divide, [](TestScriptClass* instance1, i32 x) { return instance1->get_value() / x; });
    wrapper->wrap_metamethod(
        metamethod::Multiply, [](TestScriptClass* instance1, i32 x) { return instance1->get_value() * x; });
    wrapper->wrap_metamethod(
        metamethod::UnaryMinus, [](TestScriptClass* instance1) { return -instance1->get_value(); });
    wrapper->wrap_metamethod(
        metamethod::ToString, [](TestScriptClass* instance1) {
            return std::to_string(instance1->get_value());
        });
    wrapper->wrap_metamethod(
        metamethod::Call, [](TestScriptClass* instance1, table const& /*env*/, i32 x, i32 y) {
            return x * instance1->get_value() * y;
        });

    SUBCASE("Call")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 b = *run<i32>("return wrap1(100, 5)");
        REQUIRE(b == value1 * 100 * 5);
    }
    SUBCASE("ToString")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        std::string b = *run<std::string>("return wrap1.tostring()");
        REQUIRE(b == std::to_string(t1.get_value()));
    }

    SUBCASE("Add,Subtract,Divide,Multiply")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 value2       = 200;
        global["value2"] = value2;

        i32 b = *run<i32>("return wrap1 + value2");
        REQUIRE(b == value1 + value2);

        b = *run<i32>("return wrap1 - value2");
        REQUIRE(b == value1 - value2);

        b = *run<i32>("return wrap1 / value2");
        REQUIRE(b == value1 / value2);

        b = *run<i32>("return wrap1 * value2");
        REQUIRE(b == value1 * value2);
    }
    SUBCASE("UnaryMinus")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(100);

        i32 b = *run<i32>("return -wrap1");
        REQUIRE(b == -100);
    }
}

TEST_CASE_FIXTURE(SquirrelWrapperTests, "Script.SquirrelWrapper.UnknownGetHandler")
{
    struct bar {
        int  x {0};
        auto y() -> int { return 400; }
        int  z {0};
    };

    auto wrap = create_wrapper<bar>("bar");

    SUBCASE("unhandled setter")
    {
        auto res = run("function foo(p){ p.unhandled_newindex=400 }");
        REQUIRE(!res.has_error());
        squirrel::function<void> f = get_root_table()["foo"];
        bar                      test {};
        //   REQUIRE(f.call(&test).has_error()); //TODO: should be an error
        REQUIRE_FALSE(test.z == 400);
    }
    SUBCASE("handled setter")
    {
        auto res = run("function foo(p){ p.z=400 }");
        REQUIRE(!res.has_error());
        squirrel::function<void> f = get_root_table()["foo"];

        wrap->UnknownSet.connect([](auto&& ev) {
            if (ev.Name == "z") {
                ev.get_value(ev.Instance->z);
                ev.Handled = true;
            }
        });

        bar test {};
        REQUIRE_FALSE(f.call(&test).has_error());
        REQUIRE(test.z == 400);
    }
    SUBCASE("unhandled function")
    {
        auto res = run("function foo(p) { return p.unhandled_index() }");
        REQUIRE(!res.has_error());
        squirrel::function<int> f = get_root_table()["foo"];
        bar                     test {};
        auto                    funcres = f.call(&test);
        REQUIRE(funcres.has_error());
    }
    SUBCASE("handled function")
    {
        auto res = run("function foo(p) { return p.y() }");
        REQUIRE(!res.has_error());
        squirrel::function<int> f = get_root_table()["foo"];

        auto yfunc = squirrel::make_shared_closure(std::function([](bar* fx) { return fx->y(); }));
        wrap->UnknownGet.connect([yfunc](auto&& ev) {
            if (ev.Name == "y") {
                ev.return_value(yfunc.get());
                ev.Handled = true;
            }
        });

        bar  test {};
        auto funcres = f.call(&test);
        REQUIRE_FALSE(funcres.has_error());
        REQUIRE(funcres.value() == test.y());
    }
    SUBCASE("unhandled getter")
    {
        auto res = run("function foo(p) { return p.unhandled_index }");
        REQUIRE(!res.has_error());
        squirrel::function<int> f = get_root_table()["foo"];
        bar                     test {};
        test.x       = 420;
        auto funcres = f.call(&test);
        REQUIRE(funcres.has_error());
    }
    SUBCASE("handled getter")
    {
        auto res = run("function foo(p) { return p.x }");
        REQUIRE(!res.has_error());
        squirrel::function<int> f = get_root_table()["foo"];

        wrap->UnknownGet.connect([](auto&& ev) {
            if (ev.Name == "x") {
                ev.return_value(ev.Instance->x);
                ev.Handled = true;
            }
        });

        bar test {};
        test.x       = 420;
        auto funcres = f.call(&test);
        REQUIRE_FALSE(funcres.has_error());
        REQUIRE(funcres.value() == test.x);
    }
}
