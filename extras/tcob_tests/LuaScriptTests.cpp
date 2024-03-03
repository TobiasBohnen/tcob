#include "WrapperTestsClass.hpp"

#include <cstring>
#include <string>

#include "tests.hpp"

using namespace tcob::scripting;
using namespace tcob::scripting::lua;

auto static testfuncstr() -> std::string
{
    return "huhu";
}

auto static testfuncfloat() -> float
{
    return 4.2f;
}
auto static testfuncpair(std::pair<i32, f32> const& p) -> f32
{
    return static_cast<f32>(p.first) * p.second;
}
auto static testfuncfloat2(tcob::scripting::result<f32> f, tcob::scripting::result<f32> x, int i) -> f32
{
    return f.value() * x.value() * static_cast<f32>(i);
}

struct foo {
    int x = 0;
    int y = 0;
    int z = 0;
};

namespace tcob::scripting::lua {
template <>
struct converter<foo> {
    auto static IsType(state_view ls, i32 idx) -> bool
    {
        table lt {table::Acquire(ls, idx)};
        return lt.has("x") && lt.has("y") && lt.has("z");
    }

    auto static From(state_view ls, i32& idx, foo& value) -> bool
    {
        if (ls.is_table(idx)) {
            table lt {table::Acquire(ls, idx++)};

            value.x = lt["x"];
            value.y = lt["y"];
            value.z = lt["z"];
        }
        return true;
    }

    void static To(state_view ls, foo const& value)
    {
        ls.new_table();
        table lt {table::Acquire(ls, -1)};

        lt["x"] = value.x;
        lt["y"] = value.y;
        lt["z"] = value.z;
    }
};
}

class LuaScriptTests : public lua::script {
public:
    LuaScriptTests()
        : global(get_global_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Closures")
{
    SUBCASE("misc")
    {
        std::function testFuncPrim = [](i32 i, f32 f, double d, bool b) {
            return std::to_string(i) + std::to_string(f) + std::to_string(d) + std::string(b ? "true" : "false");
        };

        i32           voidTest     = 0;
        std::function testFuncVoid = [&voidTest] { voidTest++; };

        global["test"]["Prim"] = &testFuncPrim;
        global["test"]["Void"] = &testFuncVoid;

        {
            auto res = run("str = test.Prim(20, 4.4, 5.22, true)");
            REQUIRE(res);
            std::string str = global["str"];
            REQUIRE(str == testFuncPrim(20, 4.4f, 5.22, true));
        }
        {
            auto res = run("test.Void()");
            REQUIRE(res);
            REQUIRE(voidTest == 1);
            res = run("test.Void()");
            REQUIRE(res);
            REQUIRE(voidTest == 2);
        }
        {
            global["testFunc"] = testfuncstr;
            std::string x      = *run<std::string>("return testFunc()");
            REQUIRE(x == testfuncstr());
        }
        {
            global["testFunc"] = testfuncfloat;
            f32 x              = *run<f32>("return testFunc()");
            REQUIRE(x == testfuncfloat());
        }
        {
            global["testFunc"] = testfuncfloat2;
            f32 x              = *run<f32>("return testFunc(4,4.5,3)");
            REQUIRE(x == testfuncfloat2(tcob::scripting::result<f32> {4.f}, tcob::scripting::result<f32> {4.5f}, 3));
        }
        {
            global["testFunc"] = testfuncpair;
            f32 x              = *run<f32>("return testFunc(4, 6.5)");
            REQUIRE(x == testfuncpair({4, 6.5f}));
        }
        {
            auto l             = std::function([](i32 i) { return (f32)i * 2.5f; });
            global["testFunc"] = &l;
            f32 x              = *run<f32>("return testFunc(2)");
            REQUIRE(x == 5.0f);
        }
        {
            f32  x {0};
            auto l             = std::function([&x](i32 i) { x = static_cast<f32>(i) * 2.5f; });
            global["testFunc"] = &l;
            auto res           = run("testFunc(2)");
            REQUIRE(res);
            REQUIRE(x == 5.0f);
        }
    }
    SUBCASE("return table")
    {
        i32  x {102};
        auto l             = std::function([&](i32 i) {
            table lt {get_view()};
            lt["value"] = x * i;
            return lt;
        });
        global["testFunc"] = &l;
        auto res           = run<table>("return testFunc(2)");
        REQUIRE(res);
        REQUIRE((*res)["value"].as<i32>() == 204);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Container")
{
    std::function testFuncVector = []() {
        return std::vector<std::string> {"1", "2", "3", "4", "5"};
    };

    std::function testFuncArray = []() {
        return std::array<std::string, 5> {"1", "2", "3", "4", "5"};
    };

    std::function testFuncPairPara = [](std::pair<std::string, i32> const& pair) {
        return pair.second;
    };

    std::function testFuncTuple = [](double d) {
        return std::tuple(d * 5, std::to_string(d));
    };

    std::function testFuncMap = []() {
        return std::map<std::string, i32> {
            {"abc", 123},
            {"def", 234}};
    };
    std::function testFuncUMap = []() {
        return std::unordered_map<std::string, i32> {
            {"abc", 123}, {"def", 234}};
    };

    global["test"]["Tuple"]    = &testFuncTuple;
    global["test"]["Map"]      = &testFuncMap;
    global["test"]["UMap"]     = &testFuncUMap;
    global["test"]["Vector"]   = &testFuncVector;
    global["test"]["Array"]    = &testFuncArray;
    global["test"]["PairPara"] = &testFuncPairPara;

    SUBCASE("tuple return from cpp")
    {
        auto res = run("a, b = test.Tuple(5.22)");
        REQUIRE(res);
        double      a = global["a"];
        std::string b = global["b"];
        REQUIRE(a == 5.22 * 5);
        REQUIRE(b == std::to_string(5.22));
    }
    SUBCASE("map return from cpp")
    {
        auto res = run("x = test.Map()");
        REQUIRE(res);
        std::map<std::string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("unordered_map return from cpp")
    {
        auto res = run("x = test.UMap()");
        REQUIRE(res);
        std::unordered_map<std::string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("vector return from cpp")
    {
        auto res = run("x = test.Vector()");
        REQUIRE(res);
        std::vector<std::string> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("array return from cpp")
    {
        auto res = run("x = test.Array()");
        REQUIRE(res);
        std::array<std::string, 5> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("vector parameter")
    {
        auto res = run(
            "function foo(x) "
            "   return x[2] * x[4] "
            "end ");
        REQUIRE(res);
        std::vector   vec {1, 2, 3, 4, 5};
        function<i32> func = global["foo"];
        i32           a    = func(vec);
        REQUIRE(a == 2 * 4);

        std::array arr {1, 2, 3, 4, 5};
        a = func(arr);
        REQUIRE(a == 2 * 4);
    }
    SUBCASE("tuple parameter")
    {
        auto res = run(
            "function foo(x, y, z) "
            "   if z then return x * y else return 10 end "
            "end ");
        REQUIRE(res);
        auto          tup  = std::make_tuple(4, 2, true);
        function<i32> func = global["foo"];
        i32           a    = func(tup);
        REQUIRE(a == 4 * 2);
    }
    SUBCASE("tuple of tuple parameter")
    {
        auto res = run(
            "function foo(x, y, z) "
            "   if z then return x * y else return 10 end "
            "end ");
        REQUIRE(res);
        auto          tup  = std::make_tuple(std::make_tuple(4, 2), true);
        function<i32> func = global["foo"];
        i32           a    = func(tup);
        REQUIRE(a == 4 * 2);
    }
    SUBCASE("pair parameter")
    {
        auto res = run(
            "function foo(x, y) "
            "   return x * y "
            "end ");
        REQUIRE(res);
        auto          tup  = std::make_pair(4, 2.4f);
        function<f32> func = global["foo"];
        f32           a    = func(tup);
        REQUIRE(a == 4 * 2.4f);
    }
    SUBCASE("map parameter")
    {
        auto res = run(
            "function foo(x) "
            "   return x.test "
            "end ");
        REQUIRE(res);
        std::map<std::string, i32> map  = {{"test", 123}};
        function<i32>              func = global["foo"];
        i32                        a    = func(map);
        REQUIRE(a == 123);

        std::unordered_map<std::string, i32> umap = {{"test", 245}};
        a                                         = func(umap);
        REQUIRE(a == 245);
    }
    SUBCASE("get/set vector")
    {
        std::vector<std::string> vec = {"test", "123"};
        global["foo"]                = vec;
        std::string a                = *run<std::string>("return foo[1] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[2] ");
        REQUIRE(b == "123");
    }
    SUBCASE("get/set deque")
    {
        std::deque<std::string> deck = {"test", "123"};
        global["foo"]                = deck;
        std::string a                = *run<std::string>("return foo[1] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[2] ");
        REQUIRE(b == "123");
    }
    SUBCASE("get/set span")
    {
        std::vector<std::string> vec {"test", "123"};
        std::span<std::string>   s {vec.data(), vec.size()};
        global["foo"] = s;
        std::string a = *run<std::string>("return foo[1] ");
        REQUIRE(a == vec[0]);
        std::string b = *run<std::string>("return foo[2] ");
        REQUIRE(b == vec[1]);
    }
    SUBCASE("get map")
    {
        auto res = run("rectF = {x=2.7, y=3.1, width=2.3, height=55.2} ");
        REQUIRE(res);
        std::map<std::string, f32> rectF = global["rectF"];
        REQUIRE(rectF["x"] == 2.7f);
    }
    SUBCASE("get/set map")
    {
        std::map<std::string, i32> map = {{"test", 123}};
        global["foo"]                  = map;
        i32 a                          = *run<i32>("return foo.test ");
        REQUIRE(a == 123);
    }
    SUBCASE("get multiple return values as pair")
    {
        std::pair<std::string, i32> x = *run<std::pair<std::string, i32>>("return 'ok', 10");
        REQUIRE(x.first == "ok");
        REQUIRE(x.second == 10);
    }
    SUBCASE("get multiple return values as tuple")
    {
        auto const [s, i, b] = run<std::tuple<std::string, i32, bool>>("return 'ok', 10, true").value();
        REQUIRE(s == "ok");
        REQUIRE(i == 10);
        REQUIRE(b == true);
    }
    SUBCASE("pair parameter")
    {
        function<i32> func = global["test"]["PairPara"];
        i32           a    = func(std::pair {"ok"s, 4});
        REQUIRE(a == 4);
    }
    SUBCASE("get/set set")
    {
        std::set<std::string> set1 {"test", "test2"};
        global["foo"]              = set1;
        std::set<std::string> set2 = *run<std::set<std::string>>("return foo ");
        REQUIRE(set1 == set2);
    }
    SUBCASE("set return")
    {
        std::set<i32> set = *run<std::set<i32>>("return { 1, 2, 3, 1, 2, 3, 4, 2 } ");
        REQUIRE(set == std::set<i32> {1, 2, 3, 4});
    }
    SUBCASE("get/set unordered_set")
    {
        std::unordered_set<std::string> set1 {"test", "test2"};
        global["foo"]                        = set1;
        std::unordered_set<std::string> set2 = *run<std::unordered_set<std::string>>("return foo ");
        REQUIRE(set1 == set2);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Coroutines")
{
    SUBCASE("basic")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,10 do "
            "         coroutine.yield(i) "
            "       end "
            "     end) ");
        REQUIRE(res);
        REQUIRE(global.is<coroutine>("co"));
        coroutine co = global["co"];
        REQUIRE(co.resume<i32>().value() == 1);
        REQUIRE(co.resume<i32>().value() == 2);
        REQUIRE(co.resume<i32>().value() == 3);
    }
    SUBCASE("with parameter")
    {
        auto res = run(
            "co = coroutine.create(function (x) "
            "       for i=1,10 do "
            "         coroutine.yield(i*x) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];
        REQUIRE(co.resume<i32>(2).value() == 2);
        REQUIRE(co.resume<i32>().value() == 4);
        REQUIRE(co.resume<i32>().value() == 6);
    }
    SUBCASE("check dead")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i) "
            "       end "
            "       return 1000 "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);
        result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
        result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Dead);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1000);
    }
    SUBCASE("error on resume dead")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);
        result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
        auto endresult = co.resume<void>();
        REQUIRE(co.get_status() == coroutine_status::Dead);
        REQUIRE_FALSE(endresult.has_error());
        auto endresult2 = co.resume<void>();
        REQUIRE(endresult2.has_error());
        REQUIRE(endresult2.error() == error_code::Error);
    }
    SUBCASE("status")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);
        result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
        result = co.resume<i32>();
        REQUIRE_FALSE(result.has_value());
        REQUIRE(co.get_status() == coroutine_status::Dead);
    }
    SUBCASE("return multiple values")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i,i+0.5) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<std::pair<i32, f32>>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == std::make_pair(1, 1.5f));
    }
    SUBCASE("mismatched return")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<std::pair<i32, f32>>();
        REQUIRE(result.error() == error_code::TypeMismatch);
    }
    SUBCASE("close")
    {
        auto res = run(
            "co = coroutine.create(function () "
            "       for i=1,2 do "
            "         coroutine.yield(i) "
            "       end "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);

        auto coresult = co.close();
        REQUIRE(coresult == coroutine_status::Dead);

        result = co.resume<i32>();
        REQUIRE_FALSE(result.has_value());
    }
    SUBCASE("push closure to coroutine")
    {
        auto l = std::function([](i32 i) { return (f32)i * 2.5f; });

        auto res = run(
            "co = coroutine.create(function () "
            "         coroutine.yield(100) "
            "     end) ");
        REQUIRE(res);

        coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 100);
        static_cast<void>(co.resume<void>());
        REQUIRE(co.get_status() == coroutine_status::Dead);

        co.push(&l);
        auto result2 = co.resume<f32>(15);
        REQUIRE(result2.has_value());
        REQUIRE(result2.value() == l(15));
    }
    SUBCASE("create from function")
    {
        auto res = run(
            "function co_create() "
            "     return coroutine.create( "
            "         function () "
            "             Global = 100 "
            "             coroutine.yield() "
            "             Global = 300 "
            "             coroutine.yield() "
            "             Global = 400 "
            "             coroutine.yield() "
            "         end) "
            "end");
        REQUIRE(res);

        function<coroutine> func = global["co_create"];
        coroutine           co   = func();
        REQUIRE(co.get_status() == coroutine_status::Ok);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 100);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 300);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 400);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Dead);
        REQUIRE((i32)global["Global"] == 400);

        co = func();
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 100);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 300);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Suspended);
        REQUIRE((i32)global["Global"] == 400);
        REQUIRE(co.resume());
        REQUIRE(co.get_status() == coroutine_status::Dead);
        REQUIRE((i32)global["Global"] == 400);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Enums")
{
    enum class testEnum {
        True,
        False,
        FileNotFound
    };
    std::function testFuncEnum = [](testEnum numnum) {
        return numnum;
    };

    global["test"]["Enum"] = &testFuncEnum;
    {
        function<testEnum> func = global["test"]["Enum"];
        testEnum           num  = *func.call(testEnum::FileNotFound);
        REQUIRE(num == testEnum::FileNotFound);
    }
    {
        std::unordered_map<std::string, testEnum> map = {
            {"True", testEnum::True},
            {"False", testEnum::False},
            {"FileNotFound", testEnum::FileNotFound},
        };

        global["testEnum"] = map;

        testEnum num = *run<testEnum>(
            "enum = testEnum.False "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::False);
        num = *run<testEnum>(
            "enum = testEnum.True "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::True);
        num = *run<testEnum>(
            "enum = testEnum.FileNotFound "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::FileNotFound);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Functions")
{
    SUBCASE("cpp -> lua -> cpp")
    {
        auto res = run(
            "function foo0(a) return cppFunc0(a) end "
            "function foo1(b) return 3 * b end ");
        REQUIRE(res);
        function<i32> foo0 = global["foo0"];
        function<i32> foo1 = global["foo1"];

        auto cppFunc0      = std::function([&](i32 i) { return foo1(20 * i); });
        global["cppFunc0"] = &cppFunc0;

        i32 result = foo0(10);
        REQUIRE(result == 600);
    }
    SUBCASE("lua -> cpp -> lua")
    {
        REQUIRE(run("function foo1(b) return 3 * b end "));
        function<i32> foo1     = global["foo1"];
        auto          cppFunc0 = std::function([&](i32 i) { return foo1(20 * i); });
        global["cppFunc0"]     = &cppFunc0;
        auto res               = run<i32>("return cppFunc0(10)");
        REQUIRE(res);
        REQUIRE(res.value() == 600);
    }
    SUBCASE("point_f parameter")
    {
        auto res = run("function testPoint(p) return p.x * p.y end");
        REQUIRE(res);
        function<i32> func = global["testPoint"];
        i32           a    = *func.call(point_i {2, 4});
        REQUIRE(a == 2 * 4);
        a = func(point_i {2, 4});
        REQUIRE(a == 2 * 4);
    }
    SUBCASE("i32 parameter")
    {
        function<i32> func = *run<function<i32>>("return function(x) return x*x end ");
        i32           a    = func(200);
        REQUIRE(a == 200 * 200);
    }
    SUBCASE("optional parameter")
    {
        {
            auto f = *run<function<bool>>("return function(i) return i == nil end");
            REQUIRE(f(std::nullopt) == true);
            std::optional<i32> opt;
            REQUIRE(f(opt) == true);
            opt = 10;
            REQUIRE(f(opt) == false);
        }
        {
            auto               f = *run<function<i32>>("return function(a,b,c) return b == nil and a + c or a + b + c end");
            std::optional<i32> opt;
            REQUIRE(f(1, opt, 5) == 6);
            opt = 10;
            REQUIRE(f(1, opt, 5) == 16);
        }
    }
    SUBCASE("multiple return values as pair")
    {
        auto res = run(
            "table = { } "
            "table.func = function() return 50, 'Hello' end ");
        REQUIRE(res);
        function<std::pair<i32, std::string>> func = global["table"]["func"];
        auto const [a, b]                          = func.call().value();

        REQUIRE(a == 50);
        REQUIRE(b == "Hello");
    }
    SUBCASE("multiple return values as tuple")
    {
        {
            auto res = run(
                "table = { } "
                "table.func = function() return 'Hello', 100, true end ");
            REQUIRE(res);
            function<std::tuple<std::string, i32, bool>> func = global["table"]["func"];
            auto const [a, b, c]                              = func.call().value();

            REQUIRE(a == "Hello");
            REQUIRE(b == 100);
            REQUIRE(c == true);
        }
        {
            auto res = run(
                "table = { } "
                "table.func = function() return 100, { a = 200, b = 300 }, false end ");
            REQUIRE(res);
            function<std::tuple<i32, std::map<std::string, int>, bool>> func = global["table"]["func"];
            auto [a, b, c]                                                   = func.call().value();

            REQUIRE(a == 100);
            REQUIRE(b["a"] == 200);
            REQUIRE(b["b"] == 300);
            REQUIRE(c == false);
        }
    }
    SUBCASE("load_binary from file")
    {
        auto res = run(
            "function testTable(x,y) "
            "   return x*y "
            "end ");
        REQUIRE(res);
        function<i32> func = global["testTable"];
        REQUIRE(func(10, 20) == 10 * 20);
        REQUIRE(func(20, 40) == 20 * 40);
        {
            io::ofstream fs {"test.luac"};
            func.dump(fs);
        }

        function<i32> func2 = load_binary<i32>("test.luac");
        REQUIRE(func2(10, 20) == 10 * 20);
        REQUIRE(func2(20, 40) == 20 * 40);
    }
    SUBCASE("load_binary from stream")
    {
        io::iomstream stream {};

        auto res = run(
            "function testTable(x,y) "
            "   return x*y "
            "end ");
        REQUIRE(res);
        function<i32> func = global["testTable"];
        REQUIRE(func(10, 20) == 10 * 20);
        REQUIRE(func(20, 40) == 20 * 40);
        func.dump(stream);
        stream.seek(0, io::seek_dir::Begin);

        function<i32> func2 = load_binary<i32>(stream);
        REQUIRE(func2(10, 20) == 10 * 20);
        REQUIRE(func2(20, 40) == 20 * 40);
    }
    SUBCASE("nullptr as parameter")
    {
        auto res = run(
            "function foo(a, b, c) "
            "  if not a then return 1 end "
            "  if not b then return 2 end "
            "  if not c then return 3 end "
            "  return 0 "
            "end ");
        REQUIRE(res);
        function<i32> func = global["foo"];

        i32 a = func(1, 2, 3);
        REQUIRE(a == 0);
        a = func(nullptr, 2, 3);
        REQUIRE(a == 1);
        a = func(1, nullptr, 3);
        REQUIRE(a == 2);
        a = func(1, 2, nullptr);
        REQUIRE(a == 3);
    }
    SUBCASE("parameter_pack")
    {
        {
            auto res = run(
                "function foo(a, b, c) "
                "  return a+b+c "
                "end ");
            REQUIRE(res);
            function<i32> func = global["foo"];

            parameter_pack<i32> pack;
            pack.Items = {1, 2, 3};
            i32 a      = func(pack);
            REQUIRE(a == 6);
        }
        {
            auto res = run(
                "function foo(a, b, c) "
                "  return b == true and a+c or a*c "
                "end ");
            REQUIRE(res);
            function<i32> func = global["foo"];

            parameter_pack<std::variant<i32, bool>> pack;
            pack.Items = {2, true, 3};
            i32 a      = func(pack);
            REQUIRE(a == 5);
            pack.Items = {2, false, 4};
            a          = func(pack);
            REQUIRE(a == 8);
        }
    }
    SUBCASE("call string library function")
    {
        function<std::string> func  = global["string"]["upper"];
        std::string           upper = func("hello");
        REQUIRE(upper == "HELLO");
    }
    SUBCASE("misc")
    {
        {
            auto func = run<function<i32>>("return function() return 100 end ").value();
            REQUIRE(func() == 100);
            REQUIRE(100 == func());
        }
        {
            function<i32> func = *run<function<i32>>("return function() return 100 end ");
            REQUIRE_FALSE(func() == 10);
            REQUIRE_FALSE(10 == func());
        }
        {
            function<i32> func = *run<function<i32>>("return function() return 5 end ");
            REQUIRE(func() * 20 == 100);
            REQUIRE(20 * func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return function() return 500 end ");
            REQUIRE(func() / 5 == 100);
            REQUIRE(50000 / func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return function() return 95 end ");
            REQUIRE(func() + 5 == 100);
            REQUIRE(5 + func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return function() return 105 end ");
            REQUIRE(func() - 5 == 100);
            REQUIRE(205 - func() == 100);
        }
        {
            auto func = run<function<std::vector<i32>>>("return function() return {5, 4, 3, 2, 1} end ").value();
            auto a    = func();
            REQUIRE(a[0] == 5);
            REQUIRE(a[1] == 4);
            REQUIRE(a[2] == 3);
            REQUIRE(a[3] == 2);
            REQUIRE(a[4] == 1);
        }
        {
            auto func = run<function<std::map<std::string, i32>>>("return function() return {x=5, y=4, b=3, r=2, aa=1} end ").value();
            auto a    = func();
            REQUIRE(a["x"] == 5);
            REQUIRE(a["y"] == 4);
            REQUIRE(a["b"] == 3);
            REQUIRE(a["r"] == 2);
            REQUIRE(a["aa"] == 1);
        }
        {
            auto res = run(
                "x = 0 "
                "function testVoid(p) "
                "   x = p.x * p.y "
                "end ");
            REQUIRE(res);
            function<void> func = global["testVoid"];
            func(point_i {2, 4});
            i32 x = global["x"];
            REQUIRE(x == 2 * 4);
        }
        {
            auto res = run(
                "function testMulti(f,p,r,b) "
                "   return f * p.x * r.y "
                "end ");
            REQUIRE(res);
            function<f32> func = global["testMulti"];
            f32           x    = func(10.4f, point_i {2, 4}, rect_f {0, 20, 4, 5}, true);
            REQUIRE(x == 10.4f * 2 * 20);
        }
        {
            auto res = run(
                "function testTable(x,y) "
                "   return { a = x, b = y } "
                "end ");
            REQUIRE(res);
            function<table> func = global["testTable"];
            table           tab  = func(10, 20);
            REQUIRE((i32)tab["a"] == 10);
            REQUIRE((i32)tab["b"] == 20);
        }
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.GetSet")
{
    SUBCASE("get/set std::string")
    {
        auto res = run("x = 'ok'");
        REQUIRE(res);
        std::string x = *global.get<std::string>("x");
        REQUIRE(x == "ok");

        global["x"]   = "ko";
        std::string y = global["x"];
        REQUIRE(y == "ko");
    }
    SUBCASE("get/set std::string_view")
    {
        using namespace std::literals;
        auto constexpr x {"ABCDEF"sv};
        global["x"]        = x.substr(2, 2);
        std::string_view y = global["x"];
        REQUIRE(y == "CD");
    }
    SUBCASE("get const char*")
    {
        auto res = run("x = 'ok'");
        REQUIRE(res);
        char const* x = *global.get<char const*>("x");
        REQUIRE(strcmp(x, "ok") == 0);
    }
    SUBCASE("get/set const char*")
    {
        char const* x {"ok"};
        global["x"]   = x;
        char const* y = global["x"];
        REQUIRE(strcmp(x, y) == 0);
    }
    SUBCASE("get/set i32")
    {
        auto res = run("x = 1337");
        REQUIRE(res);
        i32 x = global["x"];
        REQUIRE(x == 1337);
        global["x"] = 2000;
        x           = global["x"];
        REQUIRE(x == 2000);
        x = *run<i32>("return x");
        REQUIRE(x == 2000);
    }
    SUBCASE("delete table entry")
    {
        auto res = run("x = 1337");
        REQUIRE(res);
        REQUIRE(global.has("x"));
        global["x"] = nullptr;
        REQUIRE_FALSE(global.has("x"));
    }
    SUBCASE("get/set u8")
    {
        auto res = run("x = 30 ");
        REQUIRE(res);
        u8 x = global["x"];
        REQUIRE(x == 30);
    }
    SUBCASE("get/set nested i32")
    {
        auto res = run("x = { y = { z = 30 } }");
        REQUIRE(res);
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 30);
        global["x"]["y"]["z"] = 2000;
        x                     = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ subscript")
    {
        auto res = run("x = { y = {  } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ set")
    {
        auto res = run("x = { y = {  } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global.set("x", "y", "z", 2000);
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create global var w/ subscript")
    {
        REQUIRE_FALSE(global.has("testVar1"));
        global["testVar1"] = 2000;
        REQUIRE(global.has("testVar1"));
        i32 x = global["testVar1"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create global var w/ set")
    {
        REQUIRE_FALSE(global.has("testVar2"));
        global.set("testVar2", 2000);
        REQUIRE(global.has("testVar2"));
        i32 x = global["testVar2"];
        REQUIRE(x == 2000);
    }
    SUBCASE("try access undefined global")
    {
        REQUIRE_FALSE(global.has("testVar4"));
        auto f = global.get<f32>("testVar4");
        REQUIRE(f.error() == error_code::Undefined);
    }
    SUBCASE("access nested table")
    {
        auto res = run("x = { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res);
        i32 m = global["x"]["z"]["m"];
        REQUIRE(m == 75);
    }
    SUBCASE("access created global from function")
    {
        REQUIRE_FALSE(global.has("testVar"));
        global["testVar"] = 400;
        auto res          = run("function foo() return testVar * 10 end");
        REQUIRE(res);
        function<i32> func = global["foo"];
        i32           a    = func();
        REQUIRE(a == 400 * 10);
        global["testVar"] = 2000;
        a                 = func();
        REQUIRE(a == 2000 * 10);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Hook")
{
    SUBCASE("count in hook")
    {
        i32 linecount {0};
        i32 instcount {0};

        auto func = [&](debug const& debug) {
            if (debug.Source == "TEST") {
                switch (debug.Event) {
                case debug_event::Line:
                    linecount++;
                    break;
                case debug_event::Count:
                    instcount++;
                    break;
                case debug_event::Call:
                case debug_event::TailCall:
                case debug_event::Return:
                    break;
                }
            }
        };

        set_hook(func);
        auto res = run(
            "x = 100 \n"
            "print(x) \n"
            "y = x * 200 \n"
            "print(y) \n"
            "return 1000",
            "TEST");

        REQUIRE(linecount == 5);
        REQUIRE(instcount == 12);

        auto func2 = [&](debug const& debug) {
            if (debug.Source == "TEST") {
                switch (debug.Event) {
                case debug_event::Line:
                    linecount--;
                    break;
                case debug_event::Count:
                    instcount--;
                    break;
                case debug_event::Call:
                case debug_event::TailCall:
                case debug_event::Return:
                    break;
                }
            }
        };

        set_hook(func2);
        res = run(
            "x = 100 \n"
            "print(x) \n"
            "y = x * 200 \n"
            "print(y) \n"
            "return 1000",
            "TEST");

        REQUIRE(linecount == 0);
        REQUIRE(instcount == 0);

        remove_hook();
        res = run(
            "x = 100 \n"
            "print(x) \n"
            "y = x * 200 \n"
            "print(y) \n"
            "return 1000",
            "TEST");

        REQUIRE(linecount == 0);
        REQUIRE(instcount == 0);
    }
    SUBCASE("raise error in hook")
    {
        i32 linecount {0};

        auto func = [&](debug const& debug) {
            if (debug.Source == "TEST") {
                switch (debug.Event) {
                case debug_event::Line:
                    linecount++;
                    if (linecount == 3) {
                        raise_error("oh no 3!");
                    }
                    break;
                case debug_event::Count:
                case debug_event::Call:
                case debug_event::TailCall:
                case debug_event::Return:
                    break;
                }
            }
        };

        set_hook(func);
        auto res = run(
            "x = 100 \n"
            "print(x) \n"
            "y = x * 200 \n"
            "print(y) \n"
            "return 1000",
            "TEST");

        REQUIRE(!res);
        REQUIRE(linecount == 3);
    }
    SUBCASE("locals in hook")
    {
        i32         x {0};
        i32         y {0};
        std::string z;

        auto func = [&](debug const& debug) {
            if (debug.Source == "TEST" && debug.Event == debug_event::Return) {
                auto const view {get_view()};
                auto const guard {view.create_stack_guard()};

                for (i32 i {1};; ++i) {
                    auto val {debug.get_local(i)};
                    if (val.empty() || val[0] == '(') { break; }

                    if (val == "x") {
                        view.pull_convert_idx(-1, x);
                    } else if (val == "y") {
                        view.pull_convert_idx(-1, y);
                    } else if (val == "z") {
                        view.pull_convert_idx(-1, z);
                    }
                }
            }
        };

        set_hook(func);
        auto res = run(
            "local x = 100 "
            "local y = 400 "
            "local z = 'ok' "
            "print(x,y,z) ",
            "TEST");

        REQUIRE(res);
        REQUIRE(x == 100);
        REQUIRE(y == 400);
        REQUIRE(z == "ok");
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.IsHas")
{
    SUBCASE("is")
    {
        auto res = run(
            "a = 100 "
            "b = true "
            "c = 10.3 "
            "d = 'hello' "
            "e = { } "
            "f = function(x) return x end "
            "g = { a = 1, b = 2, c = 3 } "
            "h = { 1, 2, 3 } ");

        REQUIRE(res);

        REQUIRE(global.is<i32>("a"));
        REQUIRE(global.is<f32>("a"));
        REQUIRE(global.is<bool>("b"));
        REQUIRE(global.is<f32>("c"));
        REQUIRE(global.is<std::string>("d"));
        REQUIRE(global.is<table>("e"));
        REQUIRE(global.is<function<void>>("f"));
        REQUIRE((global.is<std::map<std::string, i32>>("g")));
        REQUIRE((global.is<std::map<i32, i32>>("h")));
        REQUIRE((global.is<std::vector<i32>>("h")));

        REQUIRE_FALSE(global.is<bool>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        REQUIRE_FALSE(global.is<table>("a"));
        REQUIRE_FALSE(global.is<function<void>>("a"));
        REQUIRE_FALSE((global.is<std::vector<std::string>>("h")));
        REQUIRE_FALSE((global.is<std::map<i32, i32>>("g")));

        REQUIRE_FALSE(global.is<bool>("c"));
        REQUIRE_FALSE(global.is<i32>("c"));

        REQUIRE_FALSE(global.is<bool>("d"));
        REQUIRE_FALSE(global.is<table>("d"));
        REQUIRE_FALSE(global.is<function<void>>("d"));
    }
    SUBCASE("has")
    {
        auto res = run("x = { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res);
        REQUIRE(global.has("x"));
        REQUIRE(global.has("x", "y"));
        REQUIRE(global.has("x", "z"));
        REQUIRE(global.has("x", "z", "m"));

        REQUIRE_FALSE(global.has("y"));
        REQUIRE_FALSE(global.has("x", "a"));
        REQUIRE_FALSE(global.has("x", "y", "z"));
        REQUIRE_FALSE(global.has("x", "z", "m", "a"));
    }
}

TEST_CASE("Script.Lua.Literals")
{
    using namespace tcob::literals;
    {
        auto script = "x = 100"_lua;
        i32  x      = script->get_global_table()["x"];
        REQUIRE(x == 100);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Map")
{
    // to Lua
    {
        std::map<std::string, rect_f> m = {{"a", {0, 1, 2, 3}}, {"b", {4, 3, 2, 1}}};
        global["map"]                   = m;
        rect_f x                        = *run<rect_f>("return map.a");
        REQUIRE(x == m["a"]);
        rect_f y = *run<rect_f>("return map.b");
        REQUIRE(y == m["b"]);
    }
    {
        std::map<i32, rect_f> m = {{1, {0, 1, 2, 3}}, {2, {4, 3, 2, 1}}};
        global["map"]           = m;
        rect_f x                = *run<rect_f>("return map[1]");
        REQUIRE(x == m[1]);
        rect_f y = *run<rect_f>("return map[2]");
        REQUIRE(y == m[2]);
    }
    {
        std::unordered_map<std::string, rect_f> m = {{"a", {0, 1, 2, 3}}, {"b", {4, 3, 2, 1}}};
        global["map"]                             = m;
        rect_f x                                  = *run<rect_f>("return map.a");
        REQUIRE(x == m["a"]);
        rect_f y = *run<rect_f>("return map.b");
        REQUIRE(y == m["b"]);
    }
    {
        std::unordered_map<i32, rect_f> m = {{1, {0, 1, 2, 3}}, {2, {4, 3, 2, 1}}};
        global["map"]                     = m;
        rect_f x                          = *run<rect_f>("return map[1]");
        REQUIRE(x == m[1]);
        rect_f y = *run<rect_f>("return map[2]");
        REQUIRE(y == m[2]);
    }
    // from Lua
    {
        std::map<std::string, rect_f> m = *run<std::map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
        REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
    }
    {
        std::map<i32, rect_f> m = *run<std::map<i32, rect_f>>("return {{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}}");
        REQUIRE(m[1] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m[2] == (rect_f {4, 3, 2, 1}));
    }
    {
        std::unordered_map<std::string, rect_f> m = *run<std::unordered_map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
        REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
    }
    {
        std::unordered_map<i32, rect_f> m = *run<std::unordered_map<i32, rect_f>>("return {{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}}");
        REQUIRE(m[1] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m[2] == (rect_f {4, 3, 2, 1}));
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Optional")
{
    std::function Optional = [](i32 i, std::optional<std::string> str) -> i32 {
        if (str.has_value()) {
            return static_cast<i32>(str.value().length());
        }

        return i;
    };
    std::function Optional2 = [](i32 i, std::optional<std::string> str, f32 f) -> f32 {
        if (str.has_value()) {
            return static_cast<f32>(str.value().length()) * f;
        }

        return static_cast<f32>(i) * f;
    };

    global["test"]["Optional"]  = &Optional;
    global["test"]["Optional2"] = &Optional2;

    {
        i32 i = *run<i32>("return test.Optional(100)");
        REQUIRE(i == Optional(100, std::nullopt));
    }
    {
        i32 i = *run<i32>("return test.Optional(100, 'hurray')");
        REQUIRE(i == 6);
    }
    {
        f32 f = *run<f32>("return test.Optional2(100, 2.5)");
        REQUIRE(f == Optional2(100, std::nullopt, 2.5f));
    }
    {
        f32 f = *run<f32>("return test.Optional2(100,'hurray', 2.5)");
        REQUIRE(f == Optional2(100, "hurray", 2.5f));
    }
    {
        std::optional<f32> f = *run<std::optional<f32>>("return 10.25");
        REQUIRE(f.has_value());
        REQUIRE(f == 10.25f);
    }
    {
        auto f = run<std::optional<f32>>("return 'ok'");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
    {
        auto f = run<std::optional<f32>>("return");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Overloads")
{
    SUBCASE("Lambdas")
    {
        auto overload = make_unique_overload(
            []() {
                return 2.5f;
            },
            [](f32 i) {
                return i * 2.5f;
            },
            [](f32 i0, f32 i1) {
                return i0 * i1 * 2.5f;
            },
            [](std::array<f32, 5> arr) {
                return std::accumulate(arr.begin(), arr.end(), 1.f, std::multiplies {}) * 2.5f;
            });
        global["overload"] = overload.get();

        auto res = run<f32>("return overload({1, 2, 3, 4, 5})");
        REQUIRE(res);
        REQUIRE(res.value() == 300.0f);

        res = run<f32>("return overload()");
        REQUIRE(res);
        REQUIRE(res.value() == 2.5f);

        res = run<f32>("return overload(2)");
        REQUIRE(res);
        REQUIRE(res.value() == 5.0f);

        res = run<f32>("return overload(2, 3)");
        REQUIRE(res);
        REQUIRE(res.value() == 15.0f);
    }
    SUBCASE("Member functions")
    {
        TestScriptClass t;
        auto            f1 = resolve_overload<i32, f32>(&TestScriptClass::overload);
        auto            f2 = resolve_overload<f32, i32>(&TestScriptClass::overload);
        auto            f3 = resolve_overload<>(&TestScriptClass::overload);

        auto overload      = make_unique_overload(f1, f2, f3);
        global["obj"]      = &t;
        global["overload"] = overload.get();

        auto res = run<f32>("return overload(obj)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload());

        res = run<f32>("return overload(obj, 1, 2.5)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload(1, 2.5f));

        res = run<f32>("return overload(obj, 2.5, 1)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload(2.5f, 1));
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.RawPointers")
{
    SUBCASE("raw c++ pointer as parameter")
    {
        TestScriptClass t;
        global["obj"] = &t;

        std::function func = [](TestScriptClass* x) { x->set_value(101); };

        global["func"] = &func;

        auto res = run("func(obj)");
        REQUIRE(res);
        REQUIRE(t.get_value() == 101);
    }

    SUBCASE("LuaOwnedPtr")
    {
        REQUIRE(TestScriptClass::ObjCount == 0);
        script_owned_ptr<TestScriptClass> t {new TestScriptClass};
        REQUIRE(TestScriptClass::ObjCount == 1);
        global["obj"] = t;
        auto res      = run("obj = nil");
        REQUIRE(res);
        get_GC().collect();
        REQUIRE(TestScriptClass::ObjCount == 0);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Require")
{
    Require.connect(
        [&](auto&& ev) {
            if (ev.Name == "tab") {
                ev.Table = *run<table>("return { a = 100, b = 200 } ");
            }
        });
    auto res = run("tabx = require 'tab' ");
    REQUIRE(res);
    auto tab = global["tabx"];
    REQUIRE(tab["a"].get<i32>().value() == 100);
    REQUIRE(tab["b"].get<i32>().value() == 200);
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Results")
{
    {
        auto x = run("function return 1 edn", "bad script");
        REQUIRE(x.error() == error_code::Error);
    }
    {
        auto res = run("x = 'ok'");
        REQUIRE(res);
        auto f = global.get<f32>("x");
        REQUIRE(f.error() == error_code::TypeMismatch);
        f = global.get<f32>("testX");
        REQUIRE(f.error() == error_code::Undefined);
        auto s = global.get<std::string>("x");
        REQUIRE(s.has_value());
        REQUIRE(s.value() == "ok");
    }
    {
        auto res = run("tableX = {1,2,3,'a'}");
        REQUIRE(res);
        auto tab = global.get<std::vector<i32>>("tableX");
        REQUIRE(tab.error() == error_code::TypeMismatch);
    }

    {
        auto res = run<i32>("return 'ok'");
        REQUIRE(res.error() == error_code::TypeMismatch);
        REQUIRE(res.value_or(200) == 200);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.RunAsync")
{
    {
        std::string script = R"-(
            function fibo(n)
                local function inner(m)
                    if m < 2 then return m end
                    return inner(m - 1) + inner(m - 2)
                end
                return inner(n)
            end
            return fibo(10);
        )-";

        auto ftr = run_async<i64>(script);
        ftr.wait();
        auto res = ftr.get();
        REQUIRE(res);
        REQUIRE(res.value() == 55);
    }
    {
        std::string script = R"-(
            function fibo(n)
                local function inner(m)
                    if m < 2 then return m end
                    return inner(m - 1) + inner(m - 2)
                end
                return inner(n)
            end
            return fibo(10);
        )-";

        io::delete_file("asynctest.lua");
        io::ofstream stream {"asynctest.lua"};
        stream.write(script);
        stream.flush();
        auto ftr = run_file_async<i64>("asynctest.lua");
        ftr.wait();
        auto res = ftr.get();
        REQUIRE(res);
        REQUIRE(res.value() == 55);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Table")
{
    SUBCASE("basic operations")
    {
        {
            auto res                             = run("tableX = { }");
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            i32 x                                = global["tableX"]["a"]["b"]["c"]["d"];
            REQUIRE(res);
            REQUIRE(x == 100);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            i32  x   = *global.get<i32>("tableX", "top", "x");
            REQUIRE(res);
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);

            i32 x {0};
            REQUIRE(global["tableX"]["top"].as<table>().try_get<i32>(x, "x"));
            REQUIRE(x == 10);

            REQUIRE_FALSE(global.try_get<i32>(x, "x"));
        }
        {
            auto res = run("tableX = {1,{x=1,y=2} }");
            i32  y   = *global.get<i32>("tableX", 2, "y");
            REQUIRE(res);
            REQUIRE(y == 2);
        }
        {
            auto res = run("tableX = {1,{x=1,y=2} }");
            global.set("tableX", 2, "y", 200);
            i32 y = *global.get<i32>("tableX", 2, "y");
            REQUIRE(res);
            REQUIRE(y == 200);
        }
        {
            auto res = run("tableX = {1,{x=1,y=2} }");
            i32  y   = global["tableX"][2]["y"];
            REQUIRE(res);
            REQUIRE(y == 2);
        }
        {
            auto res                 = run("tableX = {1,{x=1,y=2} }");
            global["tableX"][2]["y"] = 200;
            i32 y                    = *global.get<i32>("tableX", 2, "y");
            REQUIRE(res);
            REQUIRE(y == 200);
        }
        {
            table tab = *run<table>("return {4,5,2,1} ");
            i32   x   = tab[1];
            REQUIRE(x == 4);
            x = tab[2];
            REQUIRE(x == 5);
            x = tab[3];
            REQUIRE(x == 2);
            x = tab[4];
            REQUIRE(x == 1);
        }
        {
            auto res = run("tab = {4,5,2,1} ");
            REQUIRE(res);
            {
                table tab1 = global["tab"];
                tab1[1]    = 100;
                REQUIRE((i32)tab1[1] == 100);
            }
            {
                table tab1 = global["tab"];
                tab1[1]    = 100;
                REQUIRE((i32)tab1[1] == 100);
            }
        }
        {
            auto res = run("tab1 = {4,5,2,1} tab2 = {1,2,3,4} ");
            REQUIRE(res);
            table tab1a = global["tab1"];
            table tab1b = global["tab1"];
            table tab2  = global["tab2"];
            REQUIRE(tab1a == tab1b);
            REQUIRE(tab1a != tab2);
        }
        {
            table tab = *run<table>("return {left=2.7, top=3.1, width=2.3, height=55.2} ");
            f32   f   = tab["top"];
            REQUIRE(f == 3.1f);
        }
        {
            table tab = *run<table>("return {left=2.7, top=3.1, width=2.3, height=55.2} ");
            REQUIRE(tab.has("left"));
            REQUIRE(tab.has("top"));
            REQUIRE(tab.has("width"));
            REQUIRE(tab.has("height"));
        }
        {
            table tab = *run<table>("return {a = 2.4, b = true, c = 'hello'} ");
            REQUIRE(tab.is<f32>("a"));
            REQUIRE(tab["b"].is<bool>());
            REQUIRE(tab.is<std::string>("c"));
        }
        {
            table tab = *run<table>("return { a = 2.4, b = 'ok' } ");
            REQUIRE(tab["a"].get<bool>().value_or(false) == false);
            REQUIRE(tab["b"].get<std::string>().value_or("default") == "ok");
        }
        {
            table                    tab = *run<table>("return {a = 2.4, b = true, c = 'hello', 42} ");
            std::vector<std::string> vect {"a", "b", "c"};
            std::vector<std::string> keys = tab.get_keys<std::string>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            table            tab = *run<table>("return { 'a', 3, 55, a = 22 }");
            std::vector<i32> vect {1, 2, 3};
            std::vector<i32> keys = tab.get_keys<i32>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            table                                       tab = *run<table>("return {a = 2.4, 3, c = 'hello'} ");
            std::vector<std::variant<i32, std::string>> vect {1, "a", "c"};
            auto                                        keys = tab.get_keys<std::variant<i32, std::string>>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            auto res = run("rectF = {x=2.7, y=3.1, width=2.3, height=55.2} ");
            REQUIRE(res);
            table tab = global["rectF"];
            f32   f   = tab["x"];
            REQUIRE(f == 2.7f);
        }
        {
            auto res = run(
                "rectF = {x=2.7, y=3.1, width=2.3, height=55.2} "
                "function tabletest(x) "
                "   return x.y "
                "end");
            REQUIRE(res);
            table tab          = global["rectF"];
            tab["y"]           = 100.5f;
            function<f32> func = global["tabletest"];
            f32           x    = func(tab);
            REQUIRE(x == 100.5f);
            REQUIRE((f32)tab["y"] == 100.5f);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table top = global["tableX"]["top"];
            i32   x   = top["x"];
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table   tab = global["tableX"];
            point_i top = tab["top"];
            REQUIRE(top.X == 10);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table tab = global["tableX"];
            i32   top = tab["top"]["x"];
            REQUIRE(top == 10);
        }
        {
            auto res = run("tableX = {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table tab       = global["tableX"];
            tab["top"]["x"] = 400;
            i32 top         = global["tableX"]["top"]["x"];
            REQUIRE(top == 400);
        }
        {
            auto res = run("tableX = { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            table tab               = global["tableX"];
            tab["a"]["b"]["c"]["d"] = 42;
            i32 top                 = global["tableX"]["a"]["b"]["c"]["d"];
            REQUIRE(top == 42);
        }
        {
            auto res = run("tableX = { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            table tab = global["tableX"];
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 2);
            res = run(
                "tableX.a.b.c.d = 4");
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 4);
        }
        {
            auto res = run("tableX = { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            table tab = global["tableX"]["a"]["b"]["c"];
            REQUIRE((i32)tab["d"] == 2);
            res = run(
                "tableX.a.b.c.d = 4");
            REQUIRE((i32)tab["d"] == 4);
            tab = global["tableX"];
            REQUIRE(tab["a"]["b"]["c"]["d"].get<i32>().value() == 4);
        }
        {
            auto res = run("tableX = { a={ b={ bb = 'ok', c={ d=2 } } } }");
            REQUIRE(res);
            table tab                            = global["tableX"];
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            REQUIRE((i32)global["tableX"]["a"]["b"]["c"]["d"] == 100);
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 100);
            std::string x = tab["a"]["b"]["bb"];
            REQUIRE(x == "ok");
        }
        {
            auto res = run("tableX = {  }");
            REQUIRE(res);
            table tab = global["tableX"];
            table subt {};
            REQUIRE_FALSE(subt.is_valid());
            tab["sub"] = subt;
            REQUIRE(subt.is_valid());
            subt["x"] = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            auto res = run("tableX = {  }");
            REQUIRE(res);
            table tab       = global["tableX"];
            tab["sub"]      = table {};
            tab["sub"]["x"] = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            auto res = run("tableX = {  }");
            REQUIRE(res);
            table tab = global["tableX"];
            table subt;
            tab["sub"] = subt;
            subt["x"]  = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            table tab0 = *run<table>("tableX = {  } tableY = { } return tableX");
            table tab1 = global["tableX"];
            REQUIRE(tab0 == tab1);
            table tab2 = global["tableY"];
            REQUIRE(tab0 != tab2);
        }
    }
    SUBCASE("metatable")
    {
        auto tab          = table {get_view()};
        global["table"]   = tab;
        auto metatab      = table {get_view()};
        metatab["__name"] = "hello world";
        tab.set_metatable(metatab);

        auto res = run<std::string>("return tostring(table)");
        REQUIRE(res.has_value());
        REQUIRE(res.value().starts_with("hello world"));
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.TableDumper")
{
    {
        auto res = run(
            "tableX = { 2.7, 5, 6, a = 69, 7, 8, x = 10, t = { a = 20, 30.2 } }");
        REQUIRE(res);
        table tab = global["tableX"];

        io::iomstream fs;
        fs << "return ";
        tab.dump(fs);
        fs.seek(0, io::seek_dir::Begin);
        table tab2 = *run<table>(fs.read_string(fs.size_in_bytes()));

        REQUIRE((f32)tab2[1] == 2.7f);
        REQUIRE((f32)tab[1] == (f32)tab2[1]);

        REQUIRE((i32)tab2[4] == 7);
        REQUIRE((i32)tab[4] == (i32)tab2[4]);

        REQUIRE((i32)tab2["x"] == 10);
        REQUIRE((i32)tab["x"] == (i32)tab2["x"]);

        REQUIRE((f32)tab2["t"][1] == 30.2f);
        REQUIRE((f32)tab["t"][1] == (f32)tab2["t"][1]);
    }
    {
        auto res = run(
            "tableX = { left = 2.7, x = 10, t = { a = 20, y = 30.2, m = { z = 1, f = 3 } }, y = true, z = 'ok' }");
        REQUIRE(res);
        table tab = global["tableX"];

        io::iomstream fs;
        fs << "tab = ";
        tab.dump(fs);
        fs << "\nreturn tab";

        fs.seek(0, io::seek_dir::Begin);
        table tab2 = *run<table>(fs.read_string(fs.size_in_bytes()));

        REQUIRE((f32)tab2["left"] == 2.7f);
        REQUIRE((f32)tab["left"] == (f32)tab2["left"]);

        REQUIRE((bool)tab2["y"] == true);
        REQUIRE((bool)tab["y"] == (bool)tab2["y"]);

        REQUIRE(tab2["z"].get<std::string>().value() == "ok");
        REQUIRE(tab["z"].get<std::string>().value() == tab2["z"].get<std::string>().value());

        REQUIRE((i32)tab2["t"]["a"] == 20);
        REQUIRE((i32)tab["t"]["a"] == (i32)tab2["t"]["a"]);

        REQUIRE((i32)tab2["t"]["m"]["z"] == 1);
        REQUIRE((i32)tab["t"]["m"]["z"] == (i32)tab2["t"]["m"]["z"]);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.TcobTypes")
{
    static_assert(Serializable<rect_f, table>);

    std::function testFuncColor = [](color c) {
        return color {static_cast<u8>(c.R * 2),
                      static_cast<u8>(c.G * 2),
                      static_cast<u8>(c.B * 2),
                      static_cast<u8>(c.A * 2)};
    };
    std::function testFuncPointF = [](point_f p) { return point_f {p.X * 2, p.Y * 2}; };
    std::function testFuncPointI = [](point_i p) { return point_i {p.X * 2, p.Y * 2}; };
    std::function testFuncSizeI  = [](size_i s) { return size_i {s.Width * 5, s.Height * 8}; };
    std::function testFuncRectF  = [](rect_f r) { return rect_f {r.X * 2, r.Y * 2, r.Width * 2, r.Height * 2}; };
    std::function testFuncRectI  = [](rect_i r) { return rect_i {r.X * 2, r.Y * 2, r.Width * 2, r.Height * 2}; };
    std::function testFuncMix    = [](i32 i, rect_f r, color c, std::string const& s, bool b, point_i p) {
        f32 ret = static_cast<f32>(i) + r.X + c.A + static_cast<f32>(s.length()) + (b ? 1 : 100) + static_cast<f32>(p.X);
        return ret;
    };

    global["test"]["Color"]  = &testFuncColor;
    global["test"]["PointF"] = &testFuncPointF;
    global["test"]["PointI"] = &testFuncPointI;
    global["test"]["RectF"]  = &testFuncRectF;
    global["test"]["RectI"]  = &testFuncRectI;
    global["test"]["SizeI"]  = &testFuncSizeI;
    global["test"]["Mix"]    = &testFuncMix;
    SUBCASE("get from globals")
    {
        auto res = run(
            "rectI = {x=3, y=6, width=10, height=20} "
            "rectF = {x=2.7, y=3.1, width=2.3, height=55.2} "
            "color = { r= 1, g = 2, b = 3, a = 1} "
            "pointI = { x = 20, y = 400 } "
            "pointF = { x = 4.5, y = 3.23 } ");
        REQUIRE(res);
        REQUIRE(global["color"].get<color>().value() == color(1, 2, 3, 1));
        REQUIRE(global["pointI"].get<point_i>().value() == point_i(20, 400));
        REQUIRE(global["pointF"].get<point_f>().value() == point_f(4.5f, 3.23f));
        REQUIRE(global["rectI"].get<rect_i>().value() == rect_i(3, 6, 10, 20));
        REQUIRE(global["rectF"].get<rect_f>().value() == rect_f(2.7f, 3.1f, 2.3f, 55.2f));
    }
    SUBCASE("is")
    {
        auto res = run(
            "rectI = {x=3, y=6, width=10, height=20} "
            "rectI = test.RectI(rectI) "

            "rectF = {x=2.7, y=3.1, width=2.3, height=55.2} "
            "rectF = test.RectF(rectF) "

            "color = { r = 1, g = 2, b = 3, a = 1 } "
            "color = test.Color(color) "

            "pointI = { x = 20, y = 400 } "
            "pointI = test.PointI(pointI) "

            "pointF = { x = 4.5, y = 3.23 } "
            "pointF = test.PointF(pointF) "

            "sizeI = { width = 20, height = 400 } "
            "sizeI = test.SizeI(sizeI) "

            "rectIS = {x=3, y=6, width=10, height=20} "
            "rectFS = {x=2.7, y=3.1, width=2.3, height=55.2} "
            "pointIS = { x=20, y=400 } "
            "pointFS = { x=4.5, y=3.23 } "

            "degree = 160 ");
        REQUIRE(res);

        color c = global["color"];
        REQUIRE(global.is<color>("color"));
        REQUIRE(c == color(1 * 2, 2 * 2, 3 * 2, 1 * 2));

        point_i p1 = global["pointI"];
        REQUIRE(global.is<point_i>("pointI"));
        REQUIRE(global.is<point_i>("pointIS"));
        REQUIRE(p1 == point_i(20 * 2, 400 * 2));

        point_f p2 = global["pointF"];
        REQUIRE(global.is<point_f>("pointF"));
        REQUIRE(global.is<point_f>("pointFS"));
        REQUIRE(p2 == point_f(4.5f * 2, 3.23f * 2));

        size_i s1 = global["sizeI"];
        REQUIRE(global.is<size_i>("sizeI"));
        REQUIRE(s1 == size_i(20 * 5, 400 * 8));

        rect_i r1 = global["rectI"];
        REQUIRE(global.is<rect_i>("rectI"));
        REQUIRE(global.is<rect_i>("rectIS"));
        REQUIRE(r1 == rect_i(3 * 2, 6 * 2, 10 * 2, 20 * 2));

        rect_f r2 = global["rectF"];
        REQUIRE(global.is<rect_f>("rectF"));
        REQUIRE(global.is<rect_f>("rectFS"));
        REQUIRE(r2 == rect_f(2.7f * 2, 3.1f * 2, 2.3f * 2, 55.2f * 2));

        degree_f deg = global["degree"];
        REQUIRE(global.is<degree_f>("degree"));
        REQUIRE(deg == 160);
    }
    SUBCASE("parameters")
    {
        auto res = run(
            "rectF = {x=2.7, y=3.1, width=2.3, height=55.2} "
            "color = { r = 1, g = 2, b = 3, a = 1} "
            "pointI = { x = 20, y = 400 } "
            "x = test.Mix(100, rectF, color, 'Hello', false, pointI)");
        REQUIRE(res);
        f32 x = global["x"];

        REQUIRE(x == testFuncMix(100, rect_f(2.7f, 3.1f, 2.3f, 55.2f), color(1, 2, 3, 1), "Hello", false, point_i(20, 400)));
    }
    SUBCASE("error checking")
    {
        auto res = run(
            "rectF = {x=2.7, y=3.1, width='hello', height=true} "
            "color = { r = 'red', g = 'green', b = 'blue', a = 'aqua' } "
            "pointI = { x = '1', y = '400' } ");
        REQUIRE(res);

        auto rectF = global.get<rect_f>("rectF");
        REQUIRE(rectF.error() == error_code::TypeMismatch);

        auto col = global.get<color>("color");
        REQUIRE(col.error() == error_code::TypeMismatch);

        auto pointI = global.get<point_f>("pointI");
        REQUIRE(pointI.error() == error_code::TypeMismatch);
    }
    SUBCASE("api: color_stop")
    {
        auto res = run<color_stop>("return { pos = 150, value = { r = 10, g = 20, b = 40 } }").value();
        REQUIRE(res.Position == 150.f);
        REQUIRE(res.Value == color {10, 20, 40, 255});
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.TypeCoercion")
{
    SUBCASE("string from int")
    {
        auto res = run("a = 100 ");
        REQUIRE(res);
        REQUIRE(global.is<i32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        std::string val = global["a"];
        REQUIRE(val == "100");
    }
    SUBCASE("string from number")
    {
        auto res = run("a = 100.5 ");
        REQUIRE(res);
        REQUIRE(global.is<f32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        std::string val = global["a"];
        REQUIRE(val == "100.5");
    }
    SUBCASE("number from string")
    {
        auto res = run("a = '100.5' ");
        REQUIRE(res);
        REQUIRE(global.is<std::string>("a"));
        REQUIRE_FALSE(global.is<f32>("a"));
        f32 val = global["a"];
        REQUIRE(val == 100.5);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Upvalue")
{
    SUBCASE("get upvalues")
    {
        {
            auto res = run(
                "A = 100 "
                "function bar() return A end ");
            REQUIRE(res);
            function<i32> func = global["bar"];
            auto          upvalue {func.get_upvalues()};
            REQUIRE(upvalue.size() == 1);
            REQUIRE(upvalue.contains("_ENV"));
            REQUIRE(func() == 100);
        }
        {
            auto res = run(
                "A = 100 "
                "local function foo() return 42 end "
                "function bar() return foo() * A end ");
            REQUIRE(res);
            function<i32> func = global["bar"];
            auto          upvalue {func.get_upvalues()};
            REQUIRE(upvalue.size() == 2);
            REQUIRE(upvalue.contains("foo"));
            REQUIRE(upvalue.contains("_ENV"));
            REQUIRE(func() == 4200);
        }
    }
    SUBCASE("change _ENV")
    {
        table newEnv {};
        global["newEnv"] = newEnv;
        newEnv["A"]      = 11;
        newEnv["B"]      = 22;
        newEnv["C"]      = 33;

        auto res = run(
            "A = 100 "
            "B = 70 "
            "C = 42 "
            "function bar() return A * B * C end ");
        REQUIRE(res);
        function<i32> func = global["bar"];

        REQUIRE(func() == 100 * 70 * 42);

        REQUIRE(func.set_upvalue("_ENV", newEnv));
        REQUIRE(func() == 11 * 22 * 33);
    }

    SUBCASE("change local")
    {
        {
            auto res = run(
                "function GlobalFoo() return 21 end "
                "A = 100 "
                "local function foo() return 42 end "
                "function bar() return foo() * A end ");
            REQUIRE(res);
            function<i32> func = global["bar"];

            REQUIRE(func() == 4200);

            function<i32> newFoo = global["GlobalFoo"];
            REQUIRE(func.set_upvalue("foo", newFoo));
            REQUIRE(func() == 2100);
        }
        {
            auto res = run(
                "function GlobalFoo() return 21 end "
                "local foo "
                "function bar() return foo() end ");
            REQUIRE(res);
            function<i32> func   = global["bar"];
            function<i32> newFoo = global["GlobalFoo"];
            REQUIRE(func.set_upvalue("foo", newFoo));
            REQUIRE(func() == 21);
        }
        {
            auto res = run(
                "function GlobalFoo() return 21 end "
                "local function foo() return 42 end "
                "function bar0() return foo() end "
                "function bar1() return foo() * 10 end ");
            REQUIRE(res);
            function<i32> func0 = global["bar0"];
            REQUIRE(func0() == 42);
            function<i32> func1 = global["bar1"];
            REQUIRE(func1() == 420);

            function<i32> newFoo = global["GlobalFoo"];
            REQUIRE(func0.set_upvalue("foo", newFoo));
            REQUIRE(func0() == 21);
            REQUIRE(func1() == 210);
        }
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.UserDefinedConversion")
{
    std::function Foo = [](foo const& f) {
        return f.x + f.y + f.z;
    };
    global["test"]["Foo"] = &Foo;

    {
        foo i = *run<foo>("return {x=3,y=2,z=1}");
        REQUIRE(i.x == 3);
    }
    {
        i32 i = *run<i32>("return test.Foo({x=3,y=2,z=1})");
        REQUIRE(i == Foo(foo {3, 2, 1}));
    }
    {
        auto res = run("foo = {x=3,y=2,z=1}");
        REQUIRE(res);
        REQUIRE(global.is<foo>("foo"));
        res = run("foo = {x=3,n=2,z=1}");
        REQUIRE_FALSE(global.is<foo>("foo"));
    }
    {
        auto res = run(
            "function bar(p) "
            "   return p.x * p.y * p.z "
            "end ");
        REQUIRE(res);
        function<i32> func = global["bar"];
        i32           a    = *func.call(foo {1, 2, 3});
        REQUIRE(a == 6);
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.VariadicFunctions")
{
    SUBCASE("variadic lua function")
    {
        auto res = run(
            "function testArg(...) "
            "   local retValue = 0 "
            "   for i,v in ipairs({...}) do  "
            "       retValue = retValue + v "
            "   end "
            "   return retValue "
            "end ");
        REQUIRE(res);
        function<i32> func = global["testArg"];
        i32           a    = func(1, 2, 3, 4, 5, 6);
        REQUIRE(a == (1 + 2 + 3 + 4 + 5 + 6));
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Variant")
{
    std::function Variant = [](std::variant<f32, std::string, bool> var) {
        if (std::get_if<f32>(&var)) {
            return "f32";
        }

        if (std::get_if<std::string>(&var)) {
            return "string";
        }

        if (std::get_if<bool>(&var)) {
            return "bool";
        }

        return "";
    };
    global["test"]["Variant"] = &Variant;

    SUBCASE("cpp parameter")
    {
        std::string str = *run<std::string>(
            "return test.Variant('hi')");
        REQUIRE(str == Variant("hi"s));
        str = *run<std::string>(
            "return test.Variant(1.23)");
        REQUIRE(str == Variant(1.23f));
        str = *run<std::string>(
            "return test.Variant(true)");
        REQUIRE(str == Variant(true));
    }
    SUBCASE("lua parameter")
    {
        auto res = run(
            "function foo(x) "
            "return x * 10 "
            "end ");
        REQUIRE(res);
        std::variant<std::string, i32, bool> var  = 100;
        function<i32>                        func = global["foo"];
        i32                                  a    = func(var);
        REQUIRE(a == 1000);
    }
    SUBCASE("return value")
    {
        {
            auto const var = run<std::variant<std::string, i16, bool>>("return 100").value();
            REQUIRE(std::get<i16>(var) == 100);
        }
        {
            auto const var = run<std::variant<std::string, u64, bool>>("return 100").value();
            REQUIRE(std::get<u64>(var) == 100);
        }
        {
            auto const res = run<std::variant<u64, bool>>("return 'OK'");
            REQUIRE(res.error() == error_code::TypeMismatch);
        }
        {
            auto const var = run<std::variant<u64, f64>>("return 100").value();
            REQUIRE(std::get<u64>(var) == 100);
        }
        {
            auto const var = run<std::variant<u64, f64>>("return 100.0").value();
            REQUIRE(std::get<f64>(var) == 100);
        }
        {
            auto const var = run<std::variant<int, std::vector<std::string>, bool>>("return {'ok','ko'}").value();
            REQUIRE(std::get<std::vector<std::string>>(var) == std::vector<std::string> {"ok", "ko"});
        }
        {
            auto const var = run<std::variant<int, std::vector<int>, bool>>("return {1,2,3}").value();
            REQUIRE(std::get<std::vector<int>>(var) == std::vector<int> {1, 2, 3});
        }
        {
            auto const var = run<std::variant<int, std::vector<bool>, bool>>("return {true,false,true,false,true}").value();
            REQUIRE(std::get<std::vector<bool>>(var) == std::vector<bool> {true, false, true, false, true});
        }
        {
            auto const var = run<std::variant<std::vector<int>, std::vector<std::string>, int>>("return {'ok','ko'}").value();
            REQUIRE(std::get<std::vector<std::string>>(var) == std::vector<std::string> {"ok", "ko"});
        }
        {
            auto const var0 = run<std::variant<std::vector<int>, std::vector<std::string>, int>>("return {1,2,3}").value();
            REQUIRE(std::get<std::vector<int>>(var0) == std::vector<int> {1, 2, 3});
            auto const var1 = run<std::variant<std::vector<std::string>, std::vector<int>, int>>("return {1,2,3}").value();
            REQUIRE(std::get<std::vector<int>>(var1) == std::vector<int> {1, 2, 3});
        }
        {
            auto const var = run<std::variant<std::vector<bool>, point_f, int>>("return {x=1,y=2}").value();
            REQUIRE(std::get<point_f>(var) == point_f {1, 2});
        }
        {
            auto const var = run<std::variant<std::vector<bool>, size_f, int>>("return {width=1,height=2}").value();
            REQUIRE(std::get<size_f>(var) == size_f {1, 2});
        }
        {
            auto const var = run<std::variant<std::vector<bool>, rect_f, int>>("return {x=1,y=2,width=3,height=4}").value();
            REQUIRE(std::get<rect_f>(var) == rect_f {1, 2, 3, 4});
        }
        {
            auto const var = run<std::variant<std::vector<bool>, color, int>>("return {r=1,g=2,b=4}").value();
            REQUIRE(std::get<color>(var) == color {1, 2, 4, 255});
        }
    }
}

TEST_CASE_FIXTURE(LuaScriptTests, "Script.Lua.Warnings")
{
    std::string warning;
    Warning.connect([&](auto&& ev) { warning += ev.Message; });
    auto res {run("warn('test')")};
    REQUIRE(warning == "test");
    warning = "";
    res     = run("warn('test','1','2','3')");
    REQUIRE(warning == "test123");
}
