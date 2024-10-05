// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_ini.hpp"

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
    if (openBr == closeBr) { return b % 2 == 0; }
    return b == 0;
}

auto ini_reader::read_as_object(utf8_string_view txt) -> std::optional<object>
{
    _ini         = txt;
    _iniBegin    = 0;
    _iniEnd      = 0;
    _mainSection = {};

    object kvp {_mainSection};
    if (txt[0] == _settings.Settings) {
        if (!read_settings()) { return std::nullopt; }
    }
    return read_lines(kvp) ? std::optional {_mainSection} : std::nullopt;
}

auto ini_reader::read_as_array(utf8_string_view txt) -> std::optional<array>
{
    _ini         = txt;
    _iniBegin    = 0;
    _iniEnd      = 0;
    _mainSection = {};

    entry currentEntry {};
    read_inline_array(currentEntry, get_trimmed_next_line());

    return currentEntry.is<array>() ? std::optional {currentEntry.as<array>()} : std::nullopt;
}

auto ini_reader::read_lines(object& targetObject) -> bool
{
    auto line {get_trimmed_next_line()};
    for (;;) {
        if (!read_line(targetObject, line)) { return false; }
        if (is_eof()) { break; }
        line = get_trimmed_next_line();
    }

    return true;
}

auto ini_reader::read_line(object& targetObject, utf8_string_view line) -> bool
{
    entry currentEntry;
    return line.empty()
        || read_comment(line)
        || read_section_header(targetObject, line)
        || read_key_value_pair(targetObject, currentEntry, line);
}

auto ini_reader::read_comment(utf8_string_view line) -> bool
{
    if (_settings.Comment.contains(line[0])) {
        if (line.size() > 1) {
            _currentComment.Text += line.substr(1);
            _currentComment.Text += "\n";
        }
        return true;
    }

    return false;
}

auto ini_reader::read_section_header(object& targetObject, utf8_string_view line) -> bool
{
    // read object header
    if (line[0] == _settings.Section.first) {
        auto const endPos {line.find(_settings.Section.second)};
        if (endPos == utf8_string::npos || endPos == 1) { return false; } // ERROR: invalid object header

        auto const lineSize {line.size()};
        // quoted key
        if ((line[1] == '\'' || line[1] == '"') && line[1] == line[lineSize - 2]) {
            line = line.substr(2, lineSize - 4);
            auto secRes {_mainSection[utf8_string {line}]};
            if (!secRes.is<object>()) { secRes = object {}; }
            targetObject = secRes.as<object>();
        } else {
            // read sub-sections
            bool first {true};
            helper::split_for_each(
                line.substr(1, endPos - 1), _settings.Path,
                [&first, &targetObject, this](utf8_string_view token) {
                    if (token.empty()) { return false; }
                    auto secRes {(first ? _mainSection : targetObject)[utf8_string {token}]};
                    if (!secRes.is<object>()) { secRes = object {}; }
                    targetObject = secRes.as<object>();
                    first        = false;
                    return true;
                });
        }

        // inheritance
        if (line.find(_settings.Reference, endPos) != utf8_string::npos) {
            auto const inh {helper::split_preserve_brackets(line, _settings.Reference)};
            if (inh.size() != 2) { return false; }

            object     obj {_mainSection};
            auto const keys {helper::split(inh[1], _settings.Path)};
            if (keys.size() > 1) {
                for (usize i {0}; i < keys.size() - 1; ++i) {
                    if (!obj.try_get(obj, keys[i])) { return false; }
                }
            }
            if (auto* entry {obj.get_entry(keys[keys.size() - 1])}) {
                if (entry->is<object>()) {
                    targetObject.merge(entry->as<object>());
                } else {
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}

auto ini_reader::read_key_value_pair(object& targetObject, entry& currentEntry, utf8_string_view line) -> bool
{
    auto const separatorPos {helper::find_unquoted(line, _settings.KeyValueDelim)};
    if (separatorPos == utf8_string::npos) { return false; } // ERROR:  invalid pair

    auto const keyStr {helper::trim(line.substr(0, separatorPos))};
    auto const valueStr {helper::trim(line.substr(separatorPos + 1))};

    if (keyStr.empty() || valueStr.empty()) { return false; }                                      //  ERROR: empty key or value
    auto const keyStrSize {keyStr.size()};
    if (keyStr[0] == _settings.Path || keyStr[keyStrSize - 1] == _settings.Path) { return false; } //  ERROR: dot at start or end of key

    object      sec {targetObject};
    utf8_string entryKey {};
    bool        first {true};

    // unescape key
    if ((keyStr[0] == '\'' || keyStr[0] == '"') && keyStr[0] == keyStr[keyStrSize - 1]) {
        entryKey = keyStr.substr(1, keyStrSize - 2);
    } else { // read sub-keys
        helper::split_for_each(
            keyStr, _settings.Path,
            [&first, &entryKey, &sec](utf8_string_view token) {
                if (first) {
                    entryKey = utf8_string {token};
                    first    = false;
                } else {
                    auto secRes {sec[entryKey]};
                    if (!secRes.is<object>()) { secRes = object {}; }
                    sec      = secRes.as<object>();
                    entryKey = utf8_string {token};
                }
                return true;
            });
    }

    // read value string
    if (valueStr[0] == _settings.Reference && valueStr.size() > 1) {
        if (!read_ref(currentEntry, valueStr.substr(1))) {
            return false; // invalid ref
        }
    } else if (!read_value(currentEntry, valueStr)) {
        return false;     // invalid value
    }

    currentEntry.set_comment(_currentComment);
    _currentComment = {};
    sec.set_entry(entryKey, currentEntry);
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

auto ini_reader::read_ref(entry& currentEntry, utf8_string_view line) -> bool
{
    object obj {_mainSection};
    auto   keys {helper::split(line, _settings.Path)};
    if (keys.size() > 1) {
        for (usize i {0}; i < keys.size() - 1; ++i) {
            if (!obj.try_get(obj, keys[i])) { return false; }
        }
    }

    if (auto* entry {obj.get_entry(keys[keys.size() - 1])}) {
        if (entry->is<object>()) {
            currentEntry = entry->as<object>().clone(true);
        } else if (entry->is<array>()) {
            currentEntry = entry->as<array>().clone(true);
        } else {
            currentEntry = *entry;
        }

        return true;
    }

    return false;
}

auto ini_reader::read_inline_array(entry& currentEntry, utf8_string_view line) -> bool
{
    if (line[0] == _settings.Array.first) {
        utf8_string arrayLine {line};
        while (!is_eof()
               && (arrayLine.size() <= 1
                   || arrayLine[arrayLine.size() - 1] != _settings.Array.second
                   || !check_brackets(arrayLine, _settings.Array.first, _settings.Array.second))) {
            arrayLine += get_trimmed_next_line();
        }

        if (arrayLine[arrayLine.size() - 1] != _settings.Array.second) { return false; }

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
    if (line[0] == _settings.Object.first) {
        utf8_string sectionLine {line};
        while (!is_eof()
               && (sectionLine.size() <= 1
                   || sectionLine[sectionLine.size() - 1] != _settings.Object.second
                   || !check_brackets(sectionLine, _settings.Object.first, _settings.Object.second))) {
            sectionLine += get_trimmed_next_line();
        }

        if (sectionLine[sectionLine.size() - 1] != _settings.Object.second) { return false; }

        object obj {};
        if (helper::split_preserve_brackets_for_each(
                sectionLine.substr(1, sectionLine.size() - 2), ',',
                [&obj, this](utf8_string_view token) {
                    auto const tokenString {helper::trim(token)};
                    if (tokenString.empty()) { return true; } // allow empty entries
                    entry secEntry;
                    return read_key_value_pair(obj, secEntry, tokenString);
                })) {
            currentEntry.set_value(obj);
            return true;
        }
    }

    return false;
}

auto ini_reader::read_number(entry& currentEntry, utf8_string_view line) const -> bool
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
    if (!line.empty()) {
        char const first {line[0]};
        if (first == '\'') { // single-quoted (literal multi-line)
            utf8_string stringLine {line};
            bool        firstLine {true};

            while (!is_eof() && ((firstLine && line.size() == 1) || helper::trim(stringLine).back() != first)) {
                if (firstLine) {
                    firstLine = false;
                } else {
                    stringLine += "\n";
                }
                stringLine += get_next_line();
            }

            auto const endPos {stringLine.find(first, 1)};
            auto       val {stringLine.substr(1, endPos - 1)};

            if (!val.empty() && val.back() == '\n') {
                val.pop_back(); // Remove the last '\n'
            }

            currentEntry.set_value(val);
            return true;
        }

        if (first == '"') { // double-quoted (trimmed multi-line)
            utf8_string stringLine {line};
            while (!is_eof() && stringLine.size() > 1 && stringLine.back() != first) {
                stringLine += "\n" + utf8_string {get_trimmed_next_line()};
            }

            currentEntry.set_value(stringLine.substr(1, stringLine.size() - 2));
            return true;
        }
    }

    currentEntry.set_value(utf8_string {line});
    return true;
}

auto ini_reader::read_settings() -> bool
{
    //! kvp=: path=| ref=@ comment=;# section=<> object=-- array=++
    _iniEnd = 1;
    auto line {get_trimmed_next_line()};

    return helper::split_for_each(line, ' ', [&](auto kvp) {
        auto settings {helper::split_once(helper::trim(kvp), '=')};
        if (settings.second.empty()) { return false; }

        auto key {helper::trim(settings.first)};
        auto value {helper::trim(settings.second)};

        if (key == "kvp") {
            _settings.KeyValueDelim = value[0];
        } else if (key == "path") {
            _settings.Path = value[0];
        } else if (key == "ref") {
            _settings.Reference = value[0];
        } else if (key == "comment") {
            _settings.Comment.clear();
            _settings.Comment.insert(value.begin(), value.end());
        } else if (key == "section") {
            if (value.size() < 2) { return false; }
            _settings.Section = {value[0], value[1]};
        } else if (key == "object") {
            if (value.size() < 2) { return false; }
            _settings.Object = {value[0], value[1]};
        } else if (key == "array") {
            if (value.size() < 2) { return false; }
            _settings.Array = {value[0], value[1]};
        }

        return true;
    });
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

    return _ini.substr(_iniBegin, _iniEnd - _iniBegin - 1);
}

auto ini_reader::get_trimmed_next_line() -> utf8_string_view
{
    return helper::trim(get_next_line());
}

auto ini_reader::is_eof() const -> bool
{
    return _iniEnd >= _ini.size();
}

//////////////////////////////////////////////////////////////////////

auto ini_writer::write(io::ostream& stream, object const& obj) -> bool
{
    write_section(stream, obj, "");
    return true;
}

auto ini_writer::write(io::ostream& stream, array const& arr) -> bool
{
    write_array(stream, arr);
    return true;
}

void ini_writer::write_section(io::ostream& stream, object const& obj, utf8_string const& prefix) const
{
    std::unordered_map<utf8_string, object> objects;
    for (auto const& [k, v] : obj) {
        auto const& comment {v.get_comment()};
        for (auto const& c : helper::split(comment.Text, '\n')) {
            stream << ";" << c << "\n";
        }

        bool const needsEscape {k.find('.') != utf8_string::npos};
        if (v.is<object>()) {
            if (needsEscape) {
                if (prefix.empty()) {
                    objects.emplace("'" + k + "'", v.as<object>());
                } else {
                    stream << "'" << k << "' = ";
                    write_entry(stream, v);
                    stream << "\n";
                }
            } else {
                objects.emplace(k, v.as<object>());
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

void ini_writer::write_inline_section(io::ostream& stream, object const& obj) const
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

void ini_writer::write_array(io::ostream& stream, array const& arr) const
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

void ini_writer::write_entry(io::ostream& stream, entry const& ent) const
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
