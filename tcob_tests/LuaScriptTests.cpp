#include "LuaScriptTestsClass.hpp"
#include "LuaScriptTestsHelper.hpp"
#include "tests.hpp"

class LuaScriptTests : public Script {
public:
    LuaScriptTests()
    {
        global = global_table();
        open_libraries();
        register_searcher(&openrequire);
    }

    Table global;
};

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.TableDumper")
{
    {
        auto res = run_script(
            "tableX = { 2.7, 5, 6, a = 69, 7, 8, x = 10, t = { a = 20, 30.2 } }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];

        stringstream fs;
        fs << "tab = ";
        tab.dump(fs);
        fs << "\nreturn tab";

        Table tab2 = run_script<Table>(fs.str());

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
        auto res = run_script(
            "tableX = { left = 2.7, x = 10, t = { a = 20, y = 30.2, m = { z = 1, f = 3 } }, y = true, z = 'ok' }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];

        stringstream fs;
        fs << "tab = ";
        tab.dump(fs);
        fs << "\nreturn tab";

        Table tab2 = run_script<Table>(fs.str());

        REQUIRE((f32)tab2["left"] == 2.7f);
        REQUIRE((f32)tab["left"] == (f32)tab2["left"]);

        REQUIRE((bool)tab2["y"] == true);
        REQUIRE((bool)tab["y"] == (bool)tab2["y"]);

        REQUIRE(tab2.get<string>("z").Value == "ok");
        REQUIRE(tab.get<string>("z").Value == tab2.get<string>("z").Value);

        REQUIRE((i32)tab2["t"]["a"] == 20);
        REQUIRE((i32)tab["t"]["a"] == (i32)tab2["t"]["a"]);

        REQUIRE((i32)tab2["t"]["m"]["z"] == 1);
        REQUIRE((i32)tab["t"]["m"]["z"] == (i32)tab2["t"]["m"]["z"]);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Coroutines")
{
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,10 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(global.is<Coroutine>("co"));
        Coroutine co = global["co"];
        REQUIRE(co.resume<i32>().Value == 1);
        REQUIRE(co.resume<i32>().Value == 2);
        REQUIRE(co.resume<i32>().Value == 3);
    }
    {
        auto res = run_script("co = coroutine.create(function (x) "
                              "       for i=1,10 do "
                              "         coroutine.yield(i*x) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];
        REQUIRE(co.resume<i32>(2).Value == 2);
        REQUIRE(co.resume<i32>().Value == 4);
        REQUIRE(co.resume<i32>().Value == 6);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "       return 1000 "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 1);
        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 2);
        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Ok);
        REQUIRE(result.Value == 1000);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 1);
        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 2);
        auto endresult = co.resume<void>();
        REQUIRE(endresult.State == ResultState::Ok);
        auto endresult2 = co.resume<void>();
        REQUIRE(endresult2.State == ResultState::RuntimeError);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        REQUIRE(co.current_state() == CoroutineState::Ok);
        auto result = co.resume<i32>();
        REQUIRE(co.current_state() == CoroutineState::Suspended);
        result = co.resume<i32>();
        REQUIRE(co.current_state() == CoroutineState::Suspended);
        result = co.resume<i32>();
        REQUIRE(co.current_state() == CoroutineState::Ok);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 1);
        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 2);
        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::TypeMismatch);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i,i+0.5) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<pair<i32, f32>>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == make_pair(1, 1.5f));
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<pair<i32, f32>>();
        REQUIRE(result.State == ResultState::TypeMismatch);
    }
    {
        auto res = run_script("co = coroutine.create(function () "
                              "       for i=1,2 do "
                              "         coroutine.yield(i) "
                              "       end "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 1);

        auto coresult = co.close();
        REQUIRE(coresult == CoroutineState::Ok);
        REQUIRE(co.current_state() == CoroutineState::Ok);

        result = co.resume<i32>();
        REQUIRE(result.State == ResultState::RuntimeError);
    }
    {
        auto l = function([](i32 i) { return (f32)i * 2.5f; });

        auto res = run_script("co = coroutine.create(function () "
                              "         coroutine.yield(100) "
                              "     end) ");
        Coroutine co = global["co"];

        auto result = co.resume<i32>();
        REQUIRE(result.State == ResultState::Yielded);
        REQUIRE(result.Value == 100);
        static_cast<void>(co.resume<void>());
        REQUIRE(co.current_state() == CoroutineState::Ok);

        co.push(l);
        auto result2 = co.resume<f32>(15);
        REQUIRE(result2.State == ResultState::Ok);
        REQUIRE(result2.Value == l(15));
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Enums")
{
    enum class testEnum {
        True,
        False,
        FileNotFound
    };
    function testFuncEnum = [](testEnum numnum) {
        return numnum;
    };

    global["test"]["Enum"] = testFuncEnum;
    {
        Function<testEnum> func = global["test"]["Enum"];
        testEnum num = func.call(testEnum::FileNotFound);
        REQUIRE(num == testEnum::FileNotFound);
    }
    {
        unordered_map<string, testEnum> map = {
            { "True", testEnum::True },
            { "False", testEnum::False },
            { "FileNotFound", testEnum::FileNotFound },
        };

        global["testEnum"] = map;
        testEnum num = run_script<testEnum>(
            "enum = testEnum.False "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::False);
        num = run_script<testEnum>(
            "enum = testEnum.True "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::True);
        num = run_script<testEnum>(
            "enum = testEnum.FileNotFound "
            "return test.Enum(enum)");
        REQUIRE(num == testEnum::FileNotFound);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Results")
{
    {
        auto x = run_script("function return 1 edn");
        REQUIRE(x.State == ResultState::SyntaxError);
    }
    {
        auto res = run_script("x = 'ok'");
        REQUIRE(res.State == ResultState::Ok);
        auto f = global.get<f32>("x");
        REQUIRE(f.State == ResultState::TypeMismatch);
        f = global.get<f32>("testX");
        REQUIRE(f.State == ResultState::Undefined);
        auto s = global.get<string>("x");
        REQUIRE(s.State == ResultState::Ok);
        REQUIRE(s.Value == "ok");
    }
    {
        auto res = run_script(
            "tableX = {1,2,3,'a'}");
        auto tab = global.get<vector<i32>>("tableX");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(tab.State == ResultState::TypeMismatch);
        REQUIRE(tab.Value == vector<i32>({ 1, 2, 3 }));
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.UserDefinedConversion")
{
    function Foo = [](const foo& f) {
        return f.x + f.y + f.z;
    };
    global["test"]["Foo"] = Foo;

    {
        foo i = run_script<foo>("return {x=3,y=2,z=1}");
        REQUIRE(i.x == 3);
    }
    {
        i32 i = run_script<i32>("return test.Foo({x=3,y=2,z=1})");
        REQUIRE(i == Foo(foo { 3, 2, 1 }));
    }
    {
        auto res = run_script("foo = {x=3,y=2,z=1}");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(global.is<foo>("foo"));
        res = run_script("foo = {x=3,n=2,z=1}");
        REQUIRE_FALSE(global.is<foo>("foo"));
    }
    {
        auto res = run_script(
            "function bar(p) "
            "return p.x * p.y * p.z "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["bar"];
        i32 a = func.call(foo { 1, 2, 3 });
        REQUIRE(a == 6);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Optional")
{
    function Optional = [](i32 i, optional<string> str) -> isize {
        if (str.has_value()) {
            return str.value().length();
        }

        return i;
    };
    function Optional2 = [](i32 i, optional<string> str, f32 f) -> f32 {
        if (str.has_value()) {
            return str.value().length() * f;
        }

        return i * f;
    };

    global["test"]["Optional"] = Optional;
    global["test"]["Optional2"] = Optional2;

    {
        i32 i = run_script<i32>(
            "return test.Optional(100)");
        REQUIRE(i == Optional(100, nullopt));
    }
    {
        i32 i = run_script<i32>(
            "return test.Optional(100, 'hurray')");
        REQUIRE(i == 6);
    }
    {
        f32 f = run_script<f32>(
            "return test.Optional2(100, 2.5)");
        REQUIRE(f == Optional2(100, nullopt, 2.5f));
    }
    {
        f32 f = run_script<f32>(
            "return test.Optional2(100,'hurray', 2.5)");
        REQUIRE(f == Optional2(100, "hurray", 2.5f));
    }
    {
        optional<f32> f = run_script<optional<f32>>(
            "return 10.25");
        REQUIRE(f.has_value());
        REQUIRE(f == 10.25f);
    }
    {
        optional<f32> f = run_script<optional<f32>>(
            "return \"ok\"");
        REQUIRE_FALSE(f.has_value());
    }
    {
        optional<f32> f = run_script<optional<f32>>(
            "return");
        REQUIRE_FALSE(f.has_value());
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Map")
{
    // to Lua
    {
        map<string, RectF> m = { { "a", { 0, 1, 2, 3 } }, { "b", { 4, 3, 2, 1 } } };
        global["map"] = m;
        RectF x = run_script<RectF>("return map.a");
        REQUIRE(x == m["a"]);
        RectF y = run_script<RectF>("return map.b");
        REQUIRE(y == m["b"]);
    }
    {
        map<i32, RectF> m = { { 1, { 0, 1, 2, 3 } }, { 2, { 4, 3, 2, 1 } } };
        global["map"] = m;
        RectF x = run_script<RectF>("return map[1]");
        REQUIRE(x == m[1]);
        RectF y = run_script<RectF>("return map[2]");
        REQUIRE(y == m[2]);
    }
    {
        unordered_map<string, RectF> m = { { "a", { 0, 1, 2, 3 } }, { "b", { 4, 3, 2, 1 } } };
        global["map"] = m;
        RectF x = run_script<RectF>("return map.a");
        REQUIRE(x == m["a"]);
        RectF y = run_script<RectF>("return map.b");
        REQUIRE(y == m["b"]);
    }
    {
        unordered_map<i32, RectF> m = { { 1, { 0, 1, 2, 3 } }, { 2, { 4, 3, 2, 1 } } };
        global["map"] = m;
        RectF x = run_script<RectF>("return map[1]");
        REQUIRE(x == m[1]);
        RectF y = run_script<RectF>("return map[2]");
        REQUIRE(y == m[2]);
    }
    // from Lua
    {
        map<string, RectF> m = run_script<map<string, RectF>>("return {a={0,1,2,3},b={4,3,2,1}}");
        REQUIRE(m["a"] == (RectF { 0, 1, 2, 3 }));
        REQUIRE(m["b"] == (RectF { 4, 3, 2, 1 }));
    }
    {
        map<i32, RectF> m = run_script<map<i32, RectF>>("return {{0,1,2,3},{4,3,2,1}}");
        REQUIRE(m[1] == (RectF { 0, 1, 2, 3 }));
        REQUIRE(m[2] == (RectF { 4, 3, 2, 1 }));
    }
    {
        unordered_map<string, RectF> m = run_script<unordered_map<string, RectF>>("return {a={0,1,2,3},b={4,3,2,1}}");
        REQUIRE(m["a"] == (RectF { 0, 1, 2, 3 }));
        REQUIRE(m["b"] == (RectF { 4, 3, 2, 1 }));
    }
    {
        unordered_map<i32, RectF> m = run_script<unordered_map<i32, RectF>>("return {{0,1,2,3},{4,3,2,1}}");
        REQUIRE(m[1] == (RectF { 0, 1, 2, 3 }));
        REQUIRE(m[2] == (RectF { 4, 3, 2, 1 }));
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Variant")
{
    function Variant = [](variant<f32, string, bool> var) {
        if (auto* idx = get_if<f32>(&var))
            return "f32";
        else if (auto* idx = get_if<string>(&var))
            return "string";
        else if (auto* idx = get_if<bool>(&var))
            return "bool";

        return "";
    };
    global["test"]["Variant"] = Variant;

    SECTION("cpp parameter")
    {
        string str = run_script<string>(
            "return test.Variant('hi')");
        REQUIRE(str == Variant("hi"s));
        str = run_script<string>(
            "return test.Variant(1.23)");
        REQUIRE(str == Variant(1.23f));
        str = run_script<string>(
            "return test.Variant(true)");
        REQUIRE(str == Variant(true));
    }
    SECTION("lua parameter")
    {
        auto res = run_script(
            "function foo(x) "
            "return x * 10 "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        variant<string, i32, bool> var = 100;
        Function<i32> func = global["foo"];
        i32 a = func(var);
        REQUIRE(a == 1000);
    }
    SECTION("return value")
    {
        {
            const auto&& var = run_script<variant<string, i16, bool>>("return 100").Value;
            REQUIRE(get<i16>(var) == 100);
        }
        {
            const auto&& var = run_script<variant<string, u64, bool>>("return 100").Value;
            REQUIRE(get<u64>(var) == 100);
        }
        {
            const auto&& var = run_script<variant<int, vector<string>, bool>>("return {'ok','ko'}").Value;
            REQUIRE(get<vector<string>>(var) == vector<string> { "ok", "ko" });
        }
        {
            const auto&& var = run_script<variant<int, vector<int>, bool>>("return {1,2,3}").Value;
            REQUIRE(get<vector<int>>(var) == vector<int> { 1, 2, 3 });
        }
        {
            const auto&& var = run_script<variant<int, vector<bool>, bool>>("return {true,false,true,false,true}").Value;
            REQUIRE(get<vector<bool>>(var) == vector<bool> { true, false, true, false, true });
        }
        {
            const auto&& var = run_script<variant<vector<int>, vector<string>, int>>("return {'ok','ko'}").Value;
            REQUIRE(get<vector<string>>(var) == vector<string> { "ok", "ko" });
        }
        {
            const auto&& var = run_script<variant<vector<int>, vector<string>, int>>("return {1,2,3}").Value;
            REQUIRE(get<vector<int>>(var) == vector<int> { 1, 2, 3 });
        }
        {
            const auto&& var = run_script<variant<vector<bool>, PointF, int>>("return {1,2}").Value;
            REQUIRE(get<PointF>(var) == PointF { 1, 2 });
        }
        {
            const auto&& var = run_script<variant<vector<bool>, SizeF, int>>("return {1,2}").Value;
            REQUIRE(get<SizeF>(var) == SizeF { 1, 2 });
        }
        {
            const auto&& var = run_script<variant<vector<bool>, RectF, int>>("return {1,2,3,4}").Value;
            REQUIRE(get<RectF>(var) == RectF { 1, 2, 3, 4 });
        }
        {
            const auto&& var = run_script<variant<vector<bool>, Color, int>>("return {r=1,g=2,b=4}").Value;
            REQUIRE(get<Color>(var) == Color { 1, 2, 4, 255 });
        }
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Table")
{
    {
        auto res = run_script(
            "tableX = { }");
        global["tableX"]["a"]["b"]["c"]["d"] = 100;
        i32 x = global["tableX"]["a"]["b"]["c"]["d"];
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(x == 100);
    }
    {
        auto res = run_script(
            "tableX = {left=2.7, top={x=10,y=2} }");
        i32 x = global.get<i32>("tableX", "top", "x");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(x == 10);
    }
    {
        auto res = run_script(
            "tableX = {1,{x=1,y=2} }");
        i32 y = global.get<i32>("tableX", 2, "y");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(y == 2);
    }
    {
        auto res = run_script(
            "tableX = {1,{x=1,y=2} }");
        global.set("tableX", 2, "y", 200);
        i32 y = global.get<i32>("tableX", 2, "y");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(y == 200);
    }
    {
        auto res = run_script(
            "tableX = {1,{x=1,y=2} }");
        i32 y = global["tableX"][2]["y"];
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(y == 2);
    }
    {
        auto res = run_script(
            "tableX = {1,{x=1,y=2} }");
        global["tableX"][2]["y"] = 200;
        i32 y = global.get<i32>("tableX", 2, "y");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(y == 200);
    }
    {
        Table tab = run_script<Table>("return {4,5,2,1} ");
        i32 x = tab[1];
        REQUIRE(x == 4);
        x = tab[2];
        REQUIRE(x == 5);
        x = tab[3];
        REQUIRE(x == 2);
        x = tab[4];
        REQUIRE(x == 1);
    }
    {
        auto res = run_script("tab = {4,5,2,1} ");
        REQUIRE(res.State == ResultState::Ok);
        {
            Table tab1 = global["tab"];
            tab1[1] = 100;
            REQUIRE((i32)tab1[1] == 100);
        }
        {
            Table tab1 = global["tab"];
            tab1[1] = 100;
            REQUIRE((i32)tab1[1] == 100);
        }
    }
    {
        Table tab = run_script<Table>(
            "return {left=2.7, top=3.1, width=2.3, height=55.2} ");
        f32 f = tab["top"];
        REQUIRE(f == 3.1f);
    }
    {
        Table tab = run_script<Table>(
            "return {left=2.7, top=3.1, width=2.3, height=55.2} ");
        REQUIRE(tab.has("left"));
        REQUIRE(tab.has("top"));
        REQUIRE(tab.has("width"));
        REQUIRE(tab.has("height"));
    }
    {
        Table tab = run_script<Table>(
            "return {a = 2.4, b = true, c = 'hello'} ");
        REQUIRE(tab.is<f32>("a"));
        REQUIRE(tab.is<bool>("b"));
        REQUIRE(tab.is<string>("c"));
    }
    {
        Table tab = run_script<Table>(
            "return {a = 2.4, b = true, c = 'hello', 42} ");
        vector<string> vect { "a", "b", "c" };
        vector<string> keys = tab.keys<string>();
        sort(keys.begin(), keys.end());
        REQUIRE(keys == vect);
    }
    {
        Table tab = run_script<Table>(
            "return { 'a', 3, 55, a = 22 }");
        vector<i32> vect { 1, 2, 3 };
        vector<i32> keys = tab.keys<i32>();
        sort(keys.begin(), keys.end());
        REQUIRE(keys == vect);
    }
    {
        Table tab = run_script<Table>(
            "return {a = 2.4, 3, c = 'hello'} ");
        vector<variant<i32, string>> vect { 1, "a", "c" };
        auto keys = tab.keys<variant<i32, string>>();
        sort(keys.begin(), keys.end());
        REQUIRE(keys == vect);
    }
    {
        auto res = run_script(
            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} ");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["rectF"];
        f32 f = tab["left"];
        REQUIRE(f == 2.7f);
    }
    {
        auto res = run_script(
            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} "
            "function tabletest(x) "
            "return x.top "
            "end");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["rectF"];
        tab["top"] = 100.5f;
        Function<f32> func = global["tabletest"];
        f32 x = func(tab);
        REQUIRE(x == 100.5f);
        REQUIRE((f32)tab["top"] == 100.5f);
    }
    {
        auto res = run_script(
            "tableX = {left=2.7, top={x=10,y=2} }");
        REQUIRE(res.State == ResultState::Ok);
        Table top = global["tableX"]["top"];
        i32 x = top["x"];
        REQUIRE(x == 10);
    }
    {
        auto res = run_script(
            "tableX = {left=2.7, top={x=10,y=2} }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        PointI top = tab["top"];
        REQUIRE(top.X == 10);
    }
    {
        auto res = run_script(
            "tableX = {left=2.7, top={x=10,y=2} }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        i32 top = tab["top"]["x"];
        REQUIRE(top == 10);
    }
    {
        auto res = run_script(
            "tableX = {left=2.7, top={x=10,y=2} }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        tab["top"]["x"] = 400;
        i32 top = global["tableX"]["top"]["x"];
        REQUIRE(top == 400);
    }
    {
        auto res = run_script(
            "tableX = { a={ b={ c={ d=2 } } } }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        tab["a"]["b"]["c"]["d"] = 42;
        i32 top = global["tableX"]["a"]["b"]["c"]["d"];
        REQUIRE(top == 42);
    }
    {
        auto res = run_script(
            "tableX = { a={ b={ c={ d=2 } } } }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 2);
        res = run_script(
            "tableX.a.b.c.d = 4");
        REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 4);
    }
    {
        auto res = run_script(
            "tableX = { a={ b={ c={ d=2 } } } }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"]["a"]["b"]["c"];
        REQUIRE((i32)tab["d"] == 2);
        res = run_script(
            "tableX.a.b.c.d = 4");
        REQUIRE((i32)tab["d"] == 4);
        tab = global["tableX"];
        REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 4);
    }
    {
        auto res = run_script(
            "tableX = { a={ b={ bb = 'ok', c={ d=2 } } } }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab = global["tableX"];
        global["tableX"]["a"]["b"]["c"]["d"] = 100;
        REQUIRE((i32)global["tableX"]["a"]["b"]["c"]["d"] == 100);
        REQUIRE((i32)tab["a"]["b"]["c"]["d"] == 100);
        string x = tab["a"]["b"]["bb"];
        REQUIRE(x == "ok");
    }
    {
        auto res = run_script(
            "tableX = {  }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab { global["tableX"] };
        Table subt { tab.create_table("sub") };
        subt["x"] = 42;

        REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
    }
    {
        auto res = run_script(
            "tableX = {  }");
        REQUIRE(res.State == ResultState::Ok);
        Table tab { global["tableX"] };
        Table subt;
        tab["sub"] = subt;
        subt["x"] = 42;

        REQUIRE((i32)global["tableX"]["sub"]["x"] == 42);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Require")
{
    //require tests///////////////////////////////////////////////
    auto res = run_script(
        "a = require 'res/testfile' "
        "b = a.foo() ");
    REQUIRE(res.State == ResultState::Ok);
    i32 x = global["b"];
    REQUIRE(x == 300);
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.IsHas")
{
    //is tests///////////////////////////////////////////////
    {
        auto res = run_script(
            "a = 100 "
            "b = true "
            "c = 10.3 "
            "d = 'hello' "
            "e = { } "
            "f = function(x) return x end "
            "g = { a = 1, b = 2, c = 3 } "
            "h = { 1, 2, 3 } "
            "i = { 1, 2 } "
            "j = { \"a\", \"b\" } ");

        REQUIRE(res.State == ResultState::Ok);

        REQUIRE(global.is<i32>("a"));
        REQUIRE(global.is<bool>("b"));
        REQUIRE(global.is<f32>("c"));
        REQUIRE(global.is<string>("d"));
        REQUIRE(global.is<Table>("e"));
        REQUIRE(global.is<Function<void>>("f"));
        REQUIRE((global.is<map<string, i32>>("g")));
        REQUIRE((global.is<map<i32, i32>>("h")));
        REQUIRE((global.is<vector<i32>>("h")));

        REQUIRE_FALSE(global.is<bool>("a"));
        REQUIRE_FALSE(global.is<string>("a"));
        REQUIRE_FALSE(global.is<Table>("a"));
        REQUIRE_FALSE(global.is<Function<void>>("a"));
        REQUIRE_FALSE((global.is<vector<string>>("h")));
        REQUIRE_FALSE((global.is<map<i32, i32>>("g")));

        REQUIRE_FALSE(global.is<bool>("c"));
        REQUIRE_FALSE(global.is<i32>("c"));

        REQUIRE_FALSE(global.is<bool>("d"));
        REQUIRE_FALSE(global.is<Table>("d"));
        REQUIRE_FALSE(global.is<Function<void>>("d"));
    }
    //has tests///////////////////////////////////////////////
    {
        auto res = run_script(
            "x = { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res.State == ResultState::Ok);
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

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Closures")
{
    function testFuncPrim = [](i32 i, f32 f, double d, bool b) {
        return to_string(i) + to_string(f) + to_string(d) + string(b ? "true" : "false");
    };

    i32 voidTest = 0;
    function testFuncVoid = [&voidTest]() {
        voidTest++;
    };

    global["test"]["Prim"] = testFuncPrim;
    global["test"]["Void"] = testFuncVoid;

    {
        auto res = run_script(
            "str = test.Prim(20, 4.4, 5.22, true)");
        REQUIRE(res.State == ResultState::Ok);
        string str = global["str"];
        REQUIRE(str == testFuncPrim(20, 4.4f, 5.22, true));
    }
    {
        auto res = run_script(
            "test.Void()");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(voidTest == 1);
        res = run_script(
            "test.Void()");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(voidTest == 2);
    }
    {
        global["testFunc"] = testfuncstr;
        string x = run_script<string>(
            "return testFunc()");
        REQUIRE(x == testfuncstr());
    }
    {
        global["testFunc"] = testfuncfloat;
        f32 x = run_script<f32>(
            "return testFunc()");
        REQUIRE(x == testfuncfloat());
    }
    {
        global["testFunc"] = testfuncfloat2;
        f32 x = run_script<f32>(
            "return testFunc(4,4.5,3)");
        REQUIRE(x == testfuncfloat2({ 4.f }, { 4.5f }, 3));
    }
    {
        global["testFunc"] = testfuncpair;
        f32 x = run_script<f32>(
            "return testFunc(4, 6.5)");
        REQUIRE(x == testfuncpair({ 4, 6.5f }));
    }
    {
        auto l = function([](i32 i) { return (f32)i * 2.5f; });
        global["testFunc"] = l;
        f32 x = run_script<f32>(
            "return testFunc(2)");
        REQUIRE(x == 5.0f);
    }
    {
        f32 x;
        auto l = function([&x](i32 i) { x = i * 2.5f; });
        global["testFunc"] = l;
        auto res = run_script("testFunc(2)");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(x == 5.0f);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Functions")
{
    {
        auto res = run_script(
            "function testPoint(p) "
            "return p.x * p.y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["testPoint"];
        i32 a = func.call(PointI { 2, 4 });
        REQUIRE(a == 2 * 4);
        a = func(PointI { 2, 4 });
        REQUIRE(a == 2 * 4);
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function(x) return x*x end ");
        i32 a = func(200);
        REQUIRE(a == 200 * 200);
    }
    {
        auto res = run_script(
            "table = { } "
            "table.func = function() return 50, \"Hello\" end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<pair<i32, string>> func = global["table"]["func"];
        const auto& [a, b] = func.call().Value;

        REQUIRE(a == 50);
        REQUIRE(b == "Hello");
    }
    {
        auto res = run_script(
            "table = { } "
            "table.func = function() return \"Hello\", 100, true end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<tuple<string, i32, bool>> func = global["table"]["func"];
        const auto& [a, b, c] = func.call().Value;

        REQUIRE(a == "Hello");
        REQUIRE(b == 100);
        REQUIRE(c == true);
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 100 end ");
        REQUIRE(func() == 100);
        REQUIRE(100 == func());
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 100 end ");
        REQUIRE_FALSE(func() == 10);
        REQUIRE_FALSE(10 == func());
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 5 end ");
        REQUIRE(func() * 20 == 100);
        REQUIRE(20 * func() == 100);
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 500 end ");
        REQUIRE(func() / 5 == 100);
        REQUIRE(50000 / func() == 100);
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 95 end ");
        REQUIRE(func() + 5 == 100);
        REQUIRE(5 + func() == 100);
    }
    {
        Function<i32> func = run_script<Function<i32>>(
            "return function() return 105 end ");
        REQUIRE(func() - 5 == 100);
        REQUIRE(205 - func() == 100);
    }
    {
        Function<vector<i32>> func = run_script<Function<vector<i32>>>(
            "return function() return {5, 4, 3, 2, 1} end ");
        vector<i32> a = func();
        REQUIRE(a[0] == 5);
        REQUIRE(a[1] == 4);
        REQUIRE(a[2] == 3);
        REQUIRE(a[3] == 2);
        REQUIRE(a[4] == 1);
    }
    {
        Function<map<string, i32>> func = run_script<Function<map<string, i32>>>(
            "return function() return {x=5, y=4, b=3, r=2, aa=1} end ");
        map<string, i32> a = func();
        REQUIRE(a["x"] == 5);
        REQUIRE(a["y"] == 4);
        REQUIRE(a["b"] == 3);
        REQUIRE(a["r"] == 2);
        REQUIRE(a["aa"] == 1);
    }
    {
        auto res = run_script(
            "function testPoint(p) "
            "return p.x * p.y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["testPoint"];
        i32 a = func(PointI { 2, 4 });
        REQUIRE(a == 2 * 4);
    }
    {
        auto res = run_script(
            "function testPoint(p) "
            "return p.x * p.y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["testPoint"];
        PointI p = PointI { 2, 4 };
        i32 a = func(p);
        REQUIRE(a == 2 * 4);
        a = func(PointI { 6, 4 });
        REQUIRE(a == 6 * 4);
        a = func(PointI { 15, 7 });
        REQUIRE(a == 15 * 7);
    }
    {
        auto res = run_script(
            "x = 0 "
            "function testVoid(p) "
            "x = p.x * p.y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<void> func = global["testVoid"];
        func(PointI { 2, 4 });
        i32 x = global["x"];
        REQUIRE(x == 2 * 4);
    }
    {
        auto res = run_script(
            "function testMulti(f,p,r,b) "
            "return f * p.x * r.top "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<f32> func = global["testMulti"];
        f32 x = func(10.4f, PointI { 2, 4 }, RectF { 0, 20, 4, 5 }, true);
        REQUIRE(x == 10.4f * 2 * 20);
    }
    {
        auto res = run_script(
            "function testTable(x,y) "
            "return { a = x, b = y } "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<Table> func = global["testTable"];
        Table tab = func(10, 20);
        REQUIRE((i32)tab["a"] == 10);
        REQUIRE((i32)tab["b"] == 20);
    }
    {
        auto res = run_script(
            "function testTable(x,y) "
            "return x*y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["testTable"];
        REQUIRE(func(10, 20) == 10 * 20);
        REQUIRE(func(20, 40) == 20 * 40);
        {
            OutputFileStream fs { "test.luac" };
            func.dump(fs);
        }

        Function<i32> func2 = load_binary<i32>("test.luac");
        REQUIRE(func2(10, 20) == 10 * 20);
        REQUIRE(func2(20, 40) == 20 * 40);
    }
    {
        Function<string> func = global["string"]["upper"];
        string upper = func("hello");
        REQUIRE(upper == "HELLO");
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.GetSet")
{
    {
        string x;
        auto res = run_script(
            "x = 'ok'");
        REQUIRE(res.State == ResultState::Ok);
        x = global.get<string>("x");
        REQUIRE(x == "ok");
        string y = global["x"];
        REQUIRE(y == "ok");
    }
    {
        const char* x;
        auto res = run_script("x = 'ok'");
        REQUIRE(res.State == ResultState::Ok);
        x = global.get<const char*>("x");
        REQUIRE(strcmp(x, "ok") == 0);
    }
    {
        const char* x { "ok" };
        global["x"] = x;
        const char* y = global["x"];
        REQUIRE(strcmp(x, y) == 0);
    }
    {
        auto res = run_script(
            "x = 1337");
        REQUIRE(res.State == ResultState::Ok);
        i32 x = global["x"];
        REQUIRE(x == 1337);
        global["x"] = 2000;
        x = global["x"];
        REQUIRE(x == 2000);
        x = run_script<i32>("return x");
        REQUIRE(x == 2000);
    }
    {
        auto res = run_script(
            "x = 1337");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(global.has("x"));
        global["x"] = nullptr;
        REQUIRE_FALSE(global.has("x"));
    }
    {
        auto res = run_script(
            "x = { y = { z = 30 } }");
        REQUIRE(res.State == ResultState::Ok);
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 30);
        global["x"]["y"]["z"] = 2000;
        x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    {
        auto res = run_script(
            "x = { y = {  } }");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    {
        REQUIRE_FALSE(global.has("testVar1"));
        global["testVar1"] = 2000;
        REQUIRE(global.has("testVar1"));
        i32 x = global["testVar1"];
        REQUIRE(x == 2000);
    }
    {
        auto res = run_script(
            "x = { y = {  } }");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global.set("x", "y", "z", 2000);
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"];
        REQUIRE(x == 2000);
    }
    {
        REQUIRE_FALSE(global.has("testVar2"));
        global.set("testVar2", 2000);
        REQUIRE(global.has("testVar2"));
        i32 x = global["testVar2"];
        REQUIRE(x == 2000);
    }
    {
        REQUIRE_FALSE(global.has("testVar3"));
        global["testVar2"] = 2000;
        REQUIRE(global.has("testVar2"));
        i32 x = global["testVar2"];
        REQUIRE(x == 2000);
    }
    {
        REQUIRE_FALSE(global.has("testVar4"));
        auto f = global.get<f32>("testVar4");
        REQUIRE(f.State == ResultState::Undefined);
        REQUIRE(f.Value == 0);
    }
    {
        auto res = run_script(
            "x = { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res.State == ResultState::Ok);
        i32 m = global["x"]["z"]["m"];
        REQUIRE(m == 75);
    }
    {
        REQUIRE_FALSE(global.has("testVar"));
        global["testVar"] = 400;
        auto res = run_script(
            "function foo() "
            "return testVar * 10 "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        Function<i32> func = global["foo"];
        i32 a = func();
        REQUIRE(a == 400 * 10);
        global["testVar"] = 2000;
        a = func();
        REQUIRE(a == 2000 * 10);
    }
    {
        auto res = run_script(
            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} ");
        REQUIRE(res.State == ResultState::Ok);
        map<string, f32> rect = global["rectF"];
        REQUIRE(rect["left"] == 2.7f);
    }
    {
        auto res = run_script(
            "x = 30 ");
        REQUIRE(res.State == ResultState::Ok);
        u8 x = global["x"];
        REQUIRE(x == 30);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.Collection")
{
    function testFuncVector = []() {
        return vector<string> { "1", "2", "3", "4", "5" };
    };

    function testFuncArray = []() {
        return array<string, 5> { "1", "2", "3", "4", "5" };
    };

    function testFuncPairPara = [](pair<string, i32> pair) {
        return pair.second;
    };

    function testFuncTuple = [](double d) {
        return tuple(d * 5, to_string(d));
    };

    function testFuncTuplePara = [](tuple<double, string> d) {
        return get<double>(d);
    };

    function testFuncMap = []() {
        return map<string, i32> {
            { "abc", 123 },
            { "def", 234 }
        };
    };
    function testFuncUMap = []() {
        return unordered_map<string, i32> {
            { "abc", 123 }, { "def", 234 }
        };
    };

    global["test"]["Tuple"] = testFuncTuple;
    global["test"]["TuplePara"] = testFuncTuplePara;
    global["test"]["Map"] = testFuncMap;
    global["test"]["UMap"] = testFuncUMap;
    global["test"]["Vector"] = testFuncVector;
    global["test"]["Array"] = testFuncArray;
    global["test"]["PairPara"] = testFuncPairPara;

    {
        const auto& [s, i, b] = run_script<tuple<string, i32, bool>>("return 'ok', 10, true").Value;
        REQUIRE(s == "ok");
        REQUIRE(i == 10);
        REQUIRE(b == true);
    }
    {
        tuple<f64, string> tup = { 4.0, "ok" };
        Function<f64> func = global["test"]["TuplePara"];
        f64 a = func(tup);
        REQUIRE(a == 4.0);
    }
    {
        auto res = run_script(
            "a, b = test.Tuple(5.22)");
        REQUIRE(res.State == ResultState::Ok);
        double a = global["a"];
        string b = global["b"];
        REQUIRE(a == 5.22 * 5);
        REQUIRE(b == to_string(5.22));
    }
    {
        auto res = run_script(
            "x = test.Map()");
        REQUIRE(res.State == ResultState::Ok);
        map<string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    {
        auto res = run_script(
            "x = test.UMap()");
        REQUIRE(res.State == ResultState::Ok);
        unordered_map<string, i32> x = global["x"];
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    {
        auto res = run_script(
            "x = test.Vector()");
        REQUIRE(res.State == ResultState::Ok);
        vector<string> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    {
        auto res = run_script(
            "x = test.Array()");
        REQUIRE(res.State == ResultState::Ok);
        array<string, 5> vec = global["x"];
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    {
        auto res = run_script(
            "function foo(x) "
            "return x[2] * x[4] "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        vector vec { 1, 2, 3, 4, 5 };
        Function<i32> func = global["foo"];
        i32 a = func(vec);
        REQUIRE(a == 2 * 4);

        array arr { 1, 2, 3, 4, 5 };
        a = func(arr);
        REQUIRE(a == 2 * 4);
    }
    {
        auto res = run_script(
            "function foo(x, y, z) "
            "if z then return x * y else return 10 end "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        auto tup = make_tuple(4, 2, true);
        Function<i32> func = global["foo"];
        i32 a = func(tup);
        REQUIRE(a == 4 * 2);
    }
    {
        auto res = run_script(
            "function foo(x, y, z) "
            "if z then return x * y else return 10 end "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        auto tup = make_tuple(make_tuple(4, 2), true);
        Function<i32> func = global["foo"];
        i32 a = func(tup);
        REQUIRE(a == 4 * 2);
    }
    {
        auto res = run_script(
            "function foo(x, y) "
            "return x * y "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        auto tup = make_pair(4, 2.4f);
        Function<f32> func = global["foo"];
        f32 a = func(tup);
        REQUIRE(a == 4 * 2.4f);
    }
    {
        auto res = run_script(
            "function foo(x) "
            "return x.test "
            "end ");
        REQUIRE(res.State == ResultState::Ok);
        map<string, i32> map = { { "test", 123 } };
        Function<i32> func = global["foo"];
        i32 a = func(map);
        REQUIRE(a == 123);

        unordered_map<string, i32> umap = { { "test", 245 } };
        a = func(umap);
        REQUIRE(a == 245);
    }
    {
        map<string, i32> map = { { "test", 123 } };
        global["foo"] = map;
        i32 a = run_script<i32>("return foo.test ");
        REQUIRE(a == 123);
    }
    {
        pair<string, i32> x = run_script<pair<string, i32>>("return 'ok', 10");
        REQUIRE(x.first == "ok");
        REQUIRE(x.second == 10);
    }
    {
        Function<i32> func = global["test"]["PairPara"];
        i32 a = func(pair { "ok"s, 4 });
        REQUIRE(a == 4);
    }
    {
        set<string> set1 { "test", "test2" };
        global["foo"] = set1;
        set<string> set2 = run_script<set<string>>("return foo ");
        REQUIRE(set1 == set2);
    }
    {
        unordered_set<string> set1 { "test", "test2" };
        global["foo"] = set1;
        unordered_set<string> set2 = run_script<unordered_set<string>>("return foo ");
        REQUIRE(set1 == set2);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.DataStructs")
{
    function testFuncColor = [](Color c) {
        return Color { static_cast<u8>(c.R * 2),
            static_cast<u8>(c.G * 2),
            static_cast<u8>(c.B * 2),
            static_cast<u8>(c.A * 2) };
    };
    function testFuncPointF = [](PointF p) {
        return PointF { p.X * 2, p.Y * 2 };
    };
    function testFuncPointI = [](PointI p) {
        return PointI { p.X * 2, p.Y * 2 };
    };
    function testFuncSizeI = [](SizeI s) {
        return SizeI { s.Width * 5, s.Height * 8 };
    };
    function testFuncRectF = [](RectF r) {
        return RectF { r.Left * 2, r.Top * 2, r.Width * 2, r.Height * 2 };
    };
    function testFuncRectI = [](RectI r) {
        return RectI { r.Left * 2, r.Top * 2, r.Width * 2, r.Height * 2 };
    };
    function testFuncMix = [](i32 i, RectF r, Color c, string s, bool b, PointI p) {
        f32 ret = i + r.Left + c.A + s.length() + (b ? 1 : 100) + p.X;
        return ret;
    };

    global["test"]["Color"] = testFuncColor;
    global["test"]["PointF"] = testFuncPointF;
    global["test"]["PointI"] = testFuncPointI;
    global["test"]["RectF"] = testFuncRectF;
    global["test"]["RectI"] = testFuncRectI;
    global["test"]["SizeI"] = testFuncSizeI;
    global["test"]["Mix"] = testFuncMix;

    {
        auto res = run_script(
            "rectI = {left=3, top=6, width=10, height=20} "
            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} "
            "color = { r= 1, g = 2, b = 3, a = 1} "
            "pointI = { x = 20, y = 400 } "
            "pointF = { x = 4.5, y = 3.23 } ");
        REQUIRE(res.State == ResultState::Ok);
        Color c = global["color"];
        REQUIRE(c == Color(1, 2, 3, 1));
        PointI p1 = global["pointI"];
        REQUIRE(p1 == PointI(20, 400));
        PointF p2 = global["pointF"];
        REQUIRE(p2 == PointF(4.5f, 3.23f));
        RectI r1 = global["rectI"];
        REQUIRE(r1 == RectI(3, 6, 10, 20));
        RectF r2 = global["rectF"];
        REQUIRE(r2 == RectF(2.7f, 3.1f, 2.3f, 55.2f));
    }
    {
        auto res = run_script(
            "rectI = {left=3, top=6, width=10, height=20} "
            "rectI = test.RectI(rectI) "

            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} "
            "rectF = test.RectF(rectF) "

            "color = { r = 1, g = 2, b = 3, a = 1 } "
            "color = test.Color(color) "

            "pointI = { x = 20, y = 400 } "
            "pointI = test.PointI(pointI) "

            "pointF = { x = 4.5, y = 3.23 } "
            "pointF = test.PointF(pointF) "

            "sizeI = { width = 20, height = 400 } "
            "sizeI = test.SizeI(sizeI) "

            "rectIS = {3, 6, 10, 20} "
            "rectFS = {2.7, 3.1, 2.3, 55.2} "
            "pointIS = { 20, 400 } "
            "pointFS = { 4.5, 3.23 } ");
        REQUIRE(res.State == ResultState::Ok);

        Color c = global["color"];
        REQUIRE(global.is<Color>("color"));
        REQUIRE(c == Color(1 * 2, 2 * 2, 3 * 2, 1 * 2));

        PointI p1 = global["pointI"];
        REQUIRE(global.is<PointI>("pointI"));
        REQUIRE(global.is<PointI>("pointIS"));
        REQUIRE(p1 == PointI(20 * 2, 400 * 2));

        PointF p2 = global["pointF"];
        REQUIRE(global.is<PointF>("pointF"));
        REQUIRE(global.is<PointF>("pointFS"));
        REQUIRE(p2 == PointF(4.5f * 2, 3.23f * 2));

        SizeI s1 = global["sizeI"];
        REQUIRE(global.is<SizeI>("sizeI"));
        REQUIRE(s1 == SizeI(20 * 5, 400 * 8));

        RectI r1 = global["rectI"];
        REQUIRE(global.is<RectI>("rectI"));
        REQUIRE(global.is<RectI>("rectIS"));
        REQUIRE(r1 == RectI(3 * 2, 6 * 2, 10 * 2, 20 * 2));

        RectF r2 = global["rectF"];
        REQUIRE(global.is<RectF>("rectF"));
        REQUIRE(global.is<RectF>("rectFS"));
        REQUIRE(r2 == RectF(2.7f * 2, 3.1f * 2, 2.3f * 2, 55.2f * 2));
    }
    {
        auto res = run_script(
            "rectF = {left=2.7, top=3.1, width=2.3, height=55.2} "
            "color = { r = 1, g = 2, b = 3, a = 1} "
            "pointI = { x = 20, y = 400 } "
            "x = test.Mix(100, rectF, color, 'Hello', false, pointI)");
        REQUIRE(res.State == ResultState::Ok);
        f32 x = global["x"];

        REQUIRE(x == testFuncMix(100, RectF(2.7f, 3.1f, 2.3f, 55.2f), Color(1, 2, 3, 1), "Hello", false, PointI(20, 400)));
    }
    {
        auto res = run_script("Colors = require 'colors'");
        REQUIRE(res.State == ResultState::Ok);
        res = run_script(
            "ab = Colors.AliceBlue "
            "b = Colors.Blue "
            "s = Colors.Salmon "
            "w = Colors.Wheat ");

        Color c = global["ab"];
        REQUIRE(c == Colors::AliceBlue);
        c = global["b"];
        REQUIRE(c == Colors::Blue);
        c = global["s"];
        REQUIRE(c == Colors::Salmon);
        c = global["w"];
        REQUIRE(c == Colors::Wheat);
    }
}

TEST_CASE_METHOD(LuaScriptTests, "Script.Lua.RawPointers")
{
    {
        TestScriptClass t;
        global["obj"] = &t;

        function func = [](TestScriptClass* x) {
            x->set_value(101);
        };

        global["func"] = func;

        auto res = run_script("func(obj)");
        REQUIRE(res.State == ResultState::Ok);
        REQUIRE(t.get_value() == 101);
    }
    {
        REQUIRE(TestScriptClass::ObjCount == 0);
        LuaOwnedPtr<TestScriptClass> t { new TestScriptClass };
        REQUIRE(TestScriptClass::ObjCount == 1);
        global["obj"] = t;
        auto res = run_script("obj = nil");
        REQUIRE(res.State == ResultState::Ok);
        perform_GC();
        REQUIRE(TestScriptClass::ObjCount == 0);
    }
}