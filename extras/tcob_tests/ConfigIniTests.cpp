#include "tests.hpp"
#include <string>

using namespace tcob::data::config;

static std::string const EXT {".ini"};

TEST_CASE("Data.Ini.Reference")
{
    SUBCASE("ref value")
    {
        std::string const iniString =
            R"([section1]
                   b = { a = 100 }
                   c = @section1.b.a       
                   b = { a = 75 }
                )";

        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["section1"]["c"].as<i32>() == 100);
        REQUIRE(t["section1"]["b"]["a"].as<i32>() == 75);
    }
    SUBCASE("ref section")
    {
        std::string const iniString =
            R"(
            [section1]
                b = { a = 100, c = 300 }
            [section2]
                c = @section1.b
                d = @section1
                d.b.a = 200
            )";

        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["section1"]["b"]["a"].as<i32>() == 100);
        REQUIRE(t["section2"]["c"]["a"].as<i32>() == 100);
        REQUIRE(t["section2"]["c"]["c"].as<i32>() == 300);
        REQUIRE(t["section2"]["d"]["b"]["a"].as<i32>() == 200);
        REQUIRE(t["section2"]["d"]["b"]["c"].as<i32>() == 300);
    }
    SUBCASE("ref in inline section")
    {
        std::string const iniString =
            R"(
            [section1]
                b = { a = 100, c = 300 }
            [section2]
                c = { d = @section1.b.c }
            )";

        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["section2"]["c"]["d"].as<i32>() == 300);
    }
    SUBCASE("inheritance")
    {
        std::string const iniString =
            R"([section1]
                    b = { a = 100 }
               [sect@ion1]
                    b = 444
               [section2] @section1
                    c = 240
                )";

        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["sect@ion1"]["b"].as<i32>() == 444);
        REQUIRE(t["section2"]["b"]["a"].as<i32>() == 100);
        REQUIRE(t["section2"]["c"].as<i32>() == 240);
    }
    SUBCASE("unknown ref")
    {
        std::string const iniString =
            R"([section1]
                   b = { a = 100 }
                   c = @section2
                )";

        object t;
        REQUIRE_FALSE(t.parse(iniString, EXT));
    }
}

TEST_CASE("Data.Ini.Array")
{
    SUBCASE("parse")
    {
        {
            std::string const arrString {"[1,2,3,a,b,c]"};

            auto arr {array::Parse(arrString, EXT)};
            REQUIRE(arr);
            REQUIRE(arr->size() == 6);
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
            REQUIRE(arr->size() == 0);
        }
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

            multiLineString0 = '
  abc
  def 
  ghi
'  
            multiLineString1 = '
  abc
   def 

  ghi'  

            multiLineString2 = "abc
                                def
                                ghi"      
            multiLineString3 = "abc


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

    REQUIRE(t["section1"]["multiLineString0"].is<std::string>());
    REQUIRE(t["section1"]["multiLineString0"].as<std::string>() == "  abc\n  def \n  ghi");
    REQUIRE(t["section1"]["multiLineString1"].is<std::string>());
    REQUIRE(t["section1"]["multiLineString1"].as<std::string>() == "  abc\n   def \n\n  ghi");
    REQUIRE(t["section1"]["multiLineString2"].is<std::string>());
    REQUIRE(t["section1"]["multiLineString2"].as<std::string>() == "abc\ndef\nghi");
    REQUIRE(t["section1"]["multiLineString3"].is<std::string>());
    REQUIRE(t["section1"]["multiLineString3"].as<std::string>() == "abc\n\n\ndef\n\nghi");
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
            string1 = 'abcdefghi'  

            [section1.subsection]
            a = 100
            b = 500

            [section1.subsection.subsection]
            x = 300
            y = 600
            c = @
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
        REQUIRE(t["section1"]["subsection"]["subsection"]["c"].as<std::string>() == "@");

        REQUIRE(t["section1"]["string1"].as<string>() == "abcdefghi");

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
        std::string const iniString =
            R"(
            [section1]
                b = { a = 100 }
            [section1.c]
                a = 100)";

        object t;
        REQUIRE(t.parse(iniString, EXT));
        REQUIRE(t["section1"]["c"].is<object>());
        REQUIRE(t["section1"]["c"]["a"].as<i32>() == 100);
    }

    SUBCASE("empty section")
    {
        {
            std::string const iniString = R"([section1])";

            object t;
            REQUIRE(t.parse(iniString, EXT));
            REQUIRE(t["section1"].is<object>());
        }
        {
            std::string const iniString = R"(sec = { })";

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
            REQUIRE(load["section3"]["valueArray"].as<array>().size() == 5);
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

            REQUIRE(load.size() == 5);
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

            REQUIRE(load["section3"]["valueArray"].as<array>().size() == 5);
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

            REQUIRE(load.size() == 5);
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
    REQUIRE(object::Parse("'a=b'=1", EXT));

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
            valueStr   = 'test123'
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
