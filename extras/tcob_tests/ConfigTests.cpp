#include "tests.hpp"
#include <string>

using namespace tcob::data::config;

TEST_CASE("Data.Config.Get")
{
    object t;
    t["section1"]["valueBool"]          = true;
    t["section1"]["valueStr"]           = "test123";
    t["section1"]["valueSec"]["a"]      = 100;
    t["section1"]["valueSec"]["b"]      = false;
    t["section1"]["valueSec"]["c"]["l"] = 1;
    t["section1"]["valueSec"]["c"]["m"] = 32;
    t["section1"]["valueArr"]           = std::vector<int> {3, 5, 9, 13};
    t["section1"]["valueFloat"]         = 123.45;

    t["section2"]["valueBool"]  = false;
    t["section2"]["valueStr"]   = "test456";
    t["section2"]["valueInt"]   = 42;
    t["section2"]["valueFloat"] = 456.78;
    t["section2"]["value.Str"]  = "123";
    t["section2"]["value=Str"]  = "123";

    SUBCASE("try_get function")
    {
        object obj {};
        REQUIRE(t.try_get(obj, "section1"));

        bool b = false;
        REQUIRE(obj.try_get<bool>(b, "valueBool"));

        REQUIRE_FALSE(obj.try_get<bool>(b, "valueBoolXXX"));
        REQUIRE_FALSE(obj.try_get<bool>(b, "valueFloat"));

        REQUIRE(t.try_get<bool>(b, "section1", "valueSec", "b"));
    }
    SUBCASE("as function")
    {
        REQUIRE(t.as<bool>("section1", "valueBool") == true);
        REQUIRE(t.as<std::string>("section1", "valueStr") == "test123");
        REQUIRE(t.as<std::string>("section2", "value.Str") == "123");
        REQUIRE(t.as<std::string>("section2", "value=Str") == "123");
        REQUIRE(t.as<f64>("section1", "valueFloat") == 123.45);
        REQUIRE(t.as<f64>("section1", "valueSec", "a") == 100);
        REQUIRE(t.as<bool>("section1", "valueSec", "b") == false);
        REQUIRE(t.as<f64>("section1", "valueSec", "c", "l") == 1);
        REQUIRE(t.as<f64>("section1", "valueArr", 2) == 9);

        object obj {t.as<object>("section1")};
        REQUIRE(obj.as<bool>("valueBool") == true);
        REQUIRE(obj.as<std::string>("valueStr") == "test123");
        REQUIRE(obj.as<f64>("valueFloat") == 123.45);
    }
    SUBCASE("subscript")
    {
        REQUIRE(t["section1"]["valueBool"].as<bool>() == true);
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == "test123");
        REQUIRE(t["section1"]["valueFloat"].as<f64>() == 123.45);
        REQUIRE(t["section1"]["valueSec"]["a"].as<f64>() == 100);
        REQUIRE(t["section1"]["valueSec"]["b"].as<bool>() == false);

        REQUIRE(t["section2"]["valueBool"].as<bool>() == false);
        REQUIRE(t["section2"]["valueStr"].as<std::string>() == "test456");
        REQUIRE(t["section2"]["valueFloat"].as<f64>() == 456.78);
    }
    SUBCASE("get non-native types")
    {
        REQUIRE(t["section2"]["valueFloat"].as<f32>() == 456.78f);
        REQUIRE(t["section2"]["valueFloat"].get<i32>().error() == error_code::TypeMismatch);
    }
    SUBCASE("get everything as string")
    {
        REQUIRE(t["section1"]["valueBool"].as<std::string>() == "true");
        REQUIRE(t["section2"]["valueBool"].as<std::string>() == "false");
        REQUIRE(t["section2"]["valueStr"].as<std::string>() == "test456");
        REQUIRE(t["section2"]["valueInt"].as<std::string>() == "42");
        REQUIRE(t["section2"]["valueFloat"].as<std::string>() == std::to_string(456.78));
        REQUIRE(t["section1"]["valueArr"].as<std::string>() == "[ 3, 5, 9, 13 ]");
        REQUIRE(t["section1"]["valueSec"].as<std::string>() == "{ a = 100, b = false, c = { l = 1, m = 32 } }");
    }
}

TEST_CASE("Data.Config.Set")
{
    SUBCASE("set function")
    {
        object t;
        t.set("section1", "valueBool", true);
        t.set("section1", "valueStr", "test123");
        t.set("section1", "valueFloat", 123.45);
        t.set("section1", "valueSec", "a", 95);
        t.set("section1", "valueArr", 0, 42);

        REQUIRE(t.as<bool>("section1", "valueBool") == true);
        REQUIRE(t.as<std::string>("section1", "valueStr") == "test123");
        REQUIRE(t.as<f64>("section1", "valueFloat") == 123.45);
        REQUIRE(t.as<f64>("section1", "valueSec", "a") == 95);
        REQUIRE(t.as<f64>("section1", "valueArr", 0) == 42);
    }
    SUBCASE("subscript")
    {
        object t;
        t["section1"]["valueBool"]  = true;
        t["section1"]["valueStr"]   = "test123";
        t["section1"]["valueFloat"] = 123.45;

        REQUIRE(t["section1"]["valueBool"].as<bool>() == true);
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == "test123");
        REQUIRE(t["section1"]["valueFloat"].as<f64>() == 123.45);
    }
    SUBCASE("set non-native types")
    {
        object t;
        t["section1"]["f32"] = 123.45f;
        t["section1"]["i32"] = 123;
        REQUIRE(t["section1"]["f32"].as<f32>() == 123.45f);
        REQUIRE(t["section1"]["i32"].as<i32>() == 123);

        char const* x {"123"};
        t["section1"]["valueStr"] = x;
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == x);
    }
    SUBCASE("replace with object")
    {
        object t;
        t["section1"]["value"] = 123.45f;
        REQUIRE(t["section1"]["value"].as<f32>() == 123.45f);

        t["section1"]["value"]["cvalue"] = 100;
        REQUIRE(t["section1"]["value"]["cvalue"].as<i32>() == 100);
    }
}

TEST_CASE("Data.Config.Has")
{
    object t;
    t["valueBool"] = false;

    t["section1"]["valueBool"]             = true;
    t["section1"]["valueSec"]["a"]         = 100;
    t["section1"]["valueSec"]["valueBool"] = false;

    t["section2"]["valueBool"] = false;

    REQUIRE(t.has("valueBool"));
    REQUIRE(t.has("section1", "valueBool"));
    REQUIRE(t.has("section2", "valueBool"));
    REQUIRE(t.has("section1", "valueSec", "valueBool"));
}

TEST_CASE("Data.Config.Is")
{
    object t;
    t["valueBool"] = false;

    t["section1"]["valueBool"]             = true;
    t["section1"]["valueFloat"]            = 2.0;
    t["section1"]["valueInt"]              = 32;
    t["section1"]["valueSec"]["a"]         = 100;
    t["section1"]["valueSec"]["valueBool"] = false;
    t["section1"]["valueSec2"]["a"]        = 100;
    t["section1"]["valueSec2"]["arr"]      = std::vector<std::variant<int, bool, std::string>> {0, false, "ok"};
    t["section1"]["valueArr"]              = std::vector<std::variant<int, bool, std::string>> {0, false, "ok"};

    t["section2"]["valueBool"] = false;

    REQUIRE(t.is<bool>("valueBool"));
    REQUIRE(t.is<bool>("section1", "valueBool"));
    REQUIRE(t.is<bool>("section2", "valueBool"));
    REQUIRE(t.is<bool>("section1", "valueSec", "valueBool"));
    REQUIRE(t.is<i64>("section1", "valueArr", 0));
    REQUIRE(t.is<bool>("section1", "valueArr", 1));
    REQUIRE(t.is<std::string>("section1", "valueArr", 2));
    REQUIRE(t.is<i64>("section1", "valueSec2", "arr", 0));
    REQUIRE(t.is<bool>("section1", "valueSec2", "arr", 1));
    REQUIRE(t.is<std::string>("section1", "valueSec2", "arr", 2));
    REQUIRE(t.is<f64>("section1", "valueFloat"));
    REQUIRE(t.is<i64>("section1", "valueInt"));

    REQUIRE_FALSE(t.is<i64>("section1", "valueFloat"));
    REQUIRE_FALSE(t.is<bool>("section1", "valueInt"));
}

TEST_CASE("Data.Config.Array")
{
    object t;
    t["section1"]["valueBool"]  = true;
    t["section1"]["valueStr"]   = "test123";
    t["section1"]["valueArray"] = std::vector<std::variant<int, std::string, bool>> {1, "a", true};
    t["section1"]["valueFloat"] = 123.45;

    SUBCASE("access items")
    {
        REQUIRE(t["section1"]["valueArray"].is<array>());
        array arr {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr.get_size() == 3);
        REQUIRE(arr[0].is<i64>());
        REQUIRE(arr[1].is<std::string>());
        REQUIRE(arr[2].is<bool>());
        REQUIRE(arr[0].as<f64>() == 1);
        REQUIRE(arr[1].as<std::string>() == "a");
        REQUIRE(arr[2].as<bool>() == true);
    }

    SUBCASE("modify array")
    {
        REQUIRE(t["section1"]["valueArray"].is<array>());

        array arr {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr.get_size() == 3);

        array arr1 {t["section1"]["valueArray"].as<array>()};
        arr[0] = 100;
        REQUIRE(arr1[0].as<f64>() == 100);
        arr[1] = false;
        REQUIRE(arr1[1].as<bool>() == false);
        arr[2] = "testString";
        REQUIRE(arr1[2].as<std::string>() == "testString");

        array arr2 {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr2[0].as<f64>() == 100);
        REQUIRE(arr2[1].as<bool>() == false);
        REQUIRE(arr2[2].as<std::string>() == "testString");
    }

    SUBCASE("auto grow")
    {
        array a;
        a[100] = 1;
        REQUIRE(a.get_size() == 101);
        REQUIRE(a[100].as<i32>() == 1);
    }

    SUBCASE("from vector")
    {
        {
            std::vector<i32> vec {1, 2, 3, 4, 5, 6};
            array            testArray {std::span<i32 const> {vec}};
            REQUIRE(testArray.get_size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<i32>() == vec[i]);
            }
        }
        {
            std::vector<f64> vec {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            array            testArray {std::span<f64 const> {vec}};
            REQUIRE(testArray.get_size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<f64>() == vec[i]);
            }
        }
    }

    SUBCASE("from values")
    {
        {
            std::vector<i32> vec {1, 2, 3, 4, 5, 6};
            array            testArray {1, 2, 3, 4, 5, 6};
            REQUIRE(testArray.get_size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<i32>() == vec[i]);
            }
        }
        {
            std::vector<f64> vec {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            array            testArray {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            REQUIRE(testArray.get_size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<f64>() == vec[i]);
            }
        }
    }

    SUBCASE("equality")
    {
        using namespace tcob::literals;

        array test {1, 2, 3};
        array good {1, 2, 3};

        REQUIRE(test == good);

        array bad {1, true, 3};

        REQUIRE_FALSE(test == bad);
    }

    SUBCASE("get_type")
    {
        using namespace tcob::literals;

        object obj = R"(
            array  = ["a",1.2,3,true,[1,2,3],{a=1,b=2,c=3}]
        )"_ini;

        auto arr = obj["array"].as<array>();
        REQUIRE(arr.get_type(0) == type::String);
        REQUIRE(arr.get_type(1) == type::Float);
        REQUIRE(arr.get_type(2) == type::Integer);
        REQUIRE(arr.get_type(3) == type::Bool);
        REQUIRE(arr.get_type(4) == type::Array);
        REQUIRE(arr.get_type(5) == type::Object);
        REQUIRE(arr.get_type(6) == type::Null);
    }
}

TEST_CASE("Data.Config.TcobTypes")
{
    object obj;
    obj["point"] = point_i {100, 350};
    obj["color"] = color {15, 30, 12, 0};
    obj["size"]  = size_i {300, 450};
    obj["rect"]  = rect_f {4.5f, 2.5f, 30.1f, 45.01f};

    REQUIRE(obj.is<point_i>("point"));
    REQUIRE(obj["point"].as<point_i>() == point_i {100, 350});

    REQUIRE(obj.is<color>("color"));
    REQUIRE(obj["color"].as<color>() == color {15, 30, 12, 0});

    REQUIRE(obj.is<size_i>("size"));
    REQUIRE(obj["size"].as<size_i>() == size_i {300, 450});

    REQUIRE(obj.is<rect_f>("rect"));
    REQUIRE(obj["rect"].as<rect_f>() == rect_f {4.5f, 2.5f, 30.1f, 45.01f});
}

TEST_CASE("Data.Config.STLTypes")
{
    object obj;
    obj["stringArray"]       = std::vector<std::string> {"One", "Two", "Three"};
    obj["intArray"]          = std::vector<int> {1, 2, 3};
    obj["stringintMap"]["a"] = 123;
    obj["stringintMap"]["b"] = 456;
    obj["variantMap"]["f"]   = 1.5;
    obj["variantMap"]["b"]   = true;
    obj["variantMap"]["s"]   = "ok";
    obj["duration"]          = 100;
    obj["tuple"]             = std::tuple<int, std::string, bool> {123, "ok", true};
    obj["pair"]              = std::pair<std::string, int> {"ok", 100};
    obj["set"]               = std::vector<int> {1, 1, 2, 2, 3, 3};

    SUBCASE("vector")
    {
        {
            auto objectarr0 = obj["stringArray"].as<std::vector<std::string>>();
            REQUIRE(objectarr0 == std::vector<std::string> {"One", "Two", "Three"});

            auto objectarr1 = obj["intArray"].as<std::vector<int>>();
            REQUIRE(objectarr1 == std::vector<int> {1, 2, 3});
        }

        {
            obj["stringArray2"] = std::vector<std::string> {"a", "b", "c"};
            auto objectarr0     = obj["stringArray2"].as<std::vector<std::string>>();
            REQUIRE(objectarr0 == std::vector<std::string> {"a", "b", "c"});

            obj["intArray2"] = std::vector<i64> {0, 5, 10};
            auto objectarr1  = obj["intArray2"].as<std::vector<int>>();
            REQUIRE(objectarr1 == std::vector<int> {0, 5, 10});
        }
    }

    SUBCASE("array")
    {
        {
            auto objectarr0 = obj["stringArray"].as<std::array<std::string, 3>>();
            REQUIRE(objectarr0 == std::array<std::string, 3> {"One", "Two", "Three"});

            auto objectarr1 = obj["intArray"].as<std::array<int, 3>>();
            REQUIRE(objectarr1 == std::array<int, 3> {1, 2, 3});
        }

        {
            obj["stringArray2"] = std::array<std::string, 3> {"a", "b", "c"};
            auto objectarr0     = obj["stringArray2"].as<std::array<std::string, 3>>();
            REQUIRE(objectarr0 == std::array<std::string, 3> {"a", "b", "c"});

            obj["intArray2"] = std::array<int, 3> {0, 5, 10};
            auto objectarr1  = obj["intArray2"].as<std::array<int, 3>>();
            REQUIRE(objectarr1 == std::array<int, 3> {0, 5, 10});
        }
    }

    SUBCASE("tuple")
    {
        {
            auto tup0 = obj["tuple"].as<std::tuple<i32, std::string, bool>>();
            REQUIRE(tup0 == std::tuple<i32, std::string, bool> {123, "ok", true});
        }

        {
            obj["tuple2"] = std::tuple<std::string, bool, f32> {"a", false, 3.5f};
            auto tup0     = obj["tuple2"].as<std::tuple<std::string, bool, f32>>();
            REQUIRE(tup0 == std::tuple<std::string, bool, f32> {"a", false, 3.5f});
        }
    }

    SUBCASE("pair")
    {
        {
            auto pair0 = obj["pair"].as<std::pair<std::string, i32>>();
            REQUIRE(pair0 == std::pair<std::string, i32> {"ok", 100});
        }
    }

    SUBCASE("variant")
    {
        {
            auto var = obj["duration"].as<std::variant<std::string, i32>>();
            REQUIRE(std::get<int>(var) == 100);
        }
    }

    SUBCASE("optional")
    {
        {
            auto var = obj["duration"].as<std::optional<i32>>();
            REQUIRE(var);
            REQUIRE(*var == 100);
        }
        {
            auto var = obj["duration"].as<std::optional<bool>>();
            REQUIRE_FALSE(var);
        }
    }

    SUBCASE("map")
    {
        {
            auto objectMap = obj["stringintMap"].as<std::map<std::string, int>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["a"] == 123);
            REQUIRE(objectMap["b"] == 456);
        }

        {
            obj["stringintMap2"] = std::map<std::string, int> {{"c", 555}, {"d", 666}};
            auto objectMap       = obj["stringintMap2"].as<std::map<std::string, int>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["c"] == 555);
            REQUIRE(objectMap["d"] == 666);
        }

        {
            auto objectMap = obj["variantMap"].as<std::map<std::string, cfg_value>>();
            REQUIRE(objectMap.size() == 3);
            REQUIRE(std::get<f64>(objectMap["f"]) == 1.5);
            REQUIRE(std::get<bool>(objectMap["b"]) == true);
            REQUIRE(std::get<std::string>(objectMap["s"]) == "ok");
        }
    }

    SUBCASE("unordered_map")
    {
        {
            auto objectMap = obj["stringintMap"].as<std::unordered_map<std::string, int>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["a"] == 123);
            REQUIRE(objectMap["b"] == 456);
        }

        {
            obj["stringintMap2"] = std::map<std::string, int> {{"c", 555}, {"d", 666}};
            auto objectMap       = obj["stringintMap2"].as<std::unordered_map<std::string, int>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["c"] == 555);
            REQUIRE(objectMap["d"] == 666);
        }
    }

    SUBCASE("set")
    {
        {
            auto objectarr0 = obj["stringArray"].as<std::set<std::string>>();
            REQUIRE(objectarr0 == std::set<std::string> {"One", "Two", "Three"});

            auto objectarr1 = obj["intArray"].as<std::set<i32>>();
            REQUIRE(objectarr1 == std::set<i32> {1, 2, 3});

            auto objectarr2 = obj["set"].as<std::set<i32>>();
            REQUIRE(objectarr2 == std::set<i32> {1, 2, 3});
        }
    }

    SUBCASE("unordered_set")
    {
        {
            auto objectarr0 = obj["stringArray"].as<std::unordered_set<std::string>>();
            REQUIRE(objectarr0 == std::unordered_set<std::string> {"One", "Two", "Three"});

            auto objectarr1 = obj["intArray"].as<std::unordered_set<i32>>();
            REQUIRE(objectarr1 == std::unordered_set<i32> {1, 2, 3});

            auto objectarr2 = obj["set"].as<std::unordered_set<i32>>();
            REQUIRE(objectarr2 == std::unordered_set<i32> {1, 2, 3});
        }
    }

    SUBCASE("duration")
    {
        {
            auto value = obj["duration"].as<milliseconds>();
            REQUIRE(value == milliseconds {100});
        }

        {
            obj["duration2"] = milliseconds {360};
            auto value       = obj["duration2"].as<milliseconds>();
            REQUIRE(value == milliseconds {360});
        }
    }
}

TEST_CASE("Data.Config.ForLoop")
{
    SUBCASE("object")
    {
        object obj {};
        obj["first"]  = 0;
        obj["second"] = 2;
        obj["third"]  = 12;

        std::set<i32>         values;
        std::set<std::string> names;
        for (auto const& [k, v] : obj) {
            values.insert(v.as<i32>());
            names.insert(k);
        }

        REQUIRE(values == std::set<i32> {0, 2, 12});
        REQUIRE(names == std::set<std::string> {"first", "second", "third"});
    }

    SUBCASE("array")
    {
        array arr {};
        arr.add(0);
        arr.add(2);
        arr.add(12);

        std::vector<i32> values;
        for (auto const& value : arr) {
            values.push_back(value.as<i32>());
        }

        REQUIRE(values == std::vector<i32> {0, 2, 12});
    }
}

TEST_CASE("Data.Config.Schema")
{
    SUBCASE("AllOf")
    {
        schema s0;
        s0.AllOf = {
            schema::string_property {"string"},
            schema::float_property {"float"},
            schema::int_property {"integer"},
            schema::array_property {"array"},
            schema::object_property {"object"},
            schema::bool_property {"bool"},
        };

        object goodSection {};
        goodSection["string"]  = "ok";
        goodSection["float"]   = 2.f;
        goodSection["integer"] = 2;
        goodSection["array"]   = array {};
        goodSection["object"]  = object {};
        goodSection["bool"]    = true;
        REQUIRE(s0.validate(goodSection));

        object badSection0 {};
        badSection0["x"]   = "ok";
        badSection0["y"]   = 2;
        badSection0["z"]   = array {};
        badSection0["aaa"] = object {};
        badSection0["bbb"] = true;
        REQUIRE_FALSE(s0.validate(badSection0));

        object badSection1 {};
        badSection1["string"] = "ok";
        badSection1["float"]  = 2;
        badSection1["array"]  = array {};
        badSection1["object"] = object {};
        badSection1["bool"]   = true;
        REQUIRE_FALSE(s0.validate(badSection1));

        object badSection2 {};
        badSection2["string"]  = "ok";
        badSection2["float"]   = 2.f;
        badSection2["integer"] = "2";
        badSection2["array"]   = array {};
        badSection2["object"]  = object {};
        badSection2["bool"]    = true;
        REQUIRE_FALSE(s0.validate(badSection2));
    }

    SUBCASE("AnyOf")
    {
        schema s0;
        s0.AnyOf = {
            schema::string_property {"string"},
            schema::float_property {"float"},
            schema::int_property {"integer"},
        };

        object goodSection0 {};
        goodSection0["string"]  = "ok";
        goodSection0["float"]   = 2.f;
        goodSection0["integer"] = 2;
        REQUIRE(s0.validate(goodSection0));

        object goodSection1 {};
        goodSection1["string"] = "ok";
        REQUIRE(s0.validate(goodSection1));

        object goodSection2 {};
        goodSection2["float"]   = 2.f;
        goodSection2["integer"] = 2;
        REQUIRE(s0.validate(goodSection2));

        object badSection0 {};
        badSection0["bla"] = 2.f;
        REQUIRE_FALSE(s0.validate(badSection0));
    }

    SUBCASE("OneOf")
    {
        schema s0;
        s0.OneOf = {
            schema::string_property {"string"},
            schema::float_property {"float"},
            schema::int_property {"integer"},
        };

        object goodSection0 {};
        goodSection0["string"] = "ok";
        REQUIRE(s0.validate(goodSection0));

        object goodSection1 {};
        goodSection1["float"] = 2.f;
        REQUIRE(s0.validate(goodSection1));

        object badSection0 {};
        badSection0["float"]   = 2.f;
        badSection0["integer"] = 2;
        REQUIRE_FALSE(s0.validate(badSection0));

        object badSection1 {};
        badSection1["float"] = "ok";
        REQUIRE_FALSE(s0.validate(badSection1));

        object badSection2 {};
        badSection2["bla"] = 2.f;
        REQUIRE_FALSE(s0.validate(badSection2));
    }

    SUBCASE("NoneOf")
    {
        schema s0;
        s0.NoneOf = {
            schema::string_property {"string"},
            schema::float_property {"float"},
        };

        object obj {};
        obj["string"] = 12;
        REQUIRE(s0.validate(obj));

        obj["string"] = "ok";
        REQUIRE_FALSE(s0.validate(obj));

        obj["string"] = nullptr;
        REQUIRE(s0.validate(obj));

        obj["float"] = 2;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("string_property")
    {
        {
            schema s0;
            s0.AllOf = {schema::string_property {
                .Name      = "string",
                .MinLength = 3,
                .MaxLength = 5,
            }};

            object obj {};
            obj["string"] = "abc";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcd";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcde";
            REQUIRE(s0.validate(obj));

            obj["string"] = "ab";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = "abcdef";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = 123;
            REQUIRE_FALSE(s0.validate(obj));
        }
        {
            schema s0;
            s0.AllOf = {schema::string_property {
                .Name    = "string",
                .Pattern = "a*e",
            }};

            object obj {};
            obj["string"] = "abe";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcde";
            REQUIRE(s0.validate(obj));

            obj["string"] = "ab";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = "abcdef";
            REQUIRE_FALSE(s0.validate(obj));
        }
    }

    SUBCASE("float_property")
    {
        schema s0;
        s0.AllOf = {schema::float_property {
            .Name     = "float",
            .MinValue = 3.f,
            .MaxValue = 5.f,
        }};

        object obj {};
        obj["float"] = 3.f;
        REQUIRE(s0.validate(obj));
        obj["float"] = 4.f;
        REQUIRE(s0.validate(obj));
        obj["float"] = 5.f;
        REQUIRE(s0.validate(obj));

        obj["float"] = 1.f;
        REQUIRE_FALSE(s0.validate(obj));
        obj["float"] = 2.f;
        REQUIRE_FALSE(s0.validate(obj));
        obj["float"] = 5.1f;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("int_property")
    {
        schema s0;
        s0.AllOf = {schema::int_property {
            .Name     = "int",
            .MinValue = 3,
            .MaxValue = 5,
        }};

        object obj {};
        obj["int"] = 3;
        REQUIRE(s0.validate(obj));
        obj["int"] = 4;
        REQUIRE(s0.validate(obj));
        obj["int"] = 5;
        REQUIRE(s0.validate(obj));

        obj["int"] = 1;
        REQUIRE_FALSE(s0.validate(obj));
        obj["int"] = 2;
        REQUIRE_FALSE(s0.validate(obj));
        obj["int"] = 6;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("array_property")
    {
        schema s0;
        s0.AllOf = {schema::array_property {
            .Name     = "array",
            .MinSize  = 3,
            .MaxSize  = 5,
            .ItemType = type::Integer,
        }};

        array arr;
        arr.add(1);
        arr.add(2);
        arr.add(3);
        arr.add(4);

        object obj {};
        obj["array"] = arr;
        REQUIRE(s0.validate(obj));

        arr.add(3);
        arr.add(4);
        REQUIRE_FALSE(s0.validate(obj));

        for (i32 i {0}; i < 3; ++i) {
            arr.pop_back();
        }
        REQUIRE(s0.validate(obj));

        arr.add("X");
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("section_property")
    {
        auto c0 {std::make_shared<schema>()};
        c0->AllOf = {schema::string_property {"string"}, schema::int_property {"int"}};

        schema s0;
        s0.AllOf = {schema::object_property {
            .Name   = "object",
            .Schema = c0,
        }};

        object csec {};
        csec["string"] = "ok";
        csec["int"]    = 42;

        object obj {};
        obj["object"] = csec;
        REQUIRE(s0.validate(obj));

        csec["string"] = nullptr;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("result")
    {
        SUBCASE("AllOf")
        {
            schema s0;
            s0.AllOf = {schema::string_property {"string"}};

            object obj {};
            obj["float"] = 2.f;

            auto res {s0.validate(obj)};
            REQUIRE_FALSE(res);

            REQUIRE(res.Failures.size() == 1);
            REQUIRE(res.Failures[0].Constraint == "Name");
            REQUIRE(res.Failures[0].Name == "string");
            REQUIRE(res.Failures[0].Group == "AllOf");
        }
        SUBCASE("OneOf")
        {
            schema s0;
            s0.OneOf = {schema::string_property {"string"},
                        schema::float_property {"float"}};

            SUBCASE("more than one")
            {
                object obj {};
                obj["string"] = "ok";
                obj["float"]  = 2.f;

                auto res {s0.validate(obj)};
                REQUIRE_FALSE(res);

                REQUIRE(res.Failures.size() == 1);
                REQUIRE(res.Failures[0].Constraint == "Group");
                REQUIRE(res.Failures[0].Name == "float");
                REQUIRE(res.Failures[0].Group == "OneOf");
            }
            SUBCASE("none")
            {
                object obj {};
                obj["x"] = "ok";
                obj["y"] = 2.f;

                auto res {s0.validate(obj)};
                REQUIRE_FALSE(res);

                REQUIRE(res.Failures.size() == 2);
                REQUIRE(res.Failures[0].Constraint == "Name");
                REQUIRE(res.Failures[0].Name == "string");
                REQUIRE(res.Failures[0].Group == "OneOf");
                REQUIRE(res.Failures[1].Constraint == "Name");
                REQUIRE(res.Failures[1].Name == "float");
                REQUIRE(res.Failures[1].Group == "OneOf");
            }
        }
    }
    SUBCASE("FromSection")
    {
        std::shared_ptr<schema> s0;

        {
            object ssec;
            ssec["properties"]["x"]["type"]                   = "String";
            ssec["properties"]["y"]["type"]                   = "Float";
            ssec["properties"]["sub"]["type"]                 = "Object";
            ssec["properties"]["sub"]["schema"]               = "sub";
            ssec["allof"][0]                                  = "x";
            ssec["allof"][1]                                  = "y";
            ssec["allof"][2]                                  = "sub";
            ssec["schemas"]["sub"]["properties"]["a"]["type"] = "String";
            ssec["schemas"]["sub"]["properties"]["b"]["type"] = "Float";
            ssec["schemas"]["sub"]["allof"][0]                = "a";
            ssec["schemas"]["sub"]["allof"][1]                = "b";

            s0 = schema::FromObject(ssec);

            REQUIRE(s0);
            REQUIRE(s0->AllOf.size() == 3);
        }

        object sec0 {};
        sec0["x"]        = "ok";
        sec0["y"]        = 2.f;
        sec0["sub"]["a"] = "ok";
        sec0["sub"]["b"] = 2.f;
        REQUIRE(s0->validate(sec0));

        object sec1 {};
        sec1["x"] = "ok";
        sec1["y"] = 2.f;
        REQUIRE_FALSE(s0->validate(sec1));
    }
}

enum class TestEnum0 {
    True         = 0,
    False        = 1,
    FileNotFound = 2
};

enum class TestEnum1 {
    True         = 0,
    False        = 1,
    FileNotFound = 2
};

TEST_CASE("Data.Config.Enum")
{
    using namespace tcob::literals;

    SUBCASE("FromString translation")
    {

        object const t =
            R"(
            valueEnum0 = True
            valueEnum1 = False
            valueEnum2 = FileNotFound
        )"_ini;

        REQUIRE(t["valueEnum0"].is<TestEnum1>());
        REQUIRE(t["valueEnum1"].is<TestEnum1>());
        REQUIRE(t["valueEnum2"].is<TestEnum1>());

        REQUIRE(t["valueEnum0"].as<TestEnum1>() == TestEnum1::True);
        REQUIRE(t["valueEnum1"].as<TestEnum1>() == TestEnum1::False);
        REQUIRE(t["valueEnum2"].as<TestEnum1>() == TestEnum1::FileNotFound);
    }
    SUBCASE("ToString translation")
    {

        object t {};
        t["valueEnum0"] = TestEnum1::True;
        t["valueEnum1"] = TestEnum1::False;
        t["valueEnum2"] = TestEnum1::FileNotFound;

        REQUIRE(t["valueEnum0"].is<TestEnum1>());
        REQUIRE(t["valueEnum1"].is<TestEnum1>());
        REQUIRE(t["valueEnum2"].is<TestEnum1>());

        REQUIRE(t["valueEnum0"].as<TestEnum1>() == TestEnum1::True);
        REQUIRE(t["valueEnum1"].as<TestEnum1>() == TestEnum1::False);
        REQUIRE(t["valueEnum2"].as<TestEnum1>() == TestEnum1::FileNotFound);

        REQUIRE(t["valueEnum0"].as<std::string>() == "True");
        REQUIRE(t["valueEnum1"].as<std::string>() == "False");
        REQUIRE(t["valueEnum2"].as<std::string>() == "FileNotFound");
    }
}
