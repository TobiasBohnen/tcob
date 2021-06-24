#include "LuaScriptTestsClass.hpp"
#include "LuaScriptTestsHelper.hpp"
#include "tests.hpp"

class LuaWrapperTests : public LuaScript {
public:
    LuaWrapperTests()
    {
        global = global_table();
        open_libraries();
        register_searcher(&openrequire);
    }

    LuaTable global;
};

TEST_CASE_METHOD(LuaWrapperTests, "Script.Wrapper.MapWrapper")
{
    create_wrapper<map<string, i32>>("map");
    {
        map<string, i32> map = { { "a", 0 }, { "b", 1 } };
        global["wrap"] = &map;
        i32 x = run_script<i32>("return wrap.b");
        REQUIRE(x == 1);
    }
    {
        map<string, i32> map = { { "a", 0 }, { "b", 1 } };
        global["wrap"] = &map;
        auto res = run_script("wrap.b = 100");
        REQUIRE(map["b"] == 100);
        res = run_script("wrap.c = 42");
        REQUIRE(map["c"] == 42);
    }
    {
        map<string, i32> map = { { "a", 0 }, { "b", 1 } };
        global["wrap"] = &map;
        auto res = run_script("b = wrap.b");
        REQUIRE((i32)global["b"] == map["b"]);
    }

    create_wrapper<unordered_map<string, i32>>("unmap");
    {
        unordered_map<string, i32> map = { { "a", 0 }, { "b", 1 } };
        global["wrap"] = &map;
        i32 x = run_script<i32>("return wrap.b");
        REQUIRE(x == 1);
    }
    {
        unordered_map<string, i32> map = { { "a", 0 }, { "b", 1 } };
        global["wrap"] = &map;
        auto res = run_script("wrap.b = 100");
        REQUIRE(map["b"] == 100);
        res = run_script("wrap.c = 42");
        REQUIRE(map["c"] == 42);
    }
    {
        unordered_map<string, i32> map1 = { { "a", 0 }, { "b", 1 } };
        global["wrap1"] = &map1;
        unordered_map<string, i32> map2 = { { "b", 1 }, { "a", 0 } };
        global["wrap2"] = &map2;
        unordered_map<string, i32> map3 = { { "a", 1 }, { "b", 0 } };
        global["wrap3"] = &map3;
        bool res = run_script<bool>("return wrap1 == wrap2");
        REQUIRE(res);
        res = run_script<bool>("return wrap3 == wrap2");
        REQUIRE_FALSE(res);
    }
}

TEST_CASE_METHOD(LuaWrapperTests, "Script.Wrapper.VectorWrapper")
{
    create_wrapper<vector<i32>>("vec");
    {
        vector<i32> vec = { 0, 1, 2, 3, 4, 5 };
        global["wrap"] = &vec;
        i32 x = run_script<i32>("return wrap[2]");
        REQUIRE(x == vec[1]);
        auto res = run_script("wrap[4] = 100");
        REQUIRE(100 == vec[3]);
        res = run_script("wrap[7] = 100");
        REQUIRE(100 == vec[6]);
    }
    {
        vector<i32> vec = { 0, 1, 2, 3, 4, 5 };
        global["wrap"] = &vec;
        i32 x = run_script<i32>("return #wrap");
        REQUIRE(x == vec.size());
        auto res = run_script("wrap[#wrap + 1] = 6");
        REQUIRE(res.State == LuaResultState::Ok);
        REQUIRE(6 == vec[6]);
    }
    {
        vector<i32> vec = { 0, 1, 2, 3, 4, 5 };
        global["wrap"] = &vec;
        i32 x = run_script<i32>(
            "result = 0 "
            "for i, v in ipairs(wrap) do "
            "result = result + v "
            "end "
            "return result");
        REQUIRE(x == 15);
    }
}

TEST_CASE_METHOD(LuaWrapperTests, "Script.Wrapper.Wrapper")
{
    TestScriptClass t;
    global["earlywrap"] = &t;

    auto& wrapper = create_wrapper<TestScriptClass>("TSC");
    wrapper.function("foo", &TestScriptClass::foo);
    wrapper.function("add", &TestScriptClass::add_value);
    wrapper.function("bar", &TestScriptClass::bar);
    wrapper.function("me", []() -> i32 { return 40; });
    wrapper.property("age", &TestScriptClass::get_value, &TestScriptClass::set_value);
    wrapper.getter("ro_age", &TestScriptClass::get_value);
    wrapper.setter("wo_age", &TestScriptClass::set_value);
    wrapper.getter("map", &TestScriptClass::get_map);
    wrapper.function("ptr", &TestScriptClass::ptr);

    auto f1 = overload<i32, f32>(&TestScriptClass::overload);
    auto f2 = overload<f32, i32>(&TestScriptClass::overload);
    auto f3 = overload<const vector<f32>&>(&TestScriptClass::overload);
    auto f4 = overload<i32, const pair<f32, string>&, f32>(&TestScriptClass::overload);
    auto f5 = overload<const tuple<f32, i32, string>&>(&TestScriptClass::overload);
    auto f6 = []() -> f32 { return 40.0f; };
    wrapper.function("overload", f1, f2, f3, f4, f5, f6);

    wrapper.constructor<>();
    wrapper.constructor<i32>();
    wrapper.constructor<i32, f32>();
    SECTION("early wrap")
    {
        i32 x = run_script<i32>("return earlywrap:foo('test', 2, true)");
        REQUIRE(x == 2 * 4);
    }
    SECTION("index acces")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(100);
        i32 i = run_script<i32>("return wrap1[1]");
        REQUIRE(i == 100);
        auto res = run_script("wrap1[1] = 400");
        REQUIRE(res.State == LuaResultState::Ok);
        REQUIRE(t1.get_value() == 400);
    }
    SECTION("pointer parameter")
    {
        global["test"]["WrapperObj"] = testFuncWrapperObj;
        TestScriptClass t1;
        global["wrap"] = &t1;
        t1.set_value(100);
        i32 i = run_script<i32>("return test.WrapperObj(wrap)");
        REQUIRE(i == 100);
    }
    SECTION("constructor")
    {
        TestScriptClass* t = run_script<TestScriptClass*>("return TSC.new(20)");
        REQUIRE(t->get_value() == 20);
        t = run_script<TestScriptClass*>("return TSC.new(20, 3.5)");
        REQUIRE(t->get_value() == 20 * (i32)3.5f);
        t = run_script<TestScriptClass*>("return TSC.new()");
        REQUIRE(t->get_value() == 0);
    }
    SECTION("pointer from lua")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        TestScriptClass* tp = global["wrap"];
        REQUIRE(tp == &t);
    }
    SECTION("properties")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        t.set_value(42);
        i32 age = run_script<i32>("return wrap.ro_age");
        REQUIRE(age == 42);

        auto res = run_script("wrap.wo_age = 21");
        REQUIRE(res.State == LuaResultState::Ok);
        REQUIRE(t.get_value() == 21);
    }
    SECTION("overloads")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        f32 x = run_script<f32>("return wrap:overload({0.2,0.4})");
        REQUIRE(x == t.overload(vector<f32> { 0.2f, 0.4f }));

        x = run_script<f32>("return wrap:overload(4, 2.0)");
        REQUIRE(x == t.overload(4, 2.0f));

        x = run_script<f32>("return wrap:overload(2.0, 12)");
        REQUIRE(x == t.overload(2.0f, 12));

        x = run_script<f32>("return wrap:overload(15, 2.0, 'huhu', 99.9)");
        REQUIRE(x == t.overload(15, { 2.0f, "huhu" }, 99.9f));

        x = run_script<f32>("return wrap:overload(2.0, 15, 'huhu')");
        REQUIRE(x == t.overload({ 2.0f, 15, "huhu" }));

        x = run_script<f32>("return wrap:overload()");
        REQUIRE(x == 40);
    }
    SECTION("functions and properties")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        i32 x = run_script<i32>("return wrap:foo('test', 4, true)");
        auto res = run_script("wrap:bar(true, 'test', 4)");
        REQUIRE(x == 4 * 4);
        res = run_script("wrap.age = 25");
        res = run_script("age = wrap.age");
        i32 age = global["age"];
        REQUIRE(age == 25);
        REQUIRE(t.get_value() == 25);
        REQUIRE(run_script<i32>("return wrap:me()") == 40);
    }
    SECTION("more properties")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        t.set_value(350);
        auto res = run_script(
            "function foo(x) "
            "return x.age "
            "end ");
        REQUIRE(res.State == LuaResultState::Ok);

        LuaFunction<i32> func = global["foo"];
        i32 x = func(&t);
        REQUIRE(x == 350);
    }
    SECTION("even more properties")
    {
        TestScriptClass t1;
        global["wrap"] = &t1;

        t1.set_value(100);
        i32 x = run_script<i32>("return wrap.age");
        REQUIRE(x == 100);
        x = run_script<i32>("return wrap:add(20)");
        REQUIRE(x == 120);

        TestScriptClass t2;
        t2.set_value(250);

        global["wrap"] = &t2;
        x = run_script<i32>("return wrap.age");
        REQUIRE(x == 250);

        global["wrap"] = &t1;
        x = run_script<i32>("return wrap:add(20)");
        REQUIRE(x == 120);
    }
    SECTION("wrapped member")
    {
        create_wrapper<map<string, i32>>("map");
        TestScriptClass t;
        global["wrap"] = &t;
        map<string, i32>& map = *t.get_map();
        map["x"] = 100;
        auto res = run_script(
            "wrap.map.x = 300 ");
        REQUIRE(res.State == LuaResultState::Ok);
        map = *t.get_map();
        REQUIRE(map["x"] == 300);
    }
    SECTION("pointer member function")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        TestScriptClass t2;
        t2.set_value(123);
        global["wrap2"] = &t2;

        REQUIRE(t1.get_value() != t2.get_value());
        auto result = run_script("wrap1:ptr(wrap2)");
        REQUIRE(result.State == LuaResultState::Ok);
        REQUIRE(t1.get_value() == t2.get_value());
    }
}

TEST_CASE_METHOD(LuaWrapperTests, "Script.Wrapper.Metamethods")
{
    auto& wrapper = create_wrapper<TestScriptClass>("TSCB");
    wrapper.metamethod(LuaMetamethod::Add,
        [](TestScriptClass* instance1, i32 x) {
            i32 age = instance1->get_value() + x;
            return LuaOwnedPtr(new TestScriptClass(age));
        });
    wrapper.metamethod(LuaMetamethod::Subtract,
        [](TestScriptClass* instance1, i32 x) {
            i32 age = instance1->get_value() - x;
            return LuaOwnedPtr(new TestScriptClass(age));
        });
    wrapper.metamethod(LuaMetamethod::Divide,
        [](TestScriptClass* instance1, i32 x) {
            i32 age = instance1->get_value() / x;
            return LuaOwnedPtr(new TestScriptClass(age));
        });
    wrapper.metamethod(LuaMetamethod::Multiply,
        [](TestScriptClass* instance1, i32 x) {
            i32 age = instance1->get_value() * x;
            return LuaOwnedPtr(new TestScriptClass(age));
        });
    wrapper.metamethod(LuaMetamethod::LessThan,
        [](TestScriptClass* instance1, i32 x) {
            return instance1->get_value() < x;
        });
    wrapper.metamethod(LuaMetamethod::LessThan,
        [](i32 x, TestScriptClass* instance1) {
            return instance1->get_value() > x;
        });
    wrapper.metamethod(LuaMetamethod::LessOrEqualThan,
        [](TestScriptClass* instance1, i32 x) {
            return instance1->get_value() <= x;
        });
    wrapper.metamethod(LuaMetamethod::LessOrEqualThan,
        [](i32 x, TestScriptClass* instance1) {
            return instance1->get_value() >= x;
        });
    wrapper.metamethod(LuaMetamethod::UnaryMinus,
        [](TestScriptClass* instance1) {
            return -instance1->get_value();
        });
    wrapper.metamethod(LuaMetamethod::Length,
        [](TestScriptClass* instance1) {
            return instance1->get_value();
        });
    wrapper.metamethod(LuaMetamethod::ToString,
        [](TestScriptClass* instance1) {
            return std::to_string(instance1->get_value());
        });
    wrapper.metamethod(LuaMetamethod::Concat,
        [](TestScriptClass* instance1, i32 x) {
            return std::stoi(std::to_string(instance1->get_value()) + std::to_string(x));
        });
    wrapper.metamethod(LuaMetamethod::Call,
        [](TestScriptClass* instance1, i32 x) {
            return x * instance1->get_value();
        });
    SECTION("Call")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 b = run_script<i32>("return wrap1(100)");
        REQUIRE(b == age1 * 100);
    }
    SECTION("Length")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 b = run_script<i32>("return #wrap1");
        REQUIRE(b == age1);
    }
    SECTION("ToString")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        string b = run_script<string>("return tostring(wrap1)");
        REQUIRE(b == to_string(t1.get_value()));
    }
    SECTION("Concat")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 b = run_script<i32>("return wrap1 .. 10");
        REQUIRE(b == 400010);
    }
    SECTION("LessOrEqualThan")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 age2 = 4000;
        global["age2"] = age2;

        bool b = run_script<bool>("return wrap1 <= age2");
        REQUIRE(b);
        b = run_script<bool>("return wrap1 >= age2");
        REQUIRE(b);

        t1.set_value(200);
        b = run_script<bool>("return wrap1 <= age2");
        REQUIRE(b);
        b = run_script<bool>("return wrap1 >= age2");
        REQUIRE_FALSE(b);
    }
    SECTION("LessThan")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 age2 = 8000;
        global["age2"] = age2;

        bool b = run_script<bool>("return wrap1 < age2");
        REQUIRE(b);
        b = run_script<bool>("return wrap1 > age2");
        REQUIRE_FALSE(b);

        t1.set_value(16000);

        b = run_script<bool>("return wrap1 < age2");
        REQUIRE_FALSE(b);
        b = run_script<bool>("return wrap1 > age2");
        REQUIRE(b);
    }
    SECTION("autogenerated equal")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 age2 = 4000;
        TestScriptClass t2;
        global["wrap2"] = &t2;
        t2.set_value(age2);

        bool b = run_script<bool>("return wrap1 == wrap2");
        REQUIRE(b);
        b = run_script<bool>("return wrap1 ~= wrap2");
        REQUIRE_FALSE(b);

        t1.set_value(8000);

        b = run_script<bool>("return wrap1 == wrap2");
        REQUIRE_FALSE(b);
        b = run_script<bool>("return wrap1 ~= wrap2");
        REQUIRE(b);
    }
    SECTION("Add,Subtract,Divide,Multiply")
    {
        i32 age1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(age1);

        i32 age2 = 200;
        global["age2"] = age2;

        TestScriptClass* b = run_script<TestScriptClass*>("return wrap1 + age2");
        REQUIRE(b->get_value() == age1 + age2);

        b = run_script<TestScriptClass*>("return wrap1 - age2");
        REQUIRE(b->get_value() == age1 - age2);

        b = run_script<TestScriptClass*>("return wrap1 / age2");
        REQUIRE(b->get_value() == age1 / age2);

        b = run_script<TestScriptClass*>("return wrap1 * age2");
        REQUIRE(b->get_value() == age1 * age2);
    }
    SECTION("UnaryMinus")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(100);

        i32 b = run_script<i32>("return -wrap1");
        REQUIRE(b == -100);
    }
    SECTION("GC")
    {
        perform_GC();
        REQUIRE(TestScriptClass::ObjCount == 0);
    }
}

TEST_CASE_METHOD(LuaWrapperTests, "Script.Wrapper.FunctionReturn")
{
    {
        LuaClosureSharedPtr l;

        auto foo = [&l](TestScriptClass* instance1, i32 x) mutable {
            instance1->set_value(x * 10);
            auto lambda = [instance1](i32 y) {
                return instance1->get_value() + y;
            };

            l = make_shared_luaclosure(std::function(lambda));
            return l.get();
        };

        auto& wrapper = create_wrapper<TestScriptClass>("TSCB");
        wrapper.function("foo", foo);
        {
            TestScriptClass t;
            global["wrap"] = &t;
            auto ret = run_script<i32>("return wrap:foo(4)(2)");
            REQUIRE(ret.State == LuaResultState::Ok);
            REQUIRE(ret.Value == 42);
        }
    }
    {
        std::string text {};

        LuaClosureSharedPtr l;
        auto text_adder = [&l, &text](std::string& y) {
            text += y;
            return l.get();
        };

        l = make_shared_luaclosure(std::function(text_adder));

        auto text_setter = [&l, &text](std::string& x) mutable {
            text = x;
            return l.get();
        };

        auto& wrapper = create_wrapper<TestScriptClass>("TSCB");
        wrapper.function("foo", text_setter);
        {
            TestScriptClass t;
            global["wrap"] = &t;
            auto ret = run_script("wrap.foo 'hello' ' ' 'world' '!'");
            REQUIRE(ret.State == LuaResultState::Ok);
            REQUIRE(text == "hello world!");
        }
    }
}
