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

namespace bsbd {
    enum class marker_type : u8 {
        SectionStart = 0x01,
        SectionEnd   = 0x11,
        ArrayStart   = 0x21,
        ArrayEnd     = 0x31,
        Int8         = 0x02,
        Int16        = 0x12,
        Int32        = 0x22,
        Int64        = 0x32,
        UInt8        = 0x03,
        UInt16       = 0x13,
        UInt32       = 0x23,
        Float32      = 0x04,
        Float64      = 0x14,
        BoolTrue     = 0x05,
        BoolFalse    = 0x15,
        String       = 0x06
    };
}

//////////////////////////////////////////////////////////////////////

class bsbd_reader : public binary_reader {
public:
    auto read_as_object(istream& stream) -> std::optional<object> override;
    auto read_as_array(istream& stream) -> std::optional<array> override;

private:
    auto read_section(istream& stream) -> std::optional<object>;
    auto read_section_entry(istream& stream, bsbd::marker_type type, object& obj) -> bool;

    auto read_array(istream& stream) -> std::optional<array>;
    auto read_array_entry(istream& stream, bsbd::marker_type type, array& arr) -> bool;
};

//////////////////////////////////////////////////////////////////////

class bsbd_writer : public binary_writer {
public:
    auto write(ostream& stream, object const& obj) -> bool override;
    auto write(ostream& stream, array const& arr) -> bool override;

private:
    void write_section(ostream& stream, object const& obj, utf8_string const& name) const;
    void write_array(ostream& stream, array const& arr, utf8_string const& name) const;
    void write_entry(ostream& stream, entry const& ent, utf8_string const& name) const;

    void write_entry_header(ostream& stream, bsbd::marker_type type, utf8_string const& name) const;
};

}
