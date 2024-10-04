// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {
//////////////////////////////////////////////////////////////////////

class xml_reader : public text_reader {
    enum class tag_type : u8 {
        Opening,
        Closing,
        SelfClosing
    };

    struct tag {
        utf8_string                                  Name;
        std::unordered_map<utf8_string, utf8_string> Attributes;
        tag_type                                     Type {};
    };

    struct element {
        tag                                   Tag;
        utf8_string                           Value;
        std::vector<std::unique_ptr<element>> Children;
    };

public:
    auto read_as_object(utf8_string_view txt) -> std::optional<object> override;
    auto read_as_array(utf8_string_view txt) -> std::optional<array> override;

private:
    auto read_element() -> std::unique_ptr<element>;
    auto read_tag() -> std::optional<tag>;
    auto read_tag_name(tag& t) -> bool;
    auto read_tag_attributes(tag& t) -> bool;
    auto read_tag_attribute_key() -> utf8_string;
    auto read_tag_attribute_value() -> utf8_string;

    auto read_content(element& n) -> bool;

    void skip_whitespace();
    auto peek() -> char;
    auto read() -> char;
    void advance();
    void step_back();
    auto is_eof() const -> bool;

    auto convert_to_object(element const& n) -> object;
    auto convert_to_array(element const& n) -> array;
    void convert_value(entry& currentEntry, utf8_string const& str);

    usize            _xmlIndex {0};
    utf8_string_view _xml;
};

//////////////////////////////////////////////////////////////////////

class xml_writer : public text_writer {
public:
    auto write(io::ostream& stream, object const& obj) -> bool override;
    auto write(io::ostream& stream, array const& arr) -> bool override;

private:
    auto write_element(io::ostream& stream, u32 indent, utf8_string const& name, object const& obj) const -> bool;
    auto write_array(io::ostream& stream, u32 indent, utf8_string const& name, array const& arr) const -> bool;
    void write_attribute(io::ostream& stream, utf8_string const& name, entry const& ent) const;
};

}
