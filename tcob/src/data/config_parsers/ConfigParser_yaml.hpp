// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/FlatMap.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {
//////////////////////////////////////////////////////////////////////

class yaml_tokenizer {
public:
    enum class token_type : u8 {
        None,
        KeyOrScalar,
        Newline,         // '\n'
        Whitespace,      // ' '
        Indent,
        MappingKey,      // '? '
        MappingValue,    // ': '
        Sequence,        // '- '
        Comment,         // '#'
        FlowSequence,    // '[]'
        FlowMapping,     // '{}'
        StartOfDocument, // '---'
        EndOfDocument,   // '+++'
        LiteralStyle,    // '|'
        FoldedStyle,     // '>'
        DoubleQuote,     // '"'
        SingleQuote,     // '''
        Tag,             // '!'
        Anchor,          // '&'
        Alias,           // '*'
        EoF,
    };

    struct token {
        token_type  Type {token_type::None};
        utf8_string Value;
    };

    auto tokenize(utf8_string_view yaml) -> bool;

    std::vector<yaml_tokenizer::token> Tokens;

private:
    auto tokenize_line(utf8_string_view line) -> bool;

    auto get_next_line() -> utf8_string_view;
    auto is_eof() const -> bool;

    void optimize();

    usize            _yamlBegin {0};
    usize            _yamlEnd {0};
    utf8_string_view _yaml;
};

//////////////////////////////////////////////////////////////////////

class yaml_reader : public text_reader {
    enum class multiline_style : u8 {
        Normal,
        Literal,
        Folded
    };

public:
    auto read_as_object(utf8_string_view txt) -> std::optional<object> override;
    auto read_as_array(utf8_string_view txt) -> std::optional<array> override;

private:
    auto parse_map() -> std::optional<object>;
    auto parse_sequence() -> std::optional<array>;
    auto parse_flow_map(entry& currentEntry) -> bool;
    auto parse_flow_sequence(entry& currentEntry) -> bool;
    auto parse_comment() -> std::optional<comment>;
    auto parse_block(entry& currentEntry) -> bool;
    auto parse_scalar(entry& currentEntry, multiline_style style) -> bool;
    auto parse_anchor() -> utf8_string;
    auto parse_alias() -> utf8_string;
    auto convert_scalar(entry& currentEntry, utf8_string const& value) -> bool;

    void next();
    void skip_next();
    auto check_current(yaml_tokenizer::token_type current) const -> bool;
    auto check_next(yaml_tokenizer::token_type next) const -> bool;

    yaml_tokenizer        _tokenizer;
    yaml_tokenizer::token _currentToken;
    usize                 _currentIndent {0};
    yaml_tokenizer::token _nextToken;
    usize                 _nextTokenIndex {0};

    flat_map<utf8_string, entry> _anchors;
};

//////////////////////////////////////////////////////////////////////

class yaml_writer : public text_writer {
public:
    auto write(ostream& stream, object const& obj) -> bool override;
    auto write(ostream& stream, array const& arr) -> bool override;

private:
    auto write_object(ostream& stream, i32 indent, object const& obj) const -> bool;
    auto write_array(ostream& stream, i32 indent, array const& arr) const -> bool;
    void write_entry(ostream& stream, i32 indent, entry const& ent) const;
    void write_scalar(ostream& stream, entry const& ent) const;
    void write_comment(ostream& stream, entry const& ent) const;
};
}
