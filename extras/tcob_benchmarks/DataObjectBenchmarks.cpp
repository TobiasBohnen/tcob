#include "bench.hpp"
#include <string>

using namespace tcob::scripting::lua;

////////////////////////////////////////////////////////////

void static INI_Parse(benchmark::State& state)
{
    static std::string iniString {
        R"(
            string = "Test"
            number = 12
            bool = true
            object = {
               childString = "foo",
               childNumber = 77453
            }
            map = {
               a = 123,
               b = 456
            }
            stringArray = ["One", "Two", "Three"]
            intArray = [1, 2, 3]
            point = { "x" = 100, "y" = 350 }
        )"};
    data::config::object obj;

    for (auto _ : state) {
        auto x = obj.parse(iniString, ".ini");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(INI_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static BINI_Parse(benchmark::State& state)
{
    static std::string iniString {
        R"(
            string = "Test"
            number = 12
            bool = true
            object = {
               childString = "foo",
               childNumber = 77453
            }
            map = {
               a = 123,
               b = 456
            }
            stringArray = ["One", "Two", "Three"]
            intArray = [1, 2, 3]
            point = { "x" = 100, "y" = 350 }
        )"};
    data::config::object obj;
    obj.parse(iniString, ".ini");

    io::iomstream stream;
    obj.save(stream, ".bsbd");

    for (auto _ : state) {
        auto x = obj.load(stream, ".bsbd");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BINI_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static XML_Parse(benchmark::State& state)
{
    static std::string xmlString {
        R"(
        <root>
            <string>Test</string>
            <number>12</number>
            <bool>true</bool>
            <object>
                <childString>foo</childString>
                <childNumber>77453</childNumber>
            </object>
            <map>
                <a>123</a>
                <b>456</b>
            </map>
            <stringArray>
                <item>One</item>
                <item>Two</item>
                <item>Three</item>
            </stringArray>
            <intArray>
                <item>1</item>
                <item>2</item>
                <item>3</item>
            </intArray>
            <point>
                <x>100</x>
                <y>350</y>
            </point>
        </root>
)"};
    data::config::object obj;

    for (auto _ : state) {
        auto x = obj.parse(xmlString, ".xml");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(XML_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static JSON_Parse(benchmark::State& state)
{
    static std::string jsonString {
        R"({
            "string": "Test",
            "number": 12,
            "bool": true,
            "object": {
               "childString": "foo",
               "childNumber": 77453
            },
            "map": {
               "a": 123,
               "b": 456
            },
            "stringArray": ["One", "Two", "Three"],
            "intArray": [1, 2, 3],
            "point": { "x": 100, "y": 350 }
        })"};
    data::config::object obj;

    for (auto _ : state) {
        auto x = obj.parse(jsonString, ".json");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(JSON_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static YAML_Parse(benchmark::State& state)
{
    static std::string yamlString {
        R"(---
string: Test
number: 12
bool: true
object:
  childString: foo
  childNumber: 77453
map:
  a: 123
  b: 456
stringArray:
- One
- Two
- Three
intArray:
- 1
- 2
- 3
point:
  x: 100
  y: 350
)"};
    data::config::object obj;

    for (auto _ : state) {
        auto x = obj.parse(yamlString, ".yaml");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(YAML_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static LUA_Parse(benchmark::State& state)
{
    static std::string luaString {
        R"(return {
            string = "Test",
            number = 12,
            bool = true,
            object = {
               childString = "foo",
               childNumber = 77453
            },
            map = {
               a = 123,
               b = 456
            },
            stringArray = {"One", "Two", "Three"},
            intArray = {1, 2, 3},
            point = { x = 100, y = 350 }
        })"};
    scripting::lua::script scr;

    for (auto _ : state) {
        auto x = scr.run(luaString);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(LUA_Parse)->Iterations(1000000);

////////////////////////////////////////////////////////////

void static SQUIRREL_Parse(benchmark::State& state)
{
    static std::string nutString {
        R"(return a <- {
            string = "Test",
            number = 12,
            bool = true,
            object = {
               childString = "foo",
               childNumber = 77453
            },
            map = {
               a = 123,
               b = 456
            },
            stringArray = ["One", "Two", "Three"],
            intArray = [1, 2, 3],
            point = { x = 100, y = 350 }
        })"};
    scripting::squirrel::script scr;

    for (auto _ : state) {
        auto x = scr.run(nutString);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(SQUIRREL_Parse)->Iterations(1000000);