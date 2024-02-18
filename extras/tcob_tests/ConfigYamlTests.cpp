#include "tests.hpp"

using namespace tcob::data::config;

static std::string const EXT {".yaml"};

TEST_CASE("Data.Yaml.Save")
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
            save["key1"]             = 123.;
            save["section1"]["key1"] = 456.;
            save["section2"]["key1"] = 789.;

            io::delete_file(file);
            save.save(file);

            {
                object t;
                REQUIRE(t.load(file) == load_status::Ok);
                REQUIRE(t["key1"].as<f64>() == 123);
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

TEST_CASE("Data.Yaml.Parse")
{
    SUBCASE("object")
    {
        static std::string yamlString {
            R"(
---
#first
string: Test
number: 12 #trailing
float: 1.2
bool:
  true
stringArray:
  - One
  - Two
  - Three
object:
  childString1: foo1
  childString2: "foo2"
  childString3: 'foo3'
  point:
    x: 100
    y: 350
  childNumber: 77453
map: &anc
  a: 123
  b: 456
map1: *anc
map2:
    ? a
    : 456
    ? b
    : 123
ml:
  12
  34
  56
ml1: >
  12
  34
  56
ml2: |
  12
  34
  56
intArray:
- 1
- 2
- 3
tmap:
  !!int 1: !!str "a"
  !!int 2: !!str "b"
mapArray:
- *anc
- *anc
flowseq: [1,2,3]
flowmap: {"a": 1, "b": 2})"};

        object obj;
        REQUIRE(obj.parse(yamlString, EXT));

        REQUIRE(obj["string"].as<std::string>() == "Test");
        REQUIRE(obj["ml"].as<std::string>() == "12 34 56");
        REQUIRE(obj["ml1"].as<std::string>() == "12 34 56\n");
        REQUIRE(obj["ml2"].as<std::string>() == "12\n34\n56\n");
        REQUIRE(obj["number"].as<i32>() == 12);
        REQUIRE(obj["float"].as<f64>() == 1.2);
        REQUIRE(obj["bool"].as<bool>() == true);
        REQUIRE(obj["object"]["childString1"].as<std::string>() == "foo1");
        REQUIRE(obj["object"]["childString2"].as<std::string>() == "foo2");
        REQUIRE(obj["object"]["childString3"].as<std::string>() == "foo3");
        REQUIRE(obj["object"]["point"]["x"].as<i32>() == 100);
        REQUIRE(obj["object"]["point"]["y"].as<i32>() == 350);
        REQUIRE(obj["object"]["childNumber"].as<i32>() == 77453);
        REQUIRE(obj["map"]["a"].as<i32>() == 123);
        REQUIRE(obj["map"]["b"].as<i32>() == 456);
        REQUIRE(obj["map1"]["a"].as<i32>() == 123);
        REQUIRE(obj["map1"]["b"].as<i32>() == 456);
        REQUIRE(obj["map2"]["a"].as<i32>() == 456);
        REQUIRE(obj["map2"]["b"].as<i32>() == 123);
        REQUIRE(obj["stringArray"][0].as<std::string>() == "One");
        REQUIRE(obj["stringArray"][1].as<std::string>() == "Two");
        REQUIRE(obj["stringArray"][2].as<std::string>() == "Three");
        REQUIRE(obj["intArray"][0].as<i32>() == 1);
        REQUIRE(obj["intArray"][1].as<i32>() == 2);
        REQUIRE(obj["intArray"][2].as<i32>() == 3);
        object sec0 {obj["mapArray"][0].as<object>()};
        REQUIRE(sec0["a"].as<i32>() == 123);
        REQUIRE(sec0["b"].as<i32>() == 456);
        object sec1 {obj["mapArray"][1].as<object>()};
        REQUIRE(sec1["a"].as<i32>() == 123);
        REQUIRE(sec1["b"].as<i32>() == 456);
        REQUIRE(obj["flowseq"][0].as<i32>() == 1);
        REQUIRE(obj["flowseq"][1].as<i32>() == 2);
        REQUIRE(obj["flowseq"][2].as<i32>() == 3);
        REQUIRE(obj["flowmap"]["a"].as<i32>() == 1);
        REQUIRE(obj["flowmap"]["b"].as<i32>() == 2);
    }
    SUBCASE("array")
    {
        std::string const arrString =
            R"(
- 1
- 2
- 3
- a
- b
- c
        )";

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
    SUBCASE("object2")
    {
        static std::string yamlString {
            R"(
---
a: 
  ab: 
    ac: 
      - 123
b: 
  bb: 
    bc: 456
)"};

        object obj;
        REQUIRE(obj.parse(yamlString, EXT));

        REQUIRE(obj["a"]["ab"]["ac"][0].as<i32>() == 123);
        REQUIRE(obj["b"]["bb"]["bc"].as<i32>() == 456);
    }
    SUBCASE("object3")
    {
        static std::string yamlString {
            R"(
---
a: 
  ab: 
    ac: 
      123
b: 
  bb: 
    bc: 456
    bd: 789
c: 
  cb: 
    cc: 456
)"};

        object obj;
        REQUIRE(obj.parse(yamlString, EXT));

        REQUIRE(obj["a"]["ab"]["ac"].as<i32>() == 123);
        REQUIRE(obj["b"]["bb"]["bc"].as<i32>() == 456);
        REQUIRE(obj["b"]["bb"]["bd"].as<i32>() == 789);
        REQUIRE(obj["c"]["cb"]["cc"].as<i32>() == 456);
    }

    SUBCASE("object4")
    {
        static std::string yamlString {
            R"(
x: 
  - 1
  -
    - 2
  - 3
y:
  34
)"};
        object obj;
        REQUIRE(obj.parse(yamlString, EXT));
        REQUIRE(obj["x"].as<array>().get_size() == 3);
        REQUIRE(obj["x"][0].as<i32>() == 1);
        REQUIRE(obj["x"][1].as<array>().get_size() == 1);
        REQUIRE(obj["x"][1].as<array>()[0].as<i32>() == 2);
        REQUIRE(obj["x"][2].as<i32>() == 3);
        REQUIRE(obj["y"].as<i32>() == 34);
    }
}
