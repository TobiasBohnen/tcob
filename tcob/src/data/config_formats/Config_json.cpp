// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Config_json.hpp"

#include <optional>
#include <string>
#include <variant>

#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::detail {

static auto FindUnquoted(string_view source, char needle) -> string_view::size_type
{
    bool inQuotes {false};
    bool escaped {false};

    for (usize i {0}; i < source.size(); ++i) {
        char const c {source[i]};

        if (escaped) {
            escaped = false;
            continue;
        }

        if (c == '\\') {
            escaped = true;
            continue;
        }

        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes && c == needle) {
            return i;
        }
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

    auto const separatorPos {FindUnquoted(line, ':')};
    if (separatorPos == utf8_string::npos) { return false; } // ERROR: invalid pair

    auto const keyStr {helper::trim(line.substr(0, separatorPos))};
    auto const valueStr {helper::trim(line.substr(separatorPos + 1))};

    if (keyStr.size() <= 1 || valueStr.empty()) { return false; }                 //  ERROR: empty key or value
    if (keyStr[0] != '\"' || keyStr[keyStr.size() - 1] != '\"') { return false; } //  ERROR: invalid key

    // read value string
    if (!ReadEntry(currentEntry, valueStr)) { return false; } // ERROR: invalid value

    // unescape key
    utf8_string const key {keyStr.substr(1, keyStr.size() - 2)};
    obj.set_entry(key, currentEntry);
    return true;
}

auto json_reader::ReadEntry(entry& currentEntry, utf8_string_view line) -> bool
{
    return !line.empty()
        && (ReadArray(currentEntry, line)
            || ReadObject(currentEntry, line)
            || ReadScalar(currentEntry, line));
}

auto json_reader::ReadArray(entry& currentEntry, utf8_string_view line) -> bool
{
    auto const lineSize {line.size()};
    if (lineSize < 2) { return false; }
    if (line[0] != '[') { return false; }
    if (line[lineSize - 1] != ']') { return false; }

    array arr {};

    auto const splitLine {helper::trim(line.substr(1, lineSize - 2))};
    if (splitLine.empty()) {
        currentEntry.set_value(arr);
        return true;
    }

    if (!helper::split_preserve_brackets(
            splitLine, ',',
            [&arr](utf8_string_view token) {
                auto const tk {helper::trim(token)};
                entry      arrvalue;
                if (ReadEntry(arrvalue, tk)) {
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

auto json_reader::ReadObject(entry& currentEntry, utf8_string_view line) -> bool
{
    auto const lineSize {line.size()};
    if (lineSize <= 1) { return false; }
    if (line[0] != '{') { return false; }
    if (line[lineSize - 1] != '}') { return false; }

    object obj {};

    auto const splitLine {helper::trim(line.substr(1, lineSize - 2))};
    if (splitLine.empty()) {
        currentEntry.set_value(obj);
        return true;
    }

    if (helper::split_preserve_brackets(
            splitLine, ',',
            [&obj](utf8_string_view token) {
                entry objValue;
                return ReadKeyValuePair(obj, objValue, helper::trim(token));
            })) {
        currentEntry.set_value(obj);
        return true;
    }

    return false;
}

auto json_reader::ReadScalar(entry& currentEntry, utf8_string_view line) -> bool
{
    // null
    if (line == "null") {
        currentEntry.set_value(std::monostate {});
        return true;
    }

    // bool
    if (line == "true" || line == "false") {
        currentEntry.set_value(line == "true");
        return true;
    }

    // int
    if (auto const intVal {helper::to_number<i64>(line)}) {
        currentEntry.set_value(*intVal);
        return true;
    }

    // float
    if (auto const floatVal {helper::to_number<f64>(line)}) {
        currentEntry.set_value(*floatVal);
        return true;
    }

    // string
    auto const lineSize {line.size()};
    if (lineSize > 1 && line[0] == '"' && line[lineSize - 1] == '"') {
        auto const  raw {line.substr(1, lineSize - 2)};
        utf8_string unescaped;
        unescaped.reserve(raw.size());

        for (usize i {0}; i < raw.size(); ++i) {
            char const c {raw[i]};
            if (c == '\\' && i + 1 < raw.size()) {
                char const next {raw[++i]};
                switch (next) {
                case '"':  unescaped += '"'; break;
                case '\\': unescaped += '\\'; break;
                case '/':  unescaped += '/'; break;
                case 'b':  unescaped += '\b'; break;
                case 'f':  unescaped += '\f'; break;
                case 'n':  unescaped += '\n'; break;
                case 'r':  unescaped += '\r'; break;
                case 't':  unescaped += '\t'; break;
                case 'u':  {
                    if (i + 4 >= raw.size()) { return false; }

                    u32 cp {0};
                    for (i32 j {0}; j < 4; ++j) {
                        char const h {raw[++i]};

                        cp <<= 4;
                        if (h >= '0' && h <= '9') {
                            cp |= h - '0';
                        } else if (h >= 'A' && h <= 'F') {
                            cp |= h - 'A' + 10;
                        } else if (h >= 'a' && h <= 'f') {
                            cp |= h - 'a' + 10;
                        } else {
                            return false;
                        }
                    }

                    if (cp <= 0x7F) {
                        unescaped += static_cast<char>(cp);
                    } else if (cp <= 0x7FF) {
                        unescaped += static_cast<char>(0xC0 | (cp >> 6));
                        unescaped += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        unescaped += static_cast<char>(0xE0 | (cp >> 12));
                        unescaped += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        unescaped += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default: unescaped += next; break;
                }
            } else {
                unescaped += c;
            }
        }

        currentEntry.set_value(unescaped);
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

constexpr usize MAX_DEPTH {1000};

static auto EscapeJsonString(utf8_string_view str) -> utf8_string
{
    utf8_string result;
    result.reserve(str.size());

    for (auto const c : str) {
        switch (c) {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:   result += c; break;
        }
    }

    return result;
}

auto json_writer::write(io::ostream& stream, object const& obj) -> bool
{
    return write_object(stream, 0, obj, MAX_DEPTH);
}

auto json_writer::write(io::ostream& stream, array const& arr) -> bool
{
    return write_array(stream, 0, arr, MAX_DEPTH);
}

auto json_writer::write_object(io::ostream& stream, usize indent, object const& obj, usize maxDepth) const -> bool
{
    if (maxDepth == 0) { return false; }

    utf8_string indentEntry(indent + INDENT_SPACES, ' ');
    utf8_string indentClose(indent, ' ');

    stream << "{\n";
    bool first {true};
    for (auto const& [k, v] : obj) {
        if (!first) { stream << ", \n"; }
        stream << indentEntry << "\"" << EscapeJsonString(k) << "\": ";

        if (!write_entry(stream, indent + INDENT_SPACES, v, maxDepth - 1)) { return false; }

        first = false;
    }

    stream << "\n"
           << indentClose << "}";

    return true;
}

auto json_writer::write_array(io::ostream& stream, usize indent, array const& arr, usize maxDepth) const -> bool
{
    if (maxDepth == 0) { return false; }

    utf8_string indentItem(indent + INDENT_SPACES, ' ');
    utf8_string indentClose(indent, ' ');

    stream << "[\n";
    bool first {true};
    for (auto const& v : arr) {
        if (!first) { stream << ", \n"; }
        stream << indentItem;

        if (!write_entry(stream, indent + INDENT_SPACES, v, maxDepth - 1)) { return false; }

        first = false;
    }

    stream << "\n"
           << indentClose << "]";

    return true;
}

auto json_writer::write_entry(io::ostream& stream, usize indent, entry const& ent, usize maxDepth) const -> bool
{
    if (ent.is<bool>()) {
        stream << (ent.as<bool>() ? "true" : "false");
    } else if (ent.is<i64>()) {
        stream << std::to_string(ent.as<i64>());
    } else if (ent.is<f64>()) {
        stream << std::to_string(ent.as<f64>());
    } else if (ent.is<utf8_string>()) {
        stream << "\"" << EscapeJsonString(ent.as<utf8_string>()) << "\"";
    } else if (ent.is<array>()) {
        return write_array(stream, indent, ent.as<array>(), maxDepth);
    } else if (ent.is<object>()) {
        return write_object(stream, indent, ent.as<object>(), maxDepth);
    } else if (ent.is<std::monostate>()) {
        stream << "null";
    }

    return true;
}

}
