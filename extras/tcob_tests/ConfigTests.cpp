#include "tests.hpp"
#include <string>

using namespace tcob::data::config;

static std::string const EXT {".ini"};

TEST_CASE("Data.Ini.Get")
{
    std::string const iniString =
        R"(
            ;comment1_6
            [section1]
            ;comment2_6
            valueBool  = true 
            valueStr   = test123 
            #comment3_6
            valueSec   = { a = 100, b = false, c = { l = 1, m = 32 } } 
            valueArr   = [ 3, 5, 9, 13 ]
            ;comment5_6
            valueFloat = 123.45
            [section2]
            valueBool  = false
            valueStr   = "test456"
            valueInt   = 42
            #comment6_6
            valueFloat = 456.78
            'value.Str' = '123'
        )";
    object t;
    REQUIRE(t.parse(iniString, EXT));
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

TEST_CASE("Data.Ini.Set")
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

TEST_CASE("Data.Ini.Has")
{
    std::string const iniString =
        R"(
            valueBool  = false
            [section1]
            valueBool  = true
            valueSec   = { a = 100, valueBool = false }
            [section2]
            valueBool  = false
        )";

    object t;
    REQUIRE(t.parse(iniString, EXT));
    REQUIRE(t.has("valueBool"));
    REQUIRE(t.has("section1", "valueBool"));
    REQUIRE(t.has("section2", "valueBool"));
    REQUIRE(t.has("section1", "valueSec", "valueBool"));
}

TEST_CASE("Data.Ini.Is")
{
    using namespace tcob::literals;

    object const t =
        R"(
            valueBool  = false
            [section1]
            valueBool  = true
            valueFloat = 2.0
            valueInt   = 32
            valueSec   = { a = 100, valueBool = false }
            valueSec2  = { a = 100, arr = [  0, false, "ok" ] }
            valueArr   = [ 0, false, "ok" ]
            [section2]
            valueBool  = false
        )"_ini;

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

TEST_CASE("Data.Ini.Array")
{
    SUBCASE("parse")
    {
        {
            std::string const arrString {"[1,2,3,a,b,c]"};

            auto arr {array::Parse(arrString, EXT)};
            REQUIRE(arr);
            REQUIRE(arr->get_size() == 6);
            REQUIRE((*arr)[0].as<i64>() == 1);
            REQUIRE((*arr)[1].as<i64>() == 2);
            REQUIRE((*arr)[2].as<i64>() == 3);
            REQUIRE((*arr)[3].as<std::string>() == "a");
            REQUIRE((*arr)[4].as<std::string>() == "b");
            REQUIRE((*arr)[5].as<std::string>() == "c");
        }
        {
            std::string const arrString {"[ ]"};

            auto arr {array::Parse(arrString, EXT)};
            REQUIRE(arr);
            REQUIRE(arr->get_size() == 0);
        }
    }

    std::string const iniString =
        R"(
            [section1]
            valueBool  = true
            valueStr   = test123
            valueArray = [1, "a", true]
            valueFloat = 123.45            
        )";

    SUBCASE("access items")
    {
        object t;
        REQUIRE(t.parse(iniString, EXT));
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
        object t;
        REQUIRE(t.parse(iniString, EXT));
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

        object t = R"(
            array  = ["a",1.2,3,true,[1,2,3],{a=1,b=2,c=3}]
        )"_ini;

        REQUIRE(t["array"].as<array>().get_type(0) == type::String);
        REQUIRE(t["array"].as<array>().get_type(1) == type::Float);
        REQUIRE(t["array"].as<array>().get_type(2) == type::Integer);
        REQUIRE(t["array"].as<array>().get_type(3) == type::Bool);
        REQUIRE(t["array"].as<array>().get_type(4) == type::Array);
        REQUIRE(t["array"].as<array>().get_type(5) == type::Object);
        REQUIRE(t["array"].as<array>().get_type(6) == type::Null);
    }
}

TEST_CASE("Data.Ini.Multiline")
{
    std::string const iniString =
        R"(
            [section1]
            multiLineArray = [
                1,
                3,
                5,
                7
            ]
            multiLineSection = {
                a = 1, b = 3,
                c = 5, d = 7
            }
            nestedMultiLine = {
                a = 1, b = 3,
                array = [
                    1,2,
                    4
                ],
                c = 5,
                object = { b = 10,
                s = 100}, d = 7
            }

            xy_regions  = {
                acidic_floor0  = { level = 0, height = 32, width = 32, x = 36, y = 1856 },
                black_cobalt03 = { level = 0, height = 32, width = 32, x = 71, y = 36   },
                bog_green2     = { level = 0, height = 32, width = 32, x = 71, y = 456  },
                cobble_blood10 = { level = 0, height = 32, width = 32, x = 71, y = 771  }
            }

            multiLineString = "abc
                               def
                               ghi"                        
        )";

    object t;
    REQUIRE(t.parse(iniString, EXT));

    REQUIRE(t["section1"]["multiLineArray"].is<array>());
    REQUIRE(t["section1"]["multiLineArray"][0].as<i32>() == 1);
    REQUIRE(t["section1"]["multiLineArray"][1].as<i32>() == 3);
    REQUIRE(t["section1"]["multiLineArray"][2].as<i32>() == 5);
    REQUIRE(t["section1"]["multiLineArray"][3].as<i32>() == 7);

    REQUIRE(t["section1"]["multiLineSection"].is<object>());
    REQUIRE(t["section1"]["multiLineSection"]["a"].as<i32>() == 1);
    REQUIRE(t["section1"]["multiLineSection"]["b"].as<i32>() == 3);
    REQUIRE(t["section1"]["multiLineSection"]["c"].as<i32>() == 5);
    REQUIRE(t["section1"]["multiLineSection"]["d"].as<i32>() == 7);

    REQUIRE(t["section1"]["multiLineString"].is<std::string>());
    REQUIRE(t["section1"]["multiLineString"].as<std::string>() == "abc\ndef\nghi");
}

TEST_CASE("Data.Ini.Sections")
{
    SUBCASE("nested section")
    {
        std::string const iniString =
            R"(
            [section1]
            id153 = { size = { width = 16, height = 23 }, offset = { x = 1, y = 0 }, advance_x = 18, tex_region = { level = 0, x = 0, y = 0.09375, width = 0.0625, height = 0.0898438 } }
            valueBool    = true
            valueStr     = test123
            valueSection = { a = 1, b = "a", xyz = true }
            valueArray   = [1, "a", true]
            valueFloat   = 123.45
            dotSection.a   = 100   
            dotSection.c.a = 420
            dotSection.b   = 42
            dotSection.d   = [1,2,3]
            dotSection.e   = {a=1,b=2,c=3}
            dotSection.c.d = 69
            dotSection.x   = { a.x = 100, a.y = 300 }

            [section1.subsection]
            a = 100
            b = 500

            [section1.subsection.subsection]
            x = 300
            y = 600
        )";

        object t;
        REQUIRE(t.parse(iniString, EXT));

        REQUIRE(t["section1"]["id153"].is<object>());
        object id153 {t["section1"]["id153"].as<object>()};
        REQUIRE(id153["size"].as<size_u>() == size_u {16, 23});
        REQUIRE(id153["offset"].as<point_f>() == point_f {1, 0});
        REQUIRE(id153["advance_x"].as<f32>() == 18.f);
        REQUIRE(id153["tex_region"].as<rect_f>() == rect_f {0, 0.09375f, 0.0625f, 0.0898438f});
        REQUIRE(id153["tex_region"]["level"].as<i32>() == 0);

        REQUIRE(t["section1"]["dotSection"].is<object>());
        REQUIRE(t["section1"]["dotSection"]["a"].as<i32>() == 100);
        REQUIRE(t["section1"]["dotSection"]["b"].as<i32>() == 42);
        REQUIRE(t["section1"]["dotSection"]["c"]["a"].as<i32>() == 420);
        REQUIRE(t["section1"]["dotSection"]["c"]["d"].as<i32>() == 69);
        REQUIRE(t["section1"]["dotSection"]["d"][0].as<i32>() == 1);
        REQUIRE(t["section1"]["dotSection"]["d"][1].as<i32>() == 2);
        REQUIRE(t["section1"]["dotSection"]["d"][2].as<i32>() == 3);
        REQUIRE(t["section1"]["dotSection"]["e"]["a"].as<i32>() == 1);
        REQUIRE(t["section1"]["dotSection"]["e"]["b"].as<i32>() == 2);
        REQUIRE(t["section1"]["dotSection"]["e"]["c"].as<i32>() == 3);
        REQUIRE(t["section1"]["dotSection"]["x"]["a"]["x"].as<i32>() == 100);
        REQUIRE(t["section1"]["dotSection"]["x"]["a"]["y"].as<i32>() == 300);

        REQUIRE(t["section1"]["subsection"]["a"].as<i32>() == 100);
        REQUIRE(t["section1"]["subsection"]["b"].as<i32>() == 500);
        REQUIRE(t["section1"]["subsection"]["subsection"]["x"].as<i32>() == 300);
        REQUIRE(t["section1"]["subsection"]["subsection"]["y"].as<i32>() == 600);

        REQUIRE(t["section1"]["valueSection"].is<object>());
        object sec0 {t["section1"]["valueSection"].as<object>()};
        REQUIRE(sec0["a"].is<i64>());
        REQUIRE(sec0["b"].is<std::string>());
        REQUIRE(sec0["xyz"].is<bool>());
        REQUIRE(sec0["a"].as<f64>() == 1);
        REQUIRE(sec0["b"].as<std::string>() == "a");
        REQUIRE(sec0["xyz"].as<bool>() == true);
    }

    SUBCASE("inline section")
    {
        {
            std::string const iniString =
                R"([section1.a]
                   b = 100)";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["section1"]["a"].is<object>());
            REQUIRE(t["section1"]["a"]["b"].as<i32>() == 100);
        }
        {
            std::string const iniString =
                R"([section1]
                   b = { a = 100 }
                   [section1.c]
                   a = 100)";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["section1"]["c"].is<object>());
            REQUIRE(t["section1"]["c"]["a"].as<i32>() == 100);
        }
    }

    SUBCASE("empty section")
    {
        {
            std::string const iniString =
                R"([section1])";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["section1"].is<object>());
        }
        {
            std::string const iniString =
                R"(sec = { })";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["sec"].is<object>());
        }
        {
            std::string const iniString =
                R"([section1]
                   [section1.x]
                   b = 300
                   [section2]
                   a = 100
                   [section3])";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["section1"].is<object>());
            REQUIRE(t["section1"]["x"].is<object>());
            REQUIRE(t["section1"]["x"]["b"].is<i64>());
            REQUIRE(t["section2"].is<object>());
            REQUIRE(t["section2"]["a"].is<i64>());
            REQUIRE(t["section3"].is<object>());
        }
    }

    SUBCASE("modify section")
    {
        object t;
        t["section1"]["valueSection"] = object {};
        REQUIRE(t["section1"]["valueSection"].is<object>());

        object obj {t["section1"]["valueSection"].as<object>()};

        object obj1 {t["section1"]["valueSection"].as<object>()};
        obj["a"] = 100;
        REQUIRE(obj1["a"].as<f64>() == 100);
        obj["b"] = false;
        REQUIRE(obj1["b"].as<bool>() == false);
        obj["xyz"] = "testString";
        REQUIRE(obj1["xyz"].as<std::string>() == "testString");

        object obj2 {t["section1"]["valueSection"].as<object>()};
        REQUIRE(obj2["a"].as<f64>() == 100);
        REQUIRE(obj2["b"].as<bool>() == false);
        REQUIRE(obj2["xyz"].as<std::string>() == "testString");
    }

    SUBCASE("adding and removing object")
    {
        object t;

        object obj {};
        obj["a"]   = 100;
        obj["b"]   = false;
        obj["xyz"] = "testString";
        t.set("section1", obj);

        REQUIRE(t.has("section1"));
        object sec2 {t["section1"].as<object>()};
        REQUIRE(sec2["a"].as<f64>() == 100);
        REQUIRE(sec2["xyz"].as<std::string>() == "testString");
        REQUIRE(sec2["b"].as<bool>() == false);

        t.set("section1", nullptr);
        REQUIRE_FALSE(t.has("section1"));
    }

    SUBCASE("merge")
    {
        {
            object s0;
            s0["section1"]["a"] = 100;
            s0["section1"]["b"] = 200;
            s0["section2"]["a"] = 300;

            object s1;
            s1["section1"]["a"] = 150;
            s1["section1"]["c"] = 400;
            s1["section3"]["a"] = 500;

            s0.merge(s1, true);

            REQUIRE(s0["section1"]["a"].as<i32>() == 150);
            REQUIRE(s0["section1"]["b"].as<i32>() == 200);
            REQUIRE(s0["section1"]["c"].as<i32>() == 400);
            REQUIRE(s0["section2"]["a"].as<i32>() == 300);
            REQUIRE(s0["section3"]["a"].as<i32>() == 500);
        }

        {
            object s0;
            s0["section1"]["a"] = 100;
            s0["section1"]["b"] = 200;
            s0["section2"]["a"] = 300;

            object s1;
            s1["section1"]["a"] = 150;
            s1["section1"]["c"] = 400;
            s1["section3"]["a"] = 500;

            s0.merge(s1, false);

            REQUIRE(s0["section1"]["a"].as<i32>() == 100);
            REQUIRE(s0["section1"]["b"].as<i32>() == 200);
            REQUIRE(s0["section1"]["c"].as<i32>() == 400);
            REQUIRE(s0["section2"]["a"].as<i32>() == 300);
            REQUIRE(s0["section3"]["a"].as<i32>() == 500);
        }
        {
            std::string const section0Str =
                R"(
                    [texture.tex1]
                    source = tex1.png
                 )";
            object s0;
            s0.parse(section0Str, EXT);

            std::string const section1Str =
                R"(
                    [texture.tex2]
                    source = tex2.png
                 )";
            object s1;
            s1.parse(section1Str, EXT);

            object tex;
            tex.merge(s0, true);
            tex.merge(s1, true);

            REQUIRE(tex["texture"]["tex1"]["source"].as<std::string>() == "tex1.png");
            REQUIRE(tex["texture"]["tex2"]["source"].as<std::string>() == "tex2.png");
        }
    }

    SUBCASE("removing keys")
    {
        object obj {};
        obj["a"]      = 100;
        obj["b"]      = false;
        obj["xyz"]    = "testString";
        obj["c"]["d"] = 1;
        obj["c"]["e"] = 2;

        REQUIRE(obj.has("a"));
        REQUIRE(obj.has("b"));
        REQUIRE(obj.has("xyz"));
        REQUIRE(obj.has("c", "d"));
        REQUIRE(obj.has("c", "e"));

        obj["a"]      = nullptr;
        obj["b"]      = nullptr;
        obj["xyz"]    = nullptr;
        obj["c"]["d"] = nullptr;
        obj["c"]["e"] = nullptr;

        REQUIRE_FALSE(obj.has("a"));
        REQUIRE_FALSE(obj.has("b"));
        REQUIRE_FALSE(obj.has("xyz"));
        REQUIRE_FALSE(obj.has("c", "d"));
        REQUIRE_FALSE(obj.has("c", "e"));

        // delete non-existing key
        REQUIRE_FALSE(obj.has("c", "x"));
        obj["c"]["x"]["s"] = nullptr;
        REQUIRE_FALSE(obj.has("c", "x"));
        REQUIRE_FALSE(obj.has("c", "x", "s"));
    }

    SUBCASE("equality")
    {
        using namespace tcob::literals;

        object test {
            R"(
                a = 100
                b = 200
                c = [1,2,3]
                d = {a = 100, b = 300, c = 400}
            )"_ini};

        object good {
            R"(
                a = 100
                b = 200
                c = [1,2,3]
                d = {a = 100, b = 300, c = 400}
            )"_ini};

        REQUIRE(test == good);

        object bad {
            R"(
                a = 100
                b = 200
                c = true
                d = false
            )"_ini};

        REQUIRE_FALSE(test == bad);
    }

    SUBCASE("get_type")
    {
        using namespace tcob::literals;

        object t = R"(
            string = "abc"
            float  = 1.2
            int    = 100
            bool   = true
            array  = [1,2,3]
            object = {a=1,b=2,c=3}
        )"_ini;

        REQUIRE(t.get_type("string") == type::String);
        REQUIRE(t.get_type("float") == type::Float);
        REQUIRE(t.get_type("int") == type::Integer);
        REQUIRE(t.get_type("bool") == type::Bool);
        REQUIRE(t.get_type("array") == type::Array);
        REQUIRE(t.get_type("object") == type::Object);
        REQUIRE(t.get_type("foobar") == type::Null);
    }
}

TEST_CASE("Data.Ini.TcobTypes")
{
    std::string iniString {
        R"(
            point = { x = 100, y = 350 }
            color = { r = 15, g = 30, b = 12, a = 0 }
            size  = { width = 300, height = 450 }
            rect  = { x = 4.5, y = 2.5, width = 30.1, height = 45.01 }
        )"};

    object obj;
    obj.parse(iniString, EXT);

    REQUIRE(obj.is<point_i>("point"));
    REQUIRE(obj["point"].as<point_i>() == point_i {100, 350});

    REQUIRE(obj.is<color>("color"));
    REQUIRE(obj["color"].as<color>() == color {15, 30, 12, 0});

    REQUIRE(obj.is<size_i>("size"));
    REQUIRE(obj["size"].as<size_i>() == size_i {300, 450});

    REQUIRE(obj.is<rect_f>("rect"));
    REQUIRE(obj["rect"].as<rect_f>() == rect_f {4.5f, 2.5f, 30.1f, 45.01f});
}

TEST_CASE("Data.Ini.STLTypes")
{
    std::string iniString {
        R"(
            stringArray     = ["One", "Two", "Three"]
            intArray        = [1, 2, 3]
            stringintMap    = { a = 123, b = 456 }
            variantMap      = { f = 1.5, b = true, s = "ok" }
            duration        = 100
            tuple           = [123, "ok", true]
            pair            = ["ok", 100]
            set             = [1,1,2,2,3,3]
        )"};

    object obj;
    obj.parse(iniString, EXT);

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

TEST_CASE("Data.Ini.ForLoop")
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

TEST_CASE("Data.Ini.DefaultSection")
{
    std::string const iniString =
        R"(
            key1 = 123
            [section1]
            key1 = 456
            [section2]
            key1 = 789
        )";

    {
        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["key1"].as<f64>() == 123);
        REQUIRE(t["section1"]["key1"].as<f64>() == 456);
        REQUIRE(t["section2"]["key1"].as<f64>() == 789);
    }
}

TEST_CASE("Data.Ini.Save")
{
    object save;
    save["key1"]                                                 = 123.;
    save["key.10"]                                               = 321.;
    save["section1"]["valueBool"]                                = true;
    save["sectioN1"]["valueStr"]                                 = "test123";
    save["Section1"]["valueFloat"]                               = 123.45;
    save["section2"]["valueBool"]                                = false;
    save["secTion2"]["valueStr0"]                                = "test4560";
    save["secTion2"]["valueStr1"]                                = "test4561";
    save["secTion2"]["valueStr2"]                                = "test4562";
    save["secTion2"]["valueStr3"]                                = "test4563";
    save["secTion2"]["valueStr4"]                                = "test4564";
    save["secTion2"]["valueInt0"]                                = 16;
    save["secTion2"]["valueInt1"]                                = 256;
    save["secTion2"]["valueInt2"]                                = 32800;
    save["secTion2"]["valueInt3"]                                = 4563;
    save["secTion2"]["valueInt4"]                                = 4564;
    save["section2"]["valueFloat0"]                              = 56.5;
    save["section2"]["valueFloat1"]                              = 156.5;
    save["section2"]["valueFloat2"]                              = 256.782;
    save["section2"]["valueFloat3"]                              = 356.783;
    save["section2"]["valueFloat4"]                              = 456.784;
    save["section2"]["valueFloat5"]                              = 556.785;
    save["section2"]["valueFloat6"]                              = 656.786;
    save["section2"]["valueFloat7"]                              = 756.787;
    save["section2"]["valueFloat8"]                              = 856.788;
    save["section2"]["valueFloat9"]                              = 956.789;
    save["section2"]["valueFloat.10"]                            = 448.789;
    save["section3"]["valueSection"]["a"]                        = 1;
    save["section3"]["valueSection"]["b"]                        = "a";
    save["section3"]["valueSection"]["xyz"]                      = true;
    save["section3"]["valueSection"]["subsection"]["a"]          = 100;
    save["section3"]["valueSection"]["subsection"]["a.b"]["x.y"] = 100;

    object arraySubSection;
    arraySubSection["ay"] = 123;
    arraySubSection["xy"] = 436;

    array arraySubArray;
    arraySubArray.add("O");
    arraySubArray.add("K");

    array saveArray;
    saveArray.add("a");
    saveArray.add(1);
    saveArray.add(false);
    saveArray.add(arraySubSection);
    saveArray.add(arraySubArray);
    save["section3"]["valueArray"] = saveArray;

    SUBCASE("Text object")
    {
        std::string const file {"test" + EXT};

        {
            io::delete_file(file);
            save.save(file);
        }

        {
            object load;
            REQUIRE(load.load(file) == load_status::Ok);
            REQUIRE(load["key1"].as<f64>() == 123);
            REQUIRE(load["key.10"].as<f64>() == 321);
            REQUIRE(load["section1"]["valueBool"].as<bool>() == true);
            REQUIRE(load["section1"]["valueStr"].as<std::string>() == "test123");
            REQUIRE(load["section1"]["valueFloat"].as<f64>() == 123.45);

            REQUIRE(load["section2"]["valueBool"].as<bool>() == false);
            REQUIRE(load["section2"]["valueStr0"].as<std::string>() == "test4560");
            REQUIRE(load["section2"]["valueStr1"].as<std::string>() == "test4561");
            REQUIRE(load["section2"]["valueStr2"].as<std::string>() == "test4562");
            REQUIRE(load["section2"]["valueStr3"].as<std::string>() == "test4563");
            REQUIRE(load["section2"]["valueStr4"].as<std::string>() == "test4564");
            REQUIRE(load["section2"]["valueInt0"].as<i64>() == 16);
            REQUIRE(load["section2"]["valueInt1"].as<i64>() == 256);
            REQUIRE(load["section2"]["valueInt2"].as<i64>() == 32800);
            REQUIRE(load["section2"]["valueInt3"].as<i64>() == 4563);
            REQUIRE(load["section2"]["valueInt4"].as<i64>() == 4564);
            REQUIRE(load["section2"]["valueFloat0"].as<f64>() == 56.5);
            REQUIRE(load["section2"]["valueFloat1"].as<f64>() == 156.5);
            REQUIRE(load["section2"]["valueFloat2"].as<f64>() == 256.782);
            REQUIRE(load["section2"]["valueFloat3"].as<f64>() == 356.783);
            REQUIRE(load["section2"]["valueFloat4"].as<f64>() == 456.784);
            REQUIRE(load["section2"]["valueFloat5"].as<f64>() == 556.785);
            REQUIRE(load["section2"]["valueFloat6"].as<f64>() == 656.786);
            REQUIRE(load["section2"]["valueFloat7"].as<f64>() == 756.787);
            REQUIRE(load["section2"]["valueFloat8"].as<f64>() == 856.788);
            REQUIRE(load["section2"]["valueFloat9"].as<f64>() == 956.789);
            REQUIRE(load["section2"]["valueFloat.10"].as<f64>() == 448.789);
            REQUIRE(load["section3"]["valueArray"].as<array>().get_size() == 5);
            REQUIRE(load["section3"]["valueArray"][0].as<std::string>() == "a");
            REQUIRE(load["section3"]["valueArray"][1].as<f64>() == 1);
            REQUIRE(load["section3"]["valueArray"][2].as<bool>() == false);
            REQUIRE(load["section3"]["valueArray"][3].as<object>()["ay"].as<i64>() == 123);
            REQUIRE(load["section3"]["valueArray"][3].as<object>()["xy"].as<i64>() == 436);
            REQUIRE(load["section3"]["valueArray"][4].as<array>()[0].as<std::string>() == "O");
            REQUIRE(load["section3"]["valueArray"][4].as<array>()[1].as<std::string>() == "K");

            REQUIRE(load["section3"]["valueSection"]["a"].as<f64>() == 1);
            REQUIRE(load["section3"]["valueSection"]["b"].as<std::string>() == "a");
            REQUIRE(load["section3"]["valueSection"]["xyz"].as<bool>() == true);

            REQUIRE(load["section3"]["valueSection"]["subsection"]["a"].as<i64>() == 100);
            REQUIRE(load["section3"]["valueSection"]["subsection"]["a.b"]["x.y"].as<i64>() == 100);
        }
    }

    SUBCASE("Text array")
    {

        std::string const file {"test2" + EXT};

        {
            io::delete_file(file);
            saveArray.save(file);
        }

        {
            array load;
            REQUIRE(load.load(file) == load_status::Ok);

            REQUIRE(load.get_size() == 5);
            REQUIRE(load[0].as<std::string>() == "a");
            REQUIRE(load[1].as<f64>() == 1);
            REQUIRE(load[2].as<bool>() == false);
            REQUIRE(load[3].as<object>()["ay"].as<i64>() == 123);
            REQUIRE(load[3].as<object>()["xy"].as<i64>() == 436);
            REQUIRE(load[4].as<array>()[0].as<std::string>() == "O");
            REQUIRE(load[4].as<array>()[1].as<std::string>() == "K");
        }
    }

    SUBCASE("Binary object")
    {

        std::string const file {"test.bsbd"};

        {
            io::delete_file(file);
            save.save(file);
        }

        {
            object load;
            REQUIRE(load.load(file) == load_status::Ok);
            REQUIRE(load["key1"].as<f64>() == 123);

            REQUIRE(load["section1"]["valueBool"].as<bool>() == true);
            REQUIRE(load["section1"]["valueStr"].as<std::string>() == "test123");
            REQUIRE(load["section1"]["valueFloat"].as<f64>() == Approx(123.45));

            REQUIRE(load["section2"]["valueBool"].as<bool>() == false);
            REQUIRE(load["section2"]["valueStr0"].as<std::string>() == "test4560");
            REQUIRE(load["section2"]["valueStr1"].as<std::string>() == "test4561");
            REQUIRE(load["section2"]["valueStr2"].as<std::string>() == "test4562");
            REQUIRE(load["section2"]["valueStr3"].as<std::string>() == "test4563");
            REQUIRE(load["section2"]["valueStr4"].as<std::string>() == "test4564");
            REQUIRE(load["section2"]["valueInt0"].as<i64>() == 16);
            REQUIRE(load["section2"]["valueInt1"].as<i64>() == 256);
            REQUIRE(load["section2"]["valueInt2"].as<i64>() == 32800);
            REQUIRE(load["section2"]["valueInt3"].as<i64>() == 4563);
            REQUIRE(load["section2"]["valueInt4"].as<i64>() == 4564);
            REQUIRE(load["section2"]["valueFloat0"].as<f64>() == 56.5);
            REQUIRE(load["section2"]["valueFloat1"].as<f64>() == 156.5);
            REQUIRE(load["section2"]["valueFloat2"].as<f64>() == 256.782);
            REQUIRE(load["section2"]["valueFloat3"].as<f64>() == 356.783);
            REQUIRE(load["section2"]["valueFloat4"].as<f64>() == 456.784);
            REQUIRE(load["section2"]["valueFloat5"].as<f64>() == 556.785);
            REQUIRE(load["section2"]["valueFloat6"].as<f64>() == 656.786);
            REQUIRE(load["section2"]["valueFloat7"].as<f64>() == 756.787);
            REQUIRE(load["section2"]["valueFloat8"].as<f64>() == 856.788);
            REQUIRE(load["section2"]["valueFloat9"].as<f64>() == 956.789);

            REQUIRE(load["section3"]["valueArray"].as<array>().get_size() == 5);
            REQUIRE(load["section3"]["valueArray"][0].as<std::string>() == "a");
            REQUIRE(load["section3"]["valueArray"][1].as<f64>() == 1);
            REQUIRE(load["section3"]["valueArray"][2].as<bool>() == false);
            REQUIRE(load["section3"]["valueArray"][3].as<object>()["ay"].as<i64>() == 123);
            REQUIRE(load["section3"]["valueArray"][3].as<object>()["xy"].as<i64>() == 436);
            REQUIRE(load["section3"]["valueArray"][4].as<array>()[0].as<std::string>() == "O");
            REQUIRE(load["section3"]["valueArray"][4].as<array>()[1].as<std::string>() == "K");

            REQUIRE(load["section3"]["valueSection"]["a"].as<f64>() == 1);
            REQUIRE(load["section3"]["valueSection"]["b"].as<std::string>() == "a");
            REQUIRE(load["section3"]["valueSection"]["xyz"].as<bool>() == true);

            REQUIRE(load["section3"]["valueSection"]["subsection"]["a"].as<i64>() == 100);
        }
    }

    SUBCASE("Binary array")
    {

        std::string const file {"test2.bsbd"};

        {
            io::delete_file(file);
            saveArray.save(file);
        }

        {
            array load;
            REQUIRE(load.load(file) == load_status::Ok);

            REQUIRE(load.get_size() == 5);
            REQUIRE(load[0].as<std::string>() == "a");
            REQUIRE(load[1].as<f64>() == 1);
            REQUIRE(load[2].as<bool>() == false);
            REQUIRE(load[3].as<object>()["ay"].as<i64>() == 123);
            REQUIRE(load[3].as<object>()["xy"].as<i64>() == 436);
            REQUIRE(load[4].as<array>()[0].as<std::string>() == "O");
            REQUIRE(load[4].as<array>()[1].as<std::string>() == "K");
        }
    }
}

TEST_CASE("Data.Ini.Parse")
{
    REQUIRE(object::Parse("[x]\na=a", EXT));
    REQUIRE(object::Parse("[x]", EXT));
    REQUIRE(object::Parse("a=a", EXT));
    REQUIRE(object::Parse("a.a=a", EXT));
    REQUIRE(object::Parse("", EXT));

    REQUIRE_FALSE(object::Parse("a=", EXT));
    REQUIRE_FALSE(object::Parse("=a", EXT));
    REQUIRE_FALSE(object::Parse("=", EXT));
    REQUIRE_FALSE(object::Parse("[]", EXT));
    REQUIRE_FALSE(object::Parse("[asdsa\na=a", EXT));
    REQUIRE_FALSE(object::Parse("asdasdas", EXT));
    REQUIRE_FALSE(object::Parse(".=a", EXT));
    REQUIRE_FALSE(object::Parse("a.=a", EXT));
}

TEST_CASE("Data.Ini.DuplicateKey")
{
    std::string const iniString =
        R"(
            [section1]
            key = 100
            [section2]
            key = 123
            [section1]
            key = 245
        )";

    object t;
    REQUIRE(t.parse(iniString, EXT));
    REQUIRE(t["section1"]["key"].as<f64>() == 245);
}

TEST_CASE("Data.Ini.Comments")
{
    {
        std::string const iniString =
            R"(
            [section1]
            ;comment1
            a = 1
            b = 2
            ;comment2
            c = 3
            [section2]          
            d = 4
            #comment3
            e = 5
            f = 6
        )";

        object t;
        REQUIRE(t.parse(iniString, EXT));

        REQUIRE(t.as<object>("section1").get_entry("a")->get_comment().Text == "comment1\n");
        REQUIRE(t.as<object>("section1").get_entry("c")->get_comment().Text == "comment2\n");
        REQUIRE(t.as<object>("section2").get_entry("e")->get_comment().Text == "comment3\n");
    }
    {
        std::string const iniString =
            R"(
            [section1]
            ;comment1
            ;comment2
            a = 1
        )";

        object t;
        REQUIRE(t.parse(iniString, EXT));

        REQUIRE(t.as<object>("section1").get_entry("a")->get_comment().Text == "comment1\ncomment2\n");
    }
}

TEST_CASE("Data.Ini.Literals")
{
    using namespace tcob::literals;

    object t = R"(
            [section1]
            ;comment1
            valueBool  = true
            valueStr   = test123
            valueSec   = { a = 100, b = false, c = { l = 1, m = 32 } }
            valueArr   = [ 3, 5, 9, 13 ]
            ;comment1b
            valueFloat = 123.45
            [section2]
            #comment2
            valueBool  = false
            valueStr   = "test456"
            valueFloat = 456.78
        )"_ini;

    REQUIRE(t.as<bool>("section1", "valueBool") == true);
    REQUIRE(t.as<std::string>("section1", "valueStr") == "test123");
    REQUIRE(t.as<f64>("section1", "valueFloat") == 123.45);
    REQUIRE(t.as<f64>("section1", "valueSec", "a") == 100);
    REQUIRE(t.as<bool>("section1", "valueSec", "b") == false);
    REQUIRE(t.as<f64>("section1", "valueSec", "c", "l") == 1);
    REQUIRE(t.as<f64>("section1", "valueArr", 2) == 9);
}

TEST_CASE("Data.Ini.Schema")
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

TEST_CASE("Data.Ini.Enum")
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
