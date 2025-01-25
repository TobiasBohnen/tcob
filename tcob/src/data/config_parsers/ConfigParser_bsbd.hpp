// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {
//////////////////////////////////////////////////////////////////////

namespace bsbd {
    enum class marker_type : u8 {
        SectionStart = 0x01,
        SectionEnd   = 0x02,
        ArrayStart   = 0x03,
        ArrayEnd     = 0x04,
        Int8         = 0x05,
        Int16        = 0x06,
        Int32        = 0x07,
        Int64        = 0x08,
        UInt8        = 0x09,
        UInt16       = 0x0A,
        UInt32       = 0x0B,
        Float32      = 0x0C,
        Float64      = 0x0D,
        BoolTrue     = 0x0E,
        BoolFalse    = 0x0F,
        LongString   = 0x10,
        ShortString  = 0x11,
        LitInt       = 0x14
    };

}

//////////////////////////////////////////////////////////////////////

class bsbd_reader : public binary_reader {
public:
    auto read_as_object(io::istream& stream) -> std::optional<object> override;
    auto read_as_array(io::istream& stream) -> std::optional<array> override;

private:
    auto read_section(io::istream& stream) const -> std::optional<object>;
    auto read_section_entry(io::istream& stream, bsbd::marker_type type, object& obj) const -> bool;

    auto read_array(io::istream& stream) const -> std::optional<array>;
    auto read_array_entry(io::istream& stream, bsbd::marker_type type, array& arr) const -> bool;
};

//////////////////////////////////////////////////////////////////////

class bsbd_writer : public binary_writer {
public:
    auto write(io::ostream& stream, object const& obj) -> bool override;
    auto write(io::ostream& stream, array const& arr) -> bool override;

private:
    auto write_section(io::ostream& stream, object const& obj, utf8_string const& name) const -> bool;
    auto write_array(io::ostream& stream, array const& arr, utf8_string const& name) const -> bool;
    auto write_entry(io::ostream& stream, entry const& ent, utf8_string const& name) const -> bool;

    auto write_entry_header(io::ostream& stream, bsbd::marker_type type, utf8_string const& name) const -> bool;
};

}
