// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_set>
#include <utility>

#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::detail {
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

    auto read_comment(object& targetObject, utf8_string_view line) -> bool;
    auto read_section_header(object& targetObject, utf8_string_view line) -> bool;
    auto read_key_value_pair(object& targetObject, utf8_string_view line) -> bool;
    auto read_key_value_pair(object& targetObject, entry& currentEntry, utf8_string_view line) -> bool;

    auto read_ref(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_entry(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_inline_array(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_inline_section(entry& currentEntry, utf8_string_view line) -> bool;
    auto read_scalar(entry& currentEntry, utf8_string_view line) -> bool;

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

    auto write_inline_section(io::ostream& stream, object const& obj, usize maxDepth) const -> bool;

private:
    auto write_section(io::ostream& stream, object const& obj, utf8_string const& prefix, usize maxDepth) const -> bool;
    auto write_array(io::ostream& stream, array const& arr, usize maxDepth) const -> bool;
    auto write_entry(io::ostream& stream, entry const& ent, usize maxDepth) const -> bool;
};

}
