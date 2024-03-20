#include "tests.hpp"

using namespace tcob::data::config;

static std::string const EXT {".json"};

TEST_CASE("Data.Json.Save")
{
    SUBCASE("Text object")
    {
        {
            std::string const file {"test" + EXT};

            object save;
            save["section1"]["valueBool"]                       = true;
            save["section1"]["valueStr"]                        = "test123";
            save["section1"]["valueFloat"]                      = 123.45;
            save["section2"]["valueBool"]                       = false;
            save["section2"]["valueStr"]                        = "test456";
            save["section2"]["valueFloat"]                      = 456.78;
            save["section3"]["valueSection"]["a"]               = 1;
            save["section3"]["valueSection"]["b"]               = "a";
            save["section3"]["valueSection"]["xyz"]             = true;
            save["section3"]["valueSection"]["subsection"]["a"] = 100;

            save["section4"]["emptySection"] = object {};
            save["section4"]["emptyArray"]   = array {};

            object arraySubSection;
            arraySubSection["ay"] = 123;
            arraySubSection["xy"] = 436;

            array saveArray;
            saveArray.add("a");
            saveArray.add(1);
            saveArray.add(false);
            saveArray.add(arraySubSection);
            save["section3"]["valueArray"] = saveArray;

            io::delete_file(file);
            save.save(file);

            {
                object load;
                REQUIRE(load.load(file) == load_status::Ok);
                REQUIRE(load["section1"]["valueBool"].as<bool>() == true);
                REQUIRE(load["section1"]["valueStr"].as<std::string>() == "test123");
                REQUIRE(load["section1"]["valueFloat"].as<f64>() == 123.45);
                REQUIRE(load["section2"]["valueBool"].as<bool>() == false);
                REQUIRE(load["section2"]["valueStr"].as<std::string>() == "test456");
                REQUIRE(load["section2"]["valueFloat"].as<f64>() == 456.78);

                REQUIRE(load["section3"]["valueArray"].as<array>().get_size() == 4);
                REQUIRE(load["section3"]["valueArray"][0].as<std::string>() == "a");
                REQUIRE(load["section3"]["valueArray"][1].as<f64>() == 1);
                REQUIRE(load["section3"]["valueArray"][2].as<bool>() == false);
                REQUIRE(load["section3"]["valueArray"][3].as<object>()["ay"].as<i64>() == 123);
                REQUIRE(load["section3"]["valueArray"][3].as<object>()["xy"].as<i64>() == 436);

                REQUIRE(load["section3"]["valueSection"]["a"].as<f64>() == 1);
                REQUIRE(load["section3"]["valueSection"]["b"].as<std::string>() == "a");
                REQUIRE(load["section3"]["valueSection"]["xyz"].as<bool>() == true);

                REQUIRE(load["section3"]["valueSection"]["subsection"]["a"].as<i64>() == 100);
            }
        }

        {
            std::string const file {"test2" + EXT};

            object save;
            save[""]["key1"]         = 123.;
            save["section1"]["key1"] = 456.;
            save["section2"]["key1"] = 789.;

            io::delete_file(file);
            save.save(file);

            {
                object t;
                REQUIRE(t.load(file) == load_status::Ok);
                REQUIRE(t[""]["key1"].as<f64>() == 123);
                REQUIRE(t["section1"]["key1"].as<f64>() == 456);
                REQUIRE(t["section2"]["key1"].as<f64>() == 789);
            }
        }
    }

    SUBCASE("Text array")
    {
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

        std::string const file {"test3" + EXT};

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

TEST_CASE("Data.Json.Literals")
{
    using namespace tcob::literals;
    {
        auto object = R"( { "x": 100 } )"_json;

        REQUIRE(object.has("x"));
        REQUIRE(object["x"].as<i32>() == 100);
    }
}

TEST_CASE("Data.Json.Parse")
{
    SUBCASE("object")
    {
        static std::string jsonString {
            R"({
            "string": "Test",
            "number": 12,
            "float": 1.2,
            "bool": true,
            "object": {
               "childString": "foo",
               "point": { "x": 100, "y": 350 },
               "childNumber": 77453
            },
            "map": {
               "a": 123,
               "b": 456
            },
            "stringArray": ["One", "Two", "Three"],
            "intArray": [1, 2, 3],
            "bla": null
        })"};

        object obj;
        REQUIRE(obj.parse(jsonString, EXT));

        REQUIRE(obj["string"].as<std::string>() == "Test");
        REQUIRE(obj["number"].as<i32>() == 12);
        REQUIRE(obj["float"].as<f64>() == 1.2);
        REQUIRE(obj["bool"].as<bool>() == true);
        REQUIRE(obj["object"]["childString"].as<std::string>() == "foo");
        REQUIRE(obj["object"]["point"]["x"].as<i32>() == 100);
        REQUIRE(obj["object"]["point"]["y"].as<i32>() == 350);
        REQUIRE(obj["object"]["childNumber"].as<i32>() == 77453);
        REQUIRE(obj["map"]["a"].as<i32>() == 123);
        REQUIRE(obj["map"]["b"].as<i32>() == 456);
        REQUIRE(obj["stringArray"][0].as<std::string>() == "One");
        REQUIRE(obj["stringArray"][1].as<std::string>() == "Two");
        REQUIRE(obj["stringArray"][2].as<std::string>() == "Three");
        REQUIRE(obj["intArray"][0].as<i32>() == 1);
        REQUIRE(obj["intArray"][1].as<i32>() == 2);
        REQUIRE(obj["intArray"][2].as<i32>() == 3);
        REQUIRE_FALSE(obj.has("bla"));
    }

    SUBCASE("array")
    {
        std::string const arrString =
            R"([1,2,3,"a","b","c"])";

        array arr;
        REQUIRE(arr.parse(arrString, EXT));
        REQUIRE(arr.get_size() == 6);
        REQUIRE(arr[0].as<i64>() == 1);
        REQUIRE(arr[1].as<i64>() == 2);
        REQUIRE(arr[2].as<i64>() == 3);
        REQUIRE(arr[3].as<std::string>() == "a");
        REQUIRE(arr[4].as<std::string>() == "b");
        REQUIRE(arr[5].as<std::string>() == "c");
    }
    SUBCASE("empty object")
    {
        static std::string emptyObj {R"({ "object": { } })"};
        object             obj;
        REQUIRE(obj.parse(emptyObj, EXT));
        REQUIRE(obj["object"].is<object>());
    }
    SUBCASE("empty array")
    {
        static std::string emptyObj {R"({ "array": [] })"};
        object             obj;
        REQUIRE(obj.parse(emptyObj, EXT));
        REQUIRE(obj["array"].is<array>());
    }
}

TEST_CASE("Data.Json.TcobTypes")
{
    std::string json {
        R"({
            "point": { "x": 100, "y": 350 },
            "color": { "r": 15, "g": 30, "b": 12, "a": 0 },
            "size": { "width": 300, "height": 450 },
            "rect": { "x": 4.5, "y": 2.5, "width": 30.10, "height": 45.01 }
        })"};

    object obj;
    obj.parse(json, EXT);

    REQUIRE(obj.is<point_i>("point"));
    REQUIRE(obj["point"].as<point_i>() == point_i {100, 350});

    REQUIRE(obj.is<color>("color"));
    REQUIRE(obj["color"].as<color>() == color {15, 30, 12, 0});

    REQUIRE(obj.is<size_i>("size"));
    REQUIRE(obj["size"].as<size_i>() == size_i {300, 450});

    REQUIRE(obj.is<rect_f>("rect"));
    REQUIRE(obj["rect"].as<rect_f>() == rect_f {4.5f, 2.5f, 30.1f, 45.01f});
}

TEST_CASE("Data.Json.TestSuite")
{
    auto const files(io::enumerate("testfiles/json/", {"*" + EXT}, false));
    REQUIRE(files.size() == 237);
    for (auto const& file : files) {
        object     obj;
        auto const objStatus {obj.load(file, true) == load_status::Ok};
        array      arr;
        auto const arrStatus {arr.load(file) == load_status::Ok};

        auto const success {objStatus || arrStatus};
        auto const shouldSucceed {io::get_stem(file)[0] == 'y'};

        if (success != shouldSucceed) {
            FAIL(file);
        }
    }
}
