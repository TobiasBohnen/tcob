#include "WrapperTestsClass.hpp"
#include "tests.hpp"

#include <cstring>
#include <numeric>

using namespace tcob::scripting;
using namespace tcob::scripting::squirrel;

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

namespace tcob::scripting::squirrel {
template <>
struct converter<foo> {
    auto static IsType(vm_view ls, SQInteger idx) -> bool
    {
        table lt {ls, idx};
        return lt.has("x") && lt.has("y") && lt.has("z");
    }

    auto static From(vm_view ls, SQInteger& idx, foo& value) -> bool
    {
        if (ls.is_table(idx)) {
            table lt {ls, idx++};

            value.x = lt["x"];
            value.y = lt["y"];
            value.z = lt["z"];
        }
        return true;
    }

    void static To(vm_view ls, foo const& value)
    {
        ls.new_table();
        table lt {ls, -1};

        lt["x"] = value.x;
        lt["y"] = value.y;
        lt["z"] = value.z;
    }
};
}

class SquirrelScriptTests : public squirrel::script {
public:
    SquirrelScriptTests()
        : global(get_root_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Array")
{
    {
        array arr = *run<array>("return [4,5,2,1] ");
        REQUIRE(arr.get_size() == 4);
        i32 x = arr[0];
        REQUIRE(x == 4);
        x = arr[1];
        REQUIRE(x == 5);
        x = arr[2];
        REQUIRE(x == 2);
        x = arr[3];
        REQUIRE(x == 1);
    }
    {
        auto res = run("arr <- [4,5,2,1] ");
        REQUIRE(res);
        {
            array arr1 = global["arr"];
            arr1[1]    = 100;
            REQUIRE((i32)arr1[1] == 100);
        }
        {
            array arr1 = global["arr"];
            arr1[1]    = 100;
            REQUIRE((i32)arr1[1] == 100);
        }
    }
    {
        auto res = run("arr1 <- [4,5,2,1]; ");
        REQUIRE(res);
        array arr1 = global["arr1"];
        arr1[3]    = 999;
        array arr2 = global["arr1"];
        REQUIRE((i32)arr2[3] == 999);
    }
    {
        auto res = run("arr1 <- [4,5,2,1]; arr2 <- [1,2,3,4] ");
        REQUIRE(res);
        array arr1a = global["arr1"];
        array arr1b = global["arr1"];
        array arr2  = global["arr2"];
        REQUIRE(arr1a == arr1b);
        REQUIRE(arr1a != arr2);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.ClassesAndInstances")
{
    SUBCASE("fields")
    {
        class_t c {class_t::PushNew(get_view())};
        c["value"]    = 100;
        global["foo"] = c;

        auto res = run<i32>("inst <- foo(); return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);

        instance i = global["inst"];
        REQUIRE((i32)i["value"] == 100);
        i["value"] = 420;

        res = run<i32>("return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 420);

        res = run<i32>("inst2 <- foo(); return inst2.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);
    }
    SUBCASE("functions")
    {
        std::function func = [](f32 x, f32 y) {
            return x * y;
        };

        class_t c {class_t::PushNew(get_view())};
        c["func"]     = &func;
        global["foo"] = c;

        auto res = run<f32>("inst <- foo(); return inst.func(2.5, 4.1)");
        REQUIRE(res);
        REQUIRE(res.value() == 2.5f * 4.1f);
    }
    SUBCASE("create instance")
    {
        class_t c {class_t::PushNew(get_view())};
        c["value"]     = 100;
        global["inst"] = c.create_instance();

        auto res = run<i32>("return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Closures")
{
    std::function testFuncPrim = [](i32 i, f32 f, f64 d, bool b) {
        return std::to_string(i) + std::to_string(f) + std::to_string(d) + std::string(b ? "true" : "false");
    };

    i32           voidTest     = 0;
    std::function testFuncVoid = [&voidTest] {
        voidTest++;
    };

    global["test"]["Prim"] = &testFuncPrim;
    global["test"]["Void"] = &testFuncVoid;

    {
        auto res = run("str <- test.Prim(20, 4.4, 5.22, true)");
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
        f32 x              = *run<f32>("return testFunc([4, 6.5])");
        REQUIRE(x == testfuncpair({4, 6.5f}));
    }
    {
        auto l             = std::function([](i32 i) { return (f32)i * 2.5f; });
        global["testFunc"] = &l;
        f32 x              = *run<f32>("return testFunc(2)");
        REQUIRE(x == 5.0f);
    }
    {
        global["foo"]             = table::PushNew(get_view());
        auto l                    = std::function([](i32 i) { return (f32)i * 2.5f; });
        global["foo"]["testFunc"] = &l;
        f32 x                     = *run<f32>("return foo.testFunc(2)");
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

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Enums")
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
            "ex <- testEnum.False; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::False);
        num = *run<testEnum>(
            "ex <- testEnum.True; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::True);
        num = *run<testEnum>(
            "ex <- testEnum.FileNotFound; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::FileNotFound);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Functions")
{
    {
        auto res = run("function testPoint(p) { return p.x * p.y }");
        REQUIRE(res);
        function<i32> func = global["testPoint"];
        i32           a    = *func.call(point_i {2, 4});
        REQUIRE(a == 2 * 4);
        a = func(point_i {2, 4});
        REQUIRE(a == 2 * 4);
    }
    {
        function<i32> func = *run<function<i32>>("return @(x) x*x ");
        i32           a    = func(200);
        REQUIRE(a == 200 * 200);
    }
    {
        auto func = run<function<i32>>("return function() {return 100} ").value();
        REQUIRE(func() == 100);
        REQUIRE(100 == func());
    }
    {
        function<i32> func = *run<function<i32>>("return @() 100 ");
        REQUIRE_FALSE(func() == 10);
        REQUIRE_FALSE(10 == func());
    }
    {
        function<i32> func = *run<function<i32>>("return @() 5 ");
        REQUIRE(func() * 20 == 100);
        REQUIRE(20 * func() == 100);
    }
    {
        function<i32> func = *run<function<i32>>("return @() 500 ");
        REQUIRE(func() / 5 == 100);
        REQUIRE(50000 / func() == 100);
    }
    {
        function<i32> func = *run<function<i32>>("return @() 95 ");
        REQUIRE(func() + 5 == 100);
        REQUIRE(5 + func() == 100);
    }
    {
        function<i32> func = *run<function<i32>>("return @() 105 ");
        REQUIRE(func() - 5 == 100);
        REQUIRE(205 - func() == 100);
    }
    {

        auto func = run<function<std::vector<i32>>>("return @() [5, 4, 3, 2, 1] ").value();
        auto a    = func();
        REQUIRE(a[0] == 5);
        REQUIRE(a[1] == 4);
        REQUIRE(a[2] == 3);
        REQUIRE(a[3] == 2);
        REQUIRE(a[4] == 1);
    }
    {
        auto func = run<function<std::map<std::string, i32>>>("return @() {x=5, y=4, b=3, r=2, aa=1} ").value();
        auto a    = func();
        REQUIRE(a["x"] == 5);
        REQUIRE(a["y"] == 4);
        REQUIRE(a["b"] == 3);
        REQUIRE(a["r"] == 2);
        REQUIRE(a["aa"] == 1);
    }
    {
        auto res = run("function testPoint(p) { return p.x * p.y } ");
        REQUIRE(res);
        function<i32> func = global["testPoint"];
        i32           a    = func(point_i {2, 4});
        REQUIRE(a == 2 * 4);
    }
    {
        auto res = run("function testPoint(p) { return p.x * p.y } ");
        REQUIRE(res);
        function<i32> func = global["testPoint"];
        point_i       p    = point_i {2, 4};
        i32           a    = func(p);
        REQUIRE(a == 2 * 4);
        a = func(point_i {6, 4});
        REQUIRE(a == 6 * 4);
        a = func(point_i {15, 7});
        REQUIRE(a == 15 * 7);
    }
    {
        auto res = run(
            "x <- 0;"
            "function testVoid(p)  {x = p.x * p.y } ");
        REQUIRE(res);
        function<void> func = global["testVoid"];
        func(point_i {2, 4});
        i32 x = global["x"];
        REQUIRE(x == 2 * 4);
    }
    {
        auto res = run("function testMulti(f,p,r,b)  { return f * p.x * r.y } ");
        REQUIRE(res);
        function<f32> func = global["testMulti"];
        f32           x    = func(10.4f, point_i {2, 4}, rect_f {0, 20, 4, 5}, true);
        REQUIRE(x == 10.4f * 2 * 20);
    }
    {
        auto res = run("function testTable(x,y) { return { a = x, b = y } } ");
        REQUIRE(res);
        function<table> func = global["testTable"];
        table           tab  = func(10, 20);
        REQUIRE((i32)tab["a"] == 10);
        REQUIRE((i32)tab["b"] == 20);
    }
    {
        /*
        auto res = run("function testTable(x,y) { return x*y } ");
        REQUIRE(res);
        function<i32> func = global["testTable"];
        REQUIRE(func(10, 20) == 10 * 20);
        REQUIRE(func(20, 40) == 20 * 40);
        {
            ofstream fs{"test.luac"};
            func.dump(fs);
        }

        function<i32> func2 = load_binary<i32>("test.luac");
        REQUIRE(func2(10, 20) == 10 * 20);
        REQUIRE(func2(20, 40) == 20 * 40);
        */
    }
    SUBCASE("nullptr as parameter")
    {
        auto res = run(
            "function foo(a, b, c) {"
            "  if (!a) return 1; "
            "  if (!b) return 2; "
            "  if (!c) return 3; "
            "  return 0 "
            "} ");
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
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Generators")
{
    {
        auto res = run(R"(
            function geny() {
                for(local i=1;i<10;i+=1) { yield i; }
                return null;
            }

            co <- geny();
        )");
        REQUIRE(res);

        REQUIRE(global.is<generator>("co"));
        generator co = global["co"];
        REQUIRE(co.resume<i32>().value() == 1);
        REQUIRE(co.resume<i32>().value() == 2);
        REQUIRE(co.resume<i32>().value() == 3);
    }
    {
        auto res = run(R"(
            function geny(x) {
                for(local i=1;i<10;i+=1) { yield i*x; }
                return null;
            }

            co <- geny(2);
        )");
        REQUIRE(res);

        generator co = global["co"];
        REQUIRE(co.resume<i32>().value() == 2);
        REQUIRE(co.resume<i32>().value() == 4);
        REQUIRE(co.resume<i32>().value() == 6);
    }
    {
        auto res = run(R"(
            function geny() {
                for(local i=1;i<=2;i+=1) { yield i; }
                return null;
            }

            co <- geny();
        )");
        REQUIRE(res);

        generator co = global["co"];

        using return_type = std::optional<i32>;

        auto result = co.resume<return_type>();
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);
        result = co.resume<return_type>();
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
        result = co.resume<return_type>();
        REQUIRE(result.value() == std::nullopt);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.GetSet")
{
    SUBCASE("get/set std::string")
    {
        auto res = run("x <- \"ok\"");
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
        auto res = run("x <- \"ok\"");
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
        auto res = run("x <- 1337");
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
        auto res = run("x <- 1337");
        REQUIRE(res);
        REQUIRE(global.has("x"));
        global["x"] = nullptr;
        REQUIRE_FALSE(global.has("x"));
    }
    SUBCASE("get/set u8")
    {
        auto res = run("x <- 30 ");
        REQUIRE(res);
        u8 x = global["x"];
        REQUIRE(x == 30);
    }
    SUBCASE("get/set nested i32")
    {
        auto res = run("x <- { y = { z = 30 } }");
        REQUIRE(res);
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 30);
        global["x"]["y"]["z"] = 2000;
        x                     = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ subscript")
    {
        auto res = run("x <- { y = {  } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ set")
    {
        auto res = run("x <- { y = {  } }");
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

        global["testVar4"] = 1.5f;
        auto f1            = global.get<f32>("testVar4");
        REQUIRE(f1.value() == 1.5f);
    }
    SUBCASE("access nested table")
    {
        auto res = run("x <- { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res);
        i32 m = global["x"]["z"]["m"];
        REQUIRE(m == 75);
    }
    SUBCASE("access created global from function")
    {
        REQUIRE_FALSE(global.has("testVar"));
        global["testVar"] = 400;
        auto res          = run("function foo() {return testVar * 10 }");
        REQUIRE(res);
        function<i32> func = global["foo"];
        i32           a    = func();
        REQUIRE(a == 400 * 10);
        global["testVar"] = 2000;
        a                 = func();
        REQUIRE(a == 2000 * 10);
    }
    SUBCASE("get/set undefined nested i32")
    {
        auto res = run("x <- { y = { } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        i32 x                 = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.IsHas")
{
    SUBCASE("is")
    {
        auto res = run(
            "a <- 100;"
            "b <- true;"
            "c <- 10.3;"
            "d <- \"hello\";"
            "e <- { };"
            "f <- function(x) { return x };"
            "g <- { a = 1, b = 2, c = 3 };"
            "h <- [ 1, 2, 3 ];");

        REQUIRE(res);

        REQUIRE(global.is<i32>("a"));
        REQUIRE(global.is<bool>("b"));
        REQUIRE(global.is<f32>("c"));
        REQUIRE(global.is<std::string>("d"));
        REQUIRE(global.is<table>("e"));
        REQUIRE(global.is<function<void>>("f"));
        REQUIRE((global.is<std::map<std::string, i32>>("g")));
        //  REQUIRE((global.is<std::map<i32, i32>>("h")));
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
        auto res = run("x <- { y = 100, z = { m = 75, n = 5 } }");
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

TEST_CASE("Script.Squirrel.Literals")
{
    using namespace tcob::literals;
    {
        auto script = "x <- 100"_squirrel;
        i32  x      = script->get_root_table()["x"];
        REQUIRE(x == 100);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Map")
{
    // to Squirrel
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
    // from Squirrel
    {
        std::map<std::string, rect_f> m = *run<std::map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
        REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
    }
    /*
    {
        std::map<i32, rect_f> m = *run<std::map<i32, rect_f>>("return [{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}]");
        REQUIRE(m[1] == (rect_f{0, 1, 2, 3}));
        REQUIRE(m[2] == (rect_f{4, 3, 2, 1}));
    }
    */
    {
        std::unordered_map<std::string, rect_f> m = *run<std::unordered_map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
        REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
        REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
    }
    /*
    {
        std::unordered_map<i32, rect_f> m = *run<std::unordered_map<i32, rect_f>>("return [{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}]");
        REQUIRE(m[1] == (rect_f{0, 1, 2, 3}));
        REQUIRE(m[2] == (rect_f{4, 3, 2, 1}));
    }
    */
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Optional")
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
        i32 i = *run<i32>("return test.Optional(100, \"hurray\")");
        REQUIRE(i == 6);
    }
    {
        f32 f = *run<f32>("return test.Optional2(100, 2.5)");
        REQUIRE(f == Optional2(100, std::nullopt, 2.5f));
    }
    {
        f32 f = *run<f32>("return test.Optional2(100,\"hurray\", 2.5)");
        REQUIRE(f == Optional2(100, "hurray", 2.5f));
    }
    {
        std::optional<f32> f = *run<std::optional<f32>>("return 10.25");
        REQUIRE(f.has_value());
        REQUIRE(f == 10.25f);
    }
    {
        auto f = run<std::optional<f32>>("return \"ok\"");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
    {
        auto f = run<std::optional<f32>>("return");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Overloads")
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

        auto res = run<f32>("return overload([1, 2, 3, 4, 5])");
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

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Results")
{
    {
        auto x = run("function return 1 edn", "bad script");
        REQUIRE(x.error() == error_code::Error);
    }
    {
        auto res = run("x <- \"ok\"");
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
        auto res = run("arrayX <- [1,2,3,\"a\"]");
        REQUIRE(res);
        auto tab = global.get<std::vector<i32>>("arrayX");
        REQUIRE(tab.error() == error_code::TypeMismatch);
    }

    {
        auto res = run<i32>("return \"ok\"");
        REQUIRE(res.error() == error_code::TypeMismatch);
        REQUIRE(res.value_or(200) == 200);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Run")
{
    SUBCASE("get int")
    {
        {
            char const* source = R"-(
            return 1042
        )-";

            auto result = run<i64>(source);
            REQUIRE_FALSE(result.has_error());
            REQUIRE(result.value() == 1042);
        }
        {
            char const* source = R"-(
            function f() { return 521 }
            return f()
        )-";

            auto result = run<i64>(source);
            REQUIRE_FALSE(result.has_error());
            REQUIRE(result.value() == 521);
        }
    }
    SUBCASE("get double")
    {
        char const* source = R"-(
            return 1.42
        )-";

        auto result = run<f64>(source);
        REQUIRE_FALSE(result.has_error());
        REQUIRE(result.value() == Approx(1.42));
    }
    SUBCASE("get string")
    {
        char const* source = R"-(
            return "ok"
        )-";

        auto result = run<std::string>(source);
        REQUIRE_FALSE(result.has_error());
        REQUIRE(result.value() == "ok");
    }
    SUBCASE("get bool")
    {
        char const* source = R"-(
            return true
        )-";

        auto result = run<bool>(source);
        REQUIRE_FALSE(result.has_error());
        REQUIRE(result.value() == true);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.RunAsync")
{
    {
        std::string script = R"-(
            function fibo(n) {
                function inner(m) {
                    if (m < 2) return m;
                    return inner(m - 1) + inner(m - 2);
                }
                return inner(n);
            }
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
            function fibo(n) {
                function inner(m) {
                    if (m < 2) return m;
                    return inner(m - 1) + inner(m - 2);
                }
                return inner(n);
            }
            return fibo(10);
        )-";

        io::delete_file("asynctest.nut");
        io::ofstream stream {"asynctest.nut"};
        stream.write(script);
        stream.flush();
        auto ftr = run_file_async<i64>("asynctest.nut");
        ftr.wait();
        auto res = ftr.get();
        REQUIRE(res);
        REQUIRE(res.value() == 55);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.STLTypes")
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
    /*
        std::function testFuncTuple = [](double d) {
            return std::tuple(d * 5, std::to_string(d));
        };
    */
    std::function testFuncMap = []() {
        return std::map<std::string, i32> {
            {"abc", 123},
            {"def", 234}};
    };
    std::function testFuncUMap = []() {
        return std::unordered_map<std::string, i32> {
            {"abc", 123}, {"def", 234}};
    };

    // global["test"]["Tuple"]    = testFuncTuple;
    global["test"]["Map"]      = &testFuncMap;
    global["test"]["UMap"]     = &testFuncUMap;
    global["test"]["Vector"]   = &testFuncVector;
    global["test"]["Array"]    = &testFuncArray;
    global["test"]["PairPara"] = &testFuncPairPara;

    /*
        SUBCASE("tuple return from cpp")
        {
            auto res = run("a <- test.Tuple(5.22)");
            REQUIRE(res);
            double      a = global["a"];
            std::string b = global["b"];
            REQUIRE(a == 5.22 * 5);
            REQUIRE(b == std::to_string(5.22));
        }
        */
    SUBCASE("map return from cpp")
    {
        auto res = run("x <- test.Map()");
        REQUIRE(res);
        std::map<std::string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("unordered_map return from cpp")
    {
        auto res = run("x <- test.UMap()");
        REQUIRE(res);
        std::unordered_map<std::string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("vector return from cpp")
    {
        auto res = run("x <- test.Vector();");
        REQUIRE(res);
        std::vector<std::string> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("array return from cpp")
    {
        auto res = run("x <- test.Array()");
        REQUIRE(res);
        std::array<std::string, 5> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("vector parameter")
    {
        auto res = run("function foo(x) {return x[1] * x[3]} ");
        REQUIRE(res);
        std::vector   vec {1, 2, 3, 4, 5};
        function<i32> func = global["foo"];
        i32           a    = func(vec);
        REQUIRE(a == 2 * 4);

        std::array arr {1, 2, 3, 4, 5};
        a = func(arr);
        REQUIRE(a == 2 * 4);
    }
    /*
    SUBCASE("tuple parameter")
    {
        auto res = run(
            "function foo(x) "
            "   if (x[2])  return x[0] * x[1]; else return 10; "
            "end ");
        REQUIRE(res);
        auto          tup  = std::make_tuple(4, 2, true);
        function<i32> func = global["foo"];
        i32           a    = func(tup);
        REQUIRE(a == 4 * 2);
    }*/
    SUBCASE("pair parameter")
    {
        auto res = run("function foo(x) {return x[0] * x[1]} ");
        REQUIRE(res);
        auto          tup  = std::make_pair(4, 2.4f);
        function<f32> func = global["foo"];
        f32           a    = func(tup);
        REQUIRE(a == 4 * 2.4f);
    }
    SUBCASE("map parameter")
    {
        auto res = run("function foo(x) {return x.test} ");
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
        std::string a                = *run<std::string>("return foo[0] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == "123");
    }
    SUBCASE("get/set deque")
    {
        std::deque<std::string> deck = {"test", "123"};
        global["foo"]                = deck;
        std::string a                = *run<std::string>("return foo[0] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == "123");
    }
    SUBCASE("get/set span")
    {
        std::vector<std::string> vec {"test", "123"};
        std::span<std::string>   s {vec.data(), vec.size()};
        global["foo"] = s;
        std::string a = *run<std::string>("return foo[0] ");
        REQUIRE(a == vec[0]);
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == vec[1]);
    }
    SUBCASE("get map")
    {
        auto res = run("rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} ");
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
        std::pair<std::string, i32> x = *run<std::pair<std::string, i32>>("return [\"ok\", 10]");
        REQUIRE(x.first == "ok");
        REQUIRE(x.second == 10);
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
        std::set<i32> set = *run<std::set<i32>>("return [1, 2, 3, 1, 2, 3, 4, 2] ");
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

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Table")
{
    SUBCASE("table as parameter")
    {
        {
            i32           x {0};
            std::function func {[&](table& tab0) {
                REQUIRE(tab0.has("x"));
                x = tab0["x"];
            }};

            global["func"] = &func;
            auto res       = run("local tab = { x = 42 }; func(tab);");
            REQUIRE(res);
            REQUIRE(x == 42);
        }
        {
            i32           x {0};
            std::function func {[&](stack_base& root, table& tab) {
                REQUIRE(root.has("y"));
                x += root["y"].as<i32>();
                REQUIRE(tab.has("x"));
                x += tab["x"].as<i32>();
            }};

            global["func"] = &func;
            auto res       = run("y <- 100; local tab = { x = 42 }; func(tab);");
            REQUIRE(res);
            REQUIRE(x == 142);
        }
    }
    SUBCASE("basic operations")
    {
        {
            auto res                             = run("tableX <- { }");
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            i32 x                                = global["tableX"]["a"]["b"]["c"]["d"];
            REQUIRE(res);
            REQUIRE(x == 100);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            i32  x   = *global.get<i32>("tableX", "top", "x");
            REQUIRE(res);
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);

            i32 x {0};
            REQUIRE(global["tableX"]["top"].as<table>().try_get<i32>(x, "x"));
            REQUIRE(x == 10);

            REQUIRE_FALSE(global.try_get<i32>(x, "x"));
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
            table tab = *run<table>("return {a = 2.4, b = true, c = \"hello\"} ");
            REQUIRE(tab.is<f32>("a"));
            REQUIRE(tab["b"].is<bool>());
            REQUIRE(tab.is<std::string>("c"));
        }
        {
            table tab = *run<table>("return { a = 2.4, b = \"ok\" } ");
            REQUIRE(tab["a"].get<bool>().value_or(false) == false);
            REQUIRE(tab["b"].get<std::string>().value_or("default") == "ok");
        }

        {
            table                    tab = *run<table>("local tab = { a = 2.4, b = true, c = \"hello\" }; tab[1] <- 42; return tab ");
            std::vector<std::string> vect {"a", "b", "c"};
            std::vector<std::string> keys = tab.get_keys<std::string>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            table            tab = *run<table>("local tab = { a = 22 }; tab[1] <- \"a\"; tab[2] <- 3; tab[3] <- 55; return tab");
            std::vector<i32> vect {1, 2, 3};
            std::vector<i32> keys = tab.get_keys<i32>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            using var            = std::variant<i32, std::string>;
            table            tab = *run<table>("local tab = {a = 2.4, c = \"hello\"}; tab[1] <- 42; return tab ");
            std::vector<var> vect {1, "a", "c"};
            auto             keys = tab.get_keys<var>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }

        {
            auto res = run("rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} ");
            REQUIRE(res);
            table tab = global["rectF"];
            f32   f   = tab["x"];
            REQUIRE(f == 2.7f);
        }
        {
            auto res = run(
                "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} "
                "function tabletest(x) { return x.y}");
            REQUIRE(res);
            table tab          = global["rectF"];
            tab["y"]           = 100.5f;
            function<f32> func = global["tabletest"];
            f32           x    = func(tab);
            REQUIRE(x == 100.5f);
            REQUIRE((f32)tab["y"] == 100.5f);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table top = global["tableX"]["top"];
            i32   x   = top["x"];
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table   tab = global["tableX"];
            point_i top = tab["top"];
            REQUIRE(top.X == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table tab = global["tableX"];
            i32   top = tab["top"]["x"];
            REQUIRE(top == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            table tab       = global["tableX"];
            tab["top"]["x"] = 400;
            i32 top         = global["tableX"]["top"]["x"];
            REQUIRE(top == 400);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            table tab               = global["tableX"];
            tab["a"]["b"]["c"]["d"] = 42;
            i32 top                 = global["tableX"]["a"]["b"]["c"]["d"];
            REQUIRE(top == 42);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            table tab = global["tableX"];
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 2);
            res = run(
                "tableX.a.b.c.d = 4");
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 4);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
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
            auto res = run("tableX <- { a={ b={ bb = \"ok\", c={ d=2 } } } }");
            REQUIRE(res);
            table tab                            = global["tableX"];
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            REQUIRE((i32)global["tableX"]["a"]["b"]["c"]["d"] == 100);
            REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 100);
            std::string x = tab["a"]["b"]["bb"];
            REQUIRE(x == "ok");
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            table tab {global["tableX"]};
            table subt {};
            REQUIRE_FALSE(subt.is_valid());
            tab["sub"] = subt;
            REQUIRE(subt.is_valid());
            subt["x"] = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            table tab {global["tableX"]};
            tab["sub"]      = table {};
            tab["sub"]["x"] = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            table tab {global["tableX"]};
            table subt;
            tab["sub"] = subt;
            subt["x"]  = 42;

            REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
        }
        {
            table tab0 = *run<table>("tableX <- {  }; tableY <- {  }; return tableX;");
            table tab1 {global["tableX"]};
            REQUIRE(tab0 == tab1);
            table tab2 {global["tableY"]};
            REQUIRE(tab0 != tab2);
        }
        {
            table tab = *run<table>("local tableX = { a = 12 , b = { c = 100 }}; return tableX;");
            REQUIRE(tab.is<table>("b"));
            REQUIRE((i32)tab["b"]["c"] == 100);
        }
    }
    SUBCASE("metamethods")
    {
        std::function func = [](table const&) {
            return "hello world";
        };

        auto meta = run<table>(
                        "tab  <- { }; "
                        "meta <- { }; "
                        "tab.setdelegate(meta); "
                        "return meta ")
                        .value();

        meta["_tostring"] = &func;

        auto res = run<std::string>("return tab.tostring()");
        REQUIRE(res.has_value());
        REQUIRE(res.value().starts_with("hello world"));
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.TcobTypes")
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
            "rectI <- {x=3, y=6, width=10, height=20} "
            "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} "
            "color <- { r= 1, g = 2, b = 3, a = 1} "
            "pointI <- { x = 20, y = 400 } "
            "pointF <- { x = 4.5, y = 3.23 } ");
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
            "rectI <- {x=3, y=6, width=10, height=20}; "
            "rectI = test.RectI(rectI); "

            "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2}; "
            "rectF = test.RectF(rectF); "

            "color <- { r = 1, g = 2, b = 3, a = 1 }; "
            "color = test.Color(color);"

            "pointI <- { x = 20, y = 400 }; "
            "pointI = test.PointI(pointI); "

            "pointF <- { x = 4.5, y = 3.23 }; "
            "pointF = test.PointF(pointF); "

            "sizeI <- { width = 20, height = 400 }; "
            "sizeI = test.SizeI(sizeI); "

            "rectIS  <- { x=3,   y=6,   width=10,  height=20   }; "
            "rectFS  <- { x=2.7, y=3.1, width=2.3, height=55.2 }; "
            "pointIS <- { x=20,  y=400  }; "
            "pointFS <- { x=4.5, y=3.23 }; "

            "degree <- 160;");
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
            "rectF <- { x=2.7, y=3.1, width=2.3, height=55.2} "
            "color <- { r = 1, g = 2, b = 3, a = 1} "
            "pointI <- { x = 20, y = 400 } "
            "x <- test.Mix(100, rectF, color, \"Hello\", false, pointI)");
        REQUIRE(res);
        f32 x = global["x"];

        REQUIRE(x == testFuncMix(100, rect_f(2.7f, 3.1f, 2.3f, 55.2f), color(1, 2, 3, 1), "Hello", false, point_i(20, 400)));
    }
    SUBCASE("error checking")
    {
        auto res = run(
            "rectF <- { x=2.7, y=3.1, width=\"hello\", height=true } "
            "color <- { r = \"red\", g = \"green\", b = \"blue\", a = \"aqua\" } "
            "pointI <- { x = \"1\", y = \"400\" } ");
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

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Threads")
{
    {
        auto res = run(R"(
                function coroutine_test()
                {
                    ::suspend("1");
                    ::suspend("2");
                }

                 coro <- ::newthread(coroutine_test);
        )");

        REQUIRE(res);
        REQUIRE(global.is<thread>("coro"));
        thread t = global["coro"];
        REQUIRE(t.get_status() == vm_view::status::Idle);
        auto cres = t.call<std::string>();
        REQUIRE(t.get_status() == vm_view::status::Suspended);
        REQUIRE(cres);
        REQUIRE(cres.value() == "1");
        auto wres = t.wake_up<std::string>();
        REQUIRE(wres);
        REQUIRE(wres.value() == "2");
    }
    {
        auto res = run(R"(
                function coroutine_test(x)
                {
                    ::suspend(x+"1");
                    ::suspend(x+"2");
                }

                 coro <- ::newthread(coroutine_test);
        )");

        REQUIRE(res);
        REQUIRE(global.is<thread>("coro"));
        thread t = global["coro"];
        REQUIRE(t.get_status() == vm_view::status::Idle);
        auto cres = t.call<std::string>("a");
        REQUIRE(t.get_status() == vm_view::status::Suspended);
        REQUIRE(cres);
        REQUIRE(cres.value() == "a1");
        auto wres = t.wake_up<std::string>("a");
        REQUIRE(wres);
        REQUIRE(wres.value() == "a2");
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.TypeCoercion")
{
    SUBCASE("string from int")
    {
        auto res = run("a <- 100 ");
        REQUIRE(res);
        REQUIRE(global.is<i32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        std::string val = global["a"];
        REQUIRE(val == "100");
    }
    SUBCASE("string from number")
    {
        auto res = run("a <- 100.5 ");
        REQUIRE(res);
        REQUIRE(global.is<f32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        std::string val = global["a"];
        REQUIRE(val == "100.500000");
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.UserDefinedConversion")
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
        auto res = run("foo <- {x=3,y=2,z=1}");
        REQUIRE(res);
        REQUIRE(global.is<foo>("foo"));
        res = run("foo = {x=3,n=2,z=1}");
        REQUIRE_FALSE(global.is<foo>("foo"));
    }
    {
        auto res = run("function bar(p) {return p.x * p.y * p.z} ");
        REQUIRE(res);
        function<i32> func = global["bar"];
        i32           a    = *func.call(foo {1, 2, 3});
        REQUIRE(a == 6);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.VariadicFunctions")
{
    SUBCASE("variadic Squirrel function")
    {
        auto res = run(
            R"(
            function testArg(...) {
                local retValue = 0;
                foreach(i, v in vargv) {retValue += v;}
                return retValue;
            }
        )");

        REQUIRE(res);
        function<i32> func = global["testArg"];
        i32           a    = func(1, 2, 3, 4, 5, 6);
        REQUIRE(a == (1 + 2 + 3 + 4 + 5 + 6));
    }
}
