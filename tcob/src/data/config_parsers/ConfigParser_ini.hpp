// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_set>
#include <utility>

#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {
//////////////////////////////////////////////////////////////////////

struct ini_settings {
    char                     KeyValueDelim {'='};
    char                     Path {'.'};
    char                     Reference {'@'};
    std::unordered_set<char> Comment {';', '#'};
    std::pair<char, char>    Section {'[', ']'};
    std::pair<char, char>    Object {'{', '}'};
    std::pair<char, char>    Array {'[', ']'};
    char                     Settings {'!'};
};

//////////////////////////////////////////////////////////////////////

class ini_reader : public text_reader {
public:
    auto read_as_object(utf8_string_view txt) -> std::optional<object> override;
    auto read_as_array(utf8_string_view txt) -> std::optional<array> override;

private:
    auto read_lines(object& targetObject) -> bool;
    auto read_line(object& targetObject, utf8_string_view line) -> bool;

    auto read_comment(utf8_string_view line) -> bool;
    auto read_section_header(object& targetObject, utf8_string_view line) -> bool;
    auto read_key_value_pair(object& targetObject, entry& currentEntry, utf8_string_view line) -> bool;

    auto read_ref(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_value(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_inline_array(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_inline_section(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_number(entry& currentEntry, utf8_string_view line) const -> bool;
    auto read_bool(entry& currentEntry, utf8_string_view line) const -> bool;
    auto read_string(entry& currentEntry, utf8_string_view line) -> bool;

    auto read_settings() -> bool;

    auto get_next_line() -> utf8_string_view;
    auto get_trimmed_next_line() -> utf8_string_view;
    auto is_eof() const -> bool;

    usize            _iniBegin;
    usize            _iniEnd;
    utf8_string_view _ini;

    object       _mainSection;
    comment      _currentComment {};
    ini_settings _settings;
};

//////////////////////////////////////////////////////////////////////

class ini_writer : public text_writer {
public:
    auto write(io::ostream& stream, object const& obj) -> bool override;
    auto write(io::ostream& stream, array const& arr) -> bool override;

    void write_inline_section(io::ostream& stream, object const& obj) const;

private:
    void write_array(io::ostream& stream, array const& arr) const;
    void write_section(io::ostream& stream, object const& obj, utf8_string const& prefix) const;
    void write_entry(io::ostream& stream, entry const& ent) const;
};

}
