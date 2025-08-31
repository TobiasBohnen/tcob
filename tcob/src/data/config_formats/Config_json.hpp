// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::detail {
//////////////////////////////////////////////////////////////////////

class json_reader : public text_reader {
public:
    auto read_as_object(utf8_string_view txt) -> std::optional<object> override;
    auto read_as_array(utf8_string_view txt) -> std::optional<array> override;

    static auto ReadArray(entry& currentEntry, utf8_string_view line) -> bool;
    static auto ReadObject(entry& currentEntry, utf8_string_view line) -> bool;

private:
    static auto ReadKeyValuePair(object& obj, entry& currentEntry, utf8_string_view line) -> bool;

    static auto ReadEntry(entry& currentEntry, utf8_string_view line) -> bool;
    static auto ReadScalar(entry& currentEntry, utf8_string_view line) -> bool;
};

//////////////////////////////////////////////////////////////////////

class json_writer : public text_writer {
public:
    auto write(io::ostream& stream, object const& obj) -> bool override;
    auto write(io::ostream& stream, array const& arr) -> bool override;

private:
    auto write_object(io::ostream& stream, usize indent, object const& obj, usize maxDepth) const -> bool;
    auto write_array(io::ostream& stream, usize indent, array const& arr, usize maxDepth) const -> bool;
    auto write_entry(io::ostream& stream, usize indent, entry const& ent, usize maxDepth) const -> bool;
};

}
