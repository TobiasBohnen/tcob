// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_json.hpp"

#include <charconv>

#include "tcob/core/StringUtils.hpp"

namespace tcob::data::config::detail {

constexpr i32 INDENT_SPACES {2};

auto json_reader::read_as_object(utf8_string_view txt) -> std::optional<object>
{
    entry currentEntry;
    return ReadObject(currentEntry, helper::trim(txt)) ? std::optional<object> {currentEntry.as<object>()} : std::nullopt;
}

auto json_reader::read_as_array(utf8_string_view txt) -> std::optional<array>
{
    entry currentEntry;
    return ReadArray(currentEntry, helper::trim(txt)) ? std::optional<array> {currentEntry.as<array>()} : std::nullopt;
}

auto json_reader::ReadKeyValuePair(entry& currentEntry, object& obj, utf8_string_view line) -> bool
{
    auto const separatorPos {helper::find_unquoted(line, ':')};
    if (separatorPos == utf8_string::npos) { return false; } // ERROR: invalid pair

    auto keyStr {helper::trim(line.substr(0, separatorPos))};
    auto valueStr {helper::trim(line.substr(separatorPos + 1))};

    if (keyStr.size() <= 1 || valueStr.empty()) { return false; }                 //  ERROR: empty key or value
    if (keyStr[0] != '\"' || keyStr[keyStr.size() - 1] != '\"') { return false; } //  ERROR: invalid key
    if (valueStr == "null") { return true; }                                      // ignore nulled keys

    // read value string
    if (!ReadValue(currentEntry, valueStr)) { return false; } // ERROR: invalid value

    // unescape key
    utf8_string const valueKey {keyStr.substr(1, keyStr.size() - 2)};
    obj.set_entry(valueKey, currentEntry);
    return true;
}

auto json_reader::ReadValue(entry& currentEntry, utf8_string_view line) -> bool
{
    return !line.empty()
        && (ReadArray(currentEntry, line)
            || ReadObject(currentEntry, line)
            || ReadNumber(currentEntry, line)
            || ReadBool(currentEntry, line)
            || ReadString(currentEntry, line));
}

auto json_reader::ReadArray(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line[0] == '[') {
        if (line[line.size() - 1] != ']') {
            return false;
        }

        array arr {};

        auto splitLine {helper::trim(line.substr(1, line.size() - 2))};
        if (splitLine.empty()) {
            currentEntry.set_value(arr);
            return true;
        }

        if (!helper::split_preserve_brackets_for_each(
                splitLine, ',',
                [&arr](utf8_string_view token) {
                    auto const tk {helper::trim(token)};
                    if (tk == "null") { return true; } // ignore nulled keys

                    entry arrvalue;
                    if (ReadValue(arrvalue, tk)) {
                        arr.add_entry(arrvalue);
                        return true;
                    }

                    return false;
                })) {
            return false;
        }

        currentEntry.set_value(arr);
        return true;
    }

    return false;
}

auto json_reader::ReadObject(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line[0] == '{') {
        if (line[line.size() - 1] != '}') {
            return false;
        }

        object obj {};

        auto splitLine {helper::trim(line.substr(1, line.size() - 2))};
        if (splitLine.empty()) {
            currentEntry.set_value(obj);
            return true;
        }

        if (helper::split_preserve_brackets_for_each(
                splitLine, ',',
                [&obj](utf8_string_view token) {
                    entry objValue;
                    return ReadKeyValuePair(objValue, obj, helper::trim(token));
                })) {
            currentEntry.set_value(obj);
            return true;
        }
    }

    return false;
}

auto json_reader::ReadNumber(entry& currentEntry, utf8_string_view line) -> bool
{
    auto const* valueStrData {line.data()};
    auto const  valueStrSize {line.size()};

    {
        i64 valueInt {0};
        auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, valueInt)};
        if (ec == std::errc {} && p == valueStrData + valueStrSize) {
            currentEntry.set_value(valueInt);
            return true;
        }
    }

    {
        f64 valueFloat {0};
        auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, valueFloat)};
        if (ec == std::errc {} && p == valueStrData + valueStrSize) {
            currentEntry.set_value(valueFloat);
            return true;
        }
    }

    return false;
}

auto json_reader::ReadBool(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line == "true" || line == "false") {
        currentEntry.set_value(line == "true");
        return true;
    }

    return false;
}

auto json_reader::ReadString(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line.size() > 1 && line[0] == '"' && line[line.size() - 1] == '"') {
        currentEntry.set_value(utf8_string {line.substr(1, line.size() - 2)});
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

auto json_writer::write(ostream& stream, object const& obj) -> bool
{
    write_object(stream, 0, obj);
    return true;
}

auto json_writer::write(ostream& stream, array const& arr) -> bool
{
    write_array(stream, 0, arr);
    return true;
}

void json_writer::write_object(ostream& stream, i32 indent, object const& obj) const
{
    utf8_string indentEntry(indent + INDENT_SPACES, ' ');
    utf8_string indentClose(indent, ' ');

    stream << "{\n";
    bool first {true};
    for (auto const& [k, v] : obj) {
        if (!first) { stream << ", \n"; }
        stream << indentEntry << "\"" << k << "\": ";
        write_entry(stream, indent + INDENT_SPACES, v);

        first = false;
    }

    stream << "\n"
           << indentClose << "}";
}

void json_writer::write_array(ostream& stream, i32 indent, array const& arr) const
{
    utf8_string indentItem(indent + INDENT_SPACES, ' ');
    utf8_string indentClose(indent, ' ');

    stream << "[\n";
    bool first {true};
    for (auto const& v : arr) {
        if (!first) { stream << ", \n"; }
        stream << indentItem;
        write_entry(stream, indent + INDENT_SPACES, v);

        first = false;
    }

    stream << "\n"
           << indentClose << "]";
}

void json_writer::write_entry(ostream& stream, i32 indent, entry const& ent) const
{
    if (ent.is<bool>()) {
        stream << (ent.as<bool>() ? "true" : "false");
    } else if (ent.is<i64>()) {
        stream << std::to_string(ent.as<i64>());
    } else if (ent.is<f64>()) {
        stream << std::to_string(ent.as<f64>());
    } else if (ent.is<utf8_string>()) {
        stream << "\"" << ent.as<utf8_string>() << "\"";
    } else if (ent.is<array>()) {
        write_array(stream, indent, ent.as<array>());
    } else if (ent.is<object>()) {
        write_object(stream, indent, ent.as<object>());
    }
}

}
