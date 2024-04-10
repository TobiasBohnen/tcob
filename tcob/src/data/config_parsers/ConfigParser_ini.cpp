// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_ini.hpp"

#include <charconv>

#include "tcob/core/StringUtils.hpp"

namespace tcob::data::config::detail {

auto static check_brackets(utf8_string_view str, char openBr, char closeBr)
{
    assert(str[str.size() - 1] == closeBr);
    i32 b {0};
    for (auto const& c : str) {
        if (c == openBr) {
            b++;
        } else if (c == closeBr) {
            b--;
        }
    }
    return b == 0;
}

auto ini_reader::read_as_object(utf8_string_view txt) -> std::optional<object>
{
    _ini           = txt;
    _section       = {};
    _parentSection = _section;
    return read_lines() ? std::optional {_section} : std::nullopt;
}

auto ini_reader::read_as_array(utf8_string_view txt) -> std::optional<array>
{
    _ini           = txt;
    _section       = {};
    _parentSection = _section;
    entry currentEntry {};
    read_inline_array(currentEntry, get_next_line());

    return currentEntry.is<array>() ? std::optional {currentEntry.as<array>()} : std::nullopt;
}

auto ini_reader::read_lines() -> bool
{
    auto line {get_next_line()};
    for (;;) {
        if (!read_line(line)) { return false; }
        if (is_eof()) { break; }
        line = get_next_line();
    }

    return true;
}

auto ini_reader::read_line(utf8_string_view line) -> bool
{
    entry currentEntry;
    return line.empty()
        || read_comment(line)
        || read_section_header(line)
        || read_key_value_pair(currentEntry, _parentSection, line);
}

auto ini_reader::read_comment(utf8_string_view line) -> bool
{
    if (line[0] == ';' || line[0] == '#') {
        if (line.size() > 1) {
            _currentComment.Text += line.substr(1);
            _currentComment.Text += "\n";
        }
        return true;
    }

    return false;
}

auto ini_reader::read_section_header(utf8_string_view line) -> bool
{
    // read object header
    if (line[0] == '[') {
        auto const endPos {line.find(']')};
        if (endPos == utf8_string::npos || endPos == 1) { return false; } // ERROR: invalid object header

        auto const lineSize {line.size()};
        if ((line[1] == '\'' && line[lineSize - 2] == '\'') || (line[1] == '"' && line[lineSize - 2] == '"')) {
            line = line.substr(2, lineSize - 4);
            auto secRes {_section[utf8_string {line}]};
            if (!secRes.is<object>()) { secRes = object {}; }
            _parentSection = secRes.as<object>();
        } else {
            // read sub-sections
            bool first {true};
            helper::split_for_each(
                line.substr(1, endPos - 1), '.',
                [&first, this](utf8_string_view token) {
                    auto secRes {(first ? _section : _parentSection)[utf8_string {token}]};
                    if (!secRes.is<object>()) { secRes = object {}; }
                    _parentSection = secRes.as<object>();
                    first          = false;
                    return true;
                });
        }

        return true;
    }

    return false;
}

auto ini_reader::read_key_value_pair(entry& currentEntry, object const& obj, utf8_string_view line) -> bool
{
    auto const separatorPos {line.find('=')};
    if (separatorPos == utf8_string::npos) { return false; } // ERROR:  invalid pair

    auto const keyStr {helper::trim(line.substr(0, separatorPos))};
    auto const valueStr {helper::trim(line.substr(separatorPos + 1))};

    if (keyStr.empty() || valueStr.empty()) { return false; }                //  ERROR: empty key or value
    auto const keyStrSize {keyStr.size()};
    if (keyStr[0] == '.' || keyStr[keyStrSize - 1] == '.') { return false; } //  ERROR: dot at start or end of key

    object      currentSection {obj};
    utf8_string entryKey {};
    bool        first {true};

    // unescape key
    if ((keyStr[0] == '\'' && keyStr[keyStrSize - 1] == '\'') || (keyStr[0] == '"' && keyStr[keyStrSize - 1] == '"')) {
        entryKey = keyStr.substr(1, keyStrSize - 2);
    } else { // read sub-keys
        helper::split_for_each(
            keyStr, '.',
            [&first, &entryKey, &currentSection](utf8_string_view token) {
                if (first) {
                    entryKey = utf8_string {token};
                    first    = false;
                } else {
                    auto secRes {currentSection[entryKey]};
                    if (!secRes.is<object>()) { secRes = object {}; }
                    currentSection = secRes.as<object>();
                    entryKey       = utf8_string {token};
                }
                return true;
            });
    }

    // read value string
    if (!read_value(currentEntry, valueStr)) { return false; } // invalid value

    currentEntry.set_comment(_currentComment);
    _currentComment = {};
    currentSection.set_entry(entryKey, currentEntry);
    return true;
}

auto ini_reader::read_value(entry& currentEntry, utf8_string_view line) -> bool
{
    return !line.empty()
        && (read_inline_array(currentEntry, line)
            || read_inline_section(currentEntry, line)
            || read_number(currentEntry, line)
            || read_bool(currentEntry, line)
            || read_string(currentEntry, line));
}

auto ini_reader::read_inline_array(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line[0] == '[') {
        utf8_string arrayLine {line};
        while (!is_eof()
               && (arrayLine.size() <= 1 || arrayLine[arrayLine.size() - 1] != ']' || !check_brackets(arrayLine, '[', ']'))) {
            arrayLine += get_next_line();
        }

        if (arrayLine[arrayLine.size() - 1] != ']') { return false; }

        array arr {};
        if (!helper::split_preserve_brackets_for_each(
                arrayLine.substr(1, arrayLine.size() - 2), ',',
                [&arr, this](utf8_string_view token) {
                    auto const tokenString {helper::trim(token)};
                    if (tokenString.empty()) { return true; } // allow empty entries
                    entry arrEntry;
                    if (read_value(arrEntry, tokenString)) {
                        arr.add_entry(arrEntry);
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

auto ini_reader::read_inline_section(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line[0] == '{') {
        utf8_string sectionLine {line};
        while (!is_eof()
               && (sectionLine.size() <= 1 || sectionLine[sectionLine.size() - 1] != '}' || !check_brackets(sectionLine, '{', '}'))) {
            sectionLine += get_next_line();
        }

        if (sectionLine[sectionLine.size() - 1] != '}') { return false; }

        object obj {};
        if (helper::split_preserve_brackets_for_each(
                sectionLine.substr(1, sectionLine.size() - 2), ',',
                [&obj, this](utf8_string_view token) {
                    auto const tokenString {helper::trim(token)};
                    if (tokenString.empty()) { return true; } // allow empty entries
                    entry secEntry;
                    return read_key_value_pair(secEntry, obj, tokenString);
                })) {
            currentEntry.set_value(obj);
            return true;
        }
    }

    return false;
}

auto ini_reader::read_number(entry& currentEntry, utf8_string_view line) const -> bool
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

auto ini_reader::read_bool(entry& currentEntry, utf8_string_view line) const -> bool
{
    if (line == "true" || line == "false") {
        currentEntry.set_value(line == "true");
        return true;
    }

    return false;
}

auto ini_reader::read_string(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line.size() > 1) {
        char const first {line[0]};
        if (first == '"' || first == '\'') {
            utf8_string stringLine {line};
            while (!is_eof() && stringLine[stringLine.size() - 1] != first) {
                stringLine += "\n" + utf8_string {get_next_line()};
            }

            currentEntry.set_value(stringLine.substr(1, stringLine.size() - 2));
            return true;
        }
    }

    currentEntry.set_value(utf8_string {line});
    return true;
}

auto ini_reader::get_next_line() -> utf8_string_view
{
    if (is_eof()) { return ""; }

    _iniBegin = _iniEnd;
    _iniEnd   = _ini.find('\n', _iniEnd);
    if (_iniEnd == utf8_string::npos) {
        _iniEnd = _ini.size() + 1;
    } else {
        ++_iniEnd;
    }

    return helper::trim(_ini.substr(_iniBegin, _iniEnd - _iniBegin - 1));
}

auto ini_reader::is_eof() const -> bool
{
    return _iniEnd >= _ini.size();
}

//////////////////////////////////////////////////////////////////////

auto ini_writer::write(ostream& stream, object const& obj) -> bool
{
    write_section(stream, obj, "");
    return true;
}

auto ini_writer::write(ostream& stream, array const& arr) -> bool
{
    write_array(stream, arr);
    return true;
}

void ini_writer::write_section(ostream& stream, object const& obj, utf8_string const& prefix) const
{
    flat_map<utf8_string, object> objects;
    for (auto const& [k, v] : obj) {
        auto const& comment {v.get_comment()};
        for (auto const& c : helper::split(comment.Text, '\n')) {
            stream << ";" << c << "\n";
        }

        bool const needsEscape {k.find('.') != utf8_string::npos};
        if (v.is<object>()) {
            if (needsEscape) {
                if (prefix.empty()) {
                    objects.insert("'" + k + "'", v.as<object>());
                } else {
                    stream << "'" << k << "' = ";
                    write_entry(stream, v);
                    stream << "\n";
                }
            } else {
                objects.insert(k, v.as<object>());
            }
        } else {
            if (needsEscape) {
                stream << "'" << k << "' = ";
            } else {
                stream << k << " = ";
            }
            write_entry(stream, v);
            stream << "\n";
        }
    }

    utf8_string const prefixSec {prefix.empty() ? "" : prefix + "."};
    for (auto const& [k, v] : objects) {
        if (stream.tell() > 0) { stream << "\n"; }

        stream << "[" << prefixSec << k << "]\n";
        write_section(stream, v, prefixSec + k);
    }
}

void ini_writer::write_inline_section(ostream& stream, object const& obj) const
{
    stream << "{ ";
    bool first {true};
    for (auto const& [k, v] : obj) {
        if (!first) { stream << ", "; }
        first = false;

        bool const needsEscape {k.find('.') != utf8_string::npos};
        if (needsEscape) {
            stream << "'" << k << "' = ";
        } else {
            stream << k << " = ";
        }

        write_entry(stream, v);
    }
    stream << " }";
}

void ini_writer::write_array(ostream& stream, array const& arr) const
{
    stream << "[ ";
    bool first {true};
    for (auto const& v : arr) {
        if (!first) { stream << ", "; }
        first = false;

        write_entry(stream, v);
    }
    stream << " ]";
}

void ini_writer::write_entry(ostream& stream, entry const& ent) const
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
        write_array(stream, ent.as<array>());
    } else if (ent.is<object>()) {
        write_inline_section(stream, ent.as<object>());
    }
}
}
