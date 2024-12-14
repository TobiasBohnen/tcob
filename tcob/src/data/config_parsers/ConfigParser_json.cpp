// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_json.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/data/ConfigConversions.hpp"

namespace tcob::data::config::detail {

auto static find_unquoted(string_view source, char needle) -> string_view::size_type
{
    char const quote {source[0]};
    if (quote != '"' && quote != '\'') {
        return source.find(needle);
    }

    bool  inQuotes {false};
    usize pos {0};
    for (auto const c : source) {
        if (c == quote) {
            inQuotes = !inQuotes;
        } else if (!inQuotes && c == needle) {
            return pos;
        }
        ++pos;
    }

    return string_view::npos;
}

constexpr usize INDENT_SPACES {2};

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

auto json_reader::ReadKeyValuePair(object& obj, entry& currentEntry, utf8_string_view line) -> bool
{
    if (line.empty()) { return false; }

    auto const separatorPos {find_unquoted(line, ':')};
    if (separatorPos == utf8_string::npos) { return false; } // ERROR: invalid pair

    auto const keyStr {helper::trim(line.substr(0, separatorPos))};
    auto const valueStr {helper::trim(line.substr(separatorPos + 1))};

    if (keyStr.size() <= 1 || valueStr.empty()) { return false; }                 //  ERROR: empty key or value
    if (keyStr[0] != '\"' || keyStr[keyStr.size() - 1] != '\"') { return false; } //  ERROR: invalid key
    if (valueStr == "null") { return true; }                                      // ignore nulled keys

    // read value string
    if (!ReadValue(currentEntry, valueStr)) { return false; } // ERROR: invalid value

    // unescape key
    utf8_string const key {keyStr.substr(1, keyStr.size() - 2)};
    obj.set_entry(key, currentEntry);
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
    auto const lineSize {line.size()};
    if (lineSize <= 1) { return false; }

    if (line[0] == '[') {
        if (line[lineSize - 1] != ']') { return false; }

        array arr {};

        auto const splitLine {helper::trim(line.substr(1, lineSize - 2))};
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
    auto const lineSize {line.size()};
    if (lineSize <= 1) { return false; }

    if (line[0] == '{') {
        if (line[lineSize - 1] != '}') { return false; }

        object obj {};

        auto const splitLine {helper::trim(line.substr(1, lineSize - 2))};
        if (splitLine.empty()) {
            currentEntry.set_value(obj);
            return true;
        }

        if (helper::split_preserve_brackets_for_each(
                splitLine, ',',
                [&obj](utf8_string_view token) {
                    entry objValue;
                    return ReadKeyValuePair(obj, objValue, helper::trim(token));
                })) {
            currentEntry.set_value(obj);
            return true;
        }
    }

    return false;
}

auto json_reader::ReadNumber(entry& currentEntry, utf8_string_view line) -> bool
{
    if (auto const intVal {helper::to_number<i64>(line)}) {
        currentEntry.set_value(*intVal);
        return true;
    }

    if (auto const floatVal {helper::to_number<f64>(line)}) {
        currentEntry.set_value(*floatVal);
        return true;
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
    auto const lineSize {line.size()};
    if (lineSize > 1 && line[0] == '"' && line[lineSize - 1] == '"') {
        currentEntry.set_value(utf8_string {line.substr(1, lineSize - 2)});
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

auto json_writer::write(io::ostream& stream, object const& obj) -> bool
{
    write_object(stream, 0, obj);
    return true;
}

auto json_writer::write(io::ostream& stream, array const& arr) -> bool
{
    write_array(stream, 0, arr);
    return true;
}

void json_writer::write_object(io::ostream& stream, usize indent, object const& obj) const
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

void json_writer::write_array(io::ostream& stream, usize indent, array const& arr) const
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

void json_writer::write_entry(io::ostream& stream, usize indent, entry const& ent) const
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
