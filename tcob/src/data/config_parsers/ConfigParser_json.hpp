// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/core/io/Stream.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {
//////////////////////////////////////////////////////////////////////

class json_reader : public text_reader {
public:
    auto read_as_object(utf8_string_view txt) -> std::optional<object> override;
    auto read_as_array(utf8_string_view txt) -> std::optional<array> override;

    auto static ReadArray(entry& currentEntry, utf8_string_view line) -> bool;
    auto static ReadObject(entry& currentEntry, utf8_string_view line) -> bool;

private:
    auto static ReadKeyValuePair(entry& currentEntry, object& obj, utf8_string_view line) -> bool;

    auto static ReadValue(entry& currentEntry, utf8_string_view line) -> bool;
    auto static ReadNumber(entry& currentEntry, utf8_string_view line) -> bool;
    auto static ReadBool(entry& currentEntry, utf8_string_view line) -> bool;
    auto static ReadString(entry& currentEntry, utf8_string_view line) -> bool;
};

//////////////////////////////////////////////////////////////////////

class json_writer : public text_writer {
public:
    auto write(ostream& stream, object const& obj) -> bool override;
    auto write(ostream& stream, array const& arr) -> bool override;

private:
    void write_object(ostream& stream, i32 indent, object const& obj) const;
    void write_array(ostream& stream, i32 indent, array const& arr) const;
    void write_entry(ostream& stream, i32 indent, entry const& ent) const;
};

}
