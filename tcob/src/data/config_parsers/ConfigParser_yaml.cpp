// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_yaml.hpp"

#include "ConfigParser_json.hpp"

#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileStream.hpp"

namespace tcob::data::config::detail {

constexpr usize INDENT_SPACES {2};

//////////////////////////////////////////////////////////////////////
using token_type = yaml_tokenizer::token_type;

[[maybe_unused]] void static DumpTokens(std::vector<yaml_tokenizer::token>& tokens)
{
    io::ofstream fs {"out.txt"};
    for (auto& x : tokens) {
        utf8_string type;
        switch (x.Type) {
        case token_type::None:
            type = "None";
            break;
        case token_type::KeyOrScalar:
            type = "KeyOrScalar";
            break;
        case token_type::Whitespace:
            type = "Whitespace";
            break;
        case token_type::MappingKey:
            type = "MappingKey";
            break;
        case token_type::MappingValue:
            type = "MappingValue";
            break;
        case token_type::Sequence:
            type = "Sequence";
            break;
        case token_type::Comment:
            type = "Comment";
            break;
        case token_type::Newline:
            type = "Newline";
            break;
        case token_type::FlowSequence:
            type = "FlowSequence";
            break;
        case token_type::FlowMapping:
            type = "FlowMapping";
            break;
        case token_type::StartOfDocument:
            type = "StartOfDocument";
            break;
        case token_type::EndOfDocument:
            type = "EndOfDocument";
            break;
        case token_type::LiteralStyle:
            type = "LiteralStyle";
            break;
        case token_type::FoldedStyle:
            type = "FoldedStyle";
            break;
        case token_type::DoubleQuote:
            type = "DoubleQuote";
            break;
        case token_type::SingleQuote:
            type = "SingleQuote";
            break;
        case token_type::Tag:
            type = "Tag";
            break;
        case token_type::Anchor:
            type = "Anchor";
            break;
        case token_type::Alias:
            type = "Alias";
            break;
        case token_type::EoF:
            type = "EoF";
            break;
        case token_type::Indent:
            type = "Indent";
            break;
        }

        if (x.Type != token_type::Newline) {
            fs << type << ": " << x.Value << "\n";
        } else {
            fs << type << ": n\n";
        }
    }
}

auto static IsIgnored(yaml_tokenizer::token const& token) -> bool
{
    switch (token.Type) {
    case token_type::None:            // should never happen...
    case token_type::StartOfDocument: // don't care
    case token_type::EndOfDocument:   // don't care
    case token_type::Tag:             // not supported
    case token_type::MappingKey:      // gets 'optimized' away
        return true;
    case token_type::LiteralStyle:
    case token_type::FoldedStyle:
    case token_type::FlowSequence:
    case token_type::FlowMapping:
    case token_type::Anchor:
    case token_type::Alias:
    case token_type::KeyOrScalar:
    case token_type::Whitespace:
    case token_type::MappingValue:
    case token_type::Sequence:
    case token_type::Comment:
    case token_type::Newline:
    case token_type::DoubleQuote:
    case token_type::SingleQuote:
    case token_type::EoF:
    case token_type::Indent:
        return false;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

auto yaml_tokenizer::tokenize(utf8_string_view yaml) -> bool
{
    _yaml = yaml;

    while (!is_eof()) {
        auto const line {get_next_line()};
        if (!tokenize_line(line)) {
            return false;
        }
    }

    Tokens.emplace_back(token_type::EoF, "");
    optimize();
    return true;
}

auto yaml_tokenizer::tokenize_line(utf8_string_view line) -> bool
{
    for (usize i {0}; i < line.size(); ++i) {
        char const current {line[i]};
        char const next {i < line.size() - 1 ? line[i + 1] : '\0'};

        // StartOfDocument
        if (line.size() > i + 2 && current == '-' && next == '-' && line[i + 2] == '-') {
            Tokens.emplace_back(token_type::StartOfDocument, "---");
            i += 2;
            continue;
        }
        // EndOfDocument
        if (line.size() > i + 2 && current == '+' && next == '+' && line[i + 2] == '+') {
            Tokens.emplace_back(token_type::EndOfDocument, "+++");
            i += 2;
            continue;
        }

        // Whitespace
        if (current == ' ') {
            if (!Tokens.empty()) {
                if (Tokens.back().Type == token_type::Newline) {
                    Tokens.emplace_back(token_type::Indent, " ");
                } else if (Tokens.back().Type == token_type::Whitespace || Tokens.back().Type == token_type::Indent) {
                    Tokens.back().Value += " ";
                } else {
                    Tokens.emplace_back(token_type::Whitespace, " ");
                }
            } else {
                Tokens.emplace_back(token_type::Whitespace, " ");
            }
            continue;
        }

        // Tag
        if (!Tokens.empty() && Tokens.back().Type == token_type::Tag) {
            Tokens.back().Value += current;
            continue;
        }
        if (current == '!') {
            Tokens.emplace_back(token_type::Tag, "!");
            continue;
        }
        // MappingKey
        if (current == '?' && next == ' ') {
            Tokens.emplace_back(token_type::MappingKey, "?");
            ++i;
            continue;
        }
        // MappingValue
        if (current == ':' && (next == ' ' || next == '\n')) {
            Tokens.emplace_back(token_type::MappingValue, ":");
            if (next == ' ') {
                ++i;
            }
            continue;
        }
        // Sequence
        if (current == '-' && (next == ' ' || next == '\n')) {
            Tokens.emplace_back(token_type::Sequence, "-");
            if (next == ' ') {
                ++i;
            }
            continue;
        }
        // Comment
        if (current == '#') {
            Tokens.emplace_back(token_type::Comment, "#");
            continue;
        }
        // Newline
        if (current == '\n') {
            if (!Tokens.empty() && Tokens.back().Type == token_type::Newline) {
                Tokens.back().Value += "\n";
            } else {
                Tokens.emplace_back(token_type::Newline, "\n");
            }
            continue;
        }
        // LiteralStyle
        if (current == '|') {
            Tokens.emplace_back(token_type::LiteralStyle, "|");
            continue;
        }
        // FoldedStyle
        if (current == '>') {
            Tokens.emplace_back(token_type::FoldedStyle, ">");
            continue;
        }
        // DoubleQuote
        if (current == '"') {
            Tokens.emplace_back(token_type::DoubleQuote, "\"");
            continue;
        }
        // SingleQuote
        if (current == '\'') {
            Tokens.emplace_back(token_type::SingleQuote, "'");
            continue;
        }
        // Anchor
        if (current == '&') {
            Tokens.emplace_back(token_type::Anchor, "&");
            continue;
        }
        // Alias
        if (current == '*') {
            Tokens.emplace_back(token_type::Alias, "*");
            continue;
        }

        // FlowSequence
        if (current == '[') {
            usize const start {i};
            i = line.find(']');
            if (i == utf8_string::npos) {
                return false;
            }

            Tokens.emplace_back(token_type::FlowSequence, utf8_string {line.substr(start, i - start + 1)});
            continue;
        }
        // FlowMapping
        if (current == '{') {
            usize const start {i};
            i = line.find('}');
            if (i == utf8_string::npos) {
                return false;
            }

            Tokens.emplace_back(token_type::FlowMapping, utf8_string {line.substr(start, i - start + 1)});
            continue;
        }

        // KeyOrScalar
        if (!std::iscntrl(current)) {
            if (!Tokens.empty() && Tokens.back().Type == token_type::KeyOrScalar) {
                Tokens.back().Value += current;
            } else {
                Tokens.emplace_back(token_type::KeyOrScalar, utf8_string {current});
            }
        }
    }

    return true;
}

auto yaml_tokenizer::get_next_line() -> utf8_string_view
{
    if (is_eof()) {
        return "";
    }

    _yamlBegin = _yamlEnd;
    _yamlEnd   = _yaml.find('\n', _yamlEnd);
    if (_yamlEnd == utf8_string::npos) {
        _yamlEnd = _yaml.size() + 1;
    } else {
        ++_yamlEnd;
    }

    return _yaml.substr(_yamlBegin, _yamlEnd - _yamlBegin);
}

auto yaml_tokenizer::is_eof() const -> bool
{
    return _yamlEnd >= _yaml.size();
}

void yaml_tokenizer::optimize()
{
    for (usize i {0}; i < Tokens.size(); ++i) {
        if (i + 3 < Tokens.size()) {
            // MappingKey
            if (Tokens[i].Type == token_type::MappingKey
                && Tokens[i + 1].Type == token_type::KeyOrScalar
                && Tokens[i + 2].Type == token_type::Newline
                && Tokens[i + 3].Type == token_type::Indent) {
                Tokens[i].Type  = token_type::KeyOrScalar;
                Tokens[i].Value = Tokens[i + 1].Value;
                Tokens.erase(Tokens.begin() + static_cast<isize>(i) + 1, Tokens.begin() + static_cast<isize>(i) + 4);
            }
        }
        if (i + 1 < Tokens.size()) {
            // Tag
            if (Tokens[i].Type == token_type::Tag) {
                if (Tokens[i + 1].Type == token_type::Whitespace) {
                    Tokens.erase(Tokens.begin() + static_cast<isize>(i), Tokens.begin() + static_cast<isize>(i) + 2);
                } else {
                    Tokens.erase(Tokens.begin() + static_cast<isize>(i), Tokens.begin() + static_cast<isize>(i) + 1);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

auto yaml_reader::read_as_object(utf8_string_view txt) -> std::optional<object>
{
    if (_tokenizer.tokenize(txt)) {
        next();
        auto        obj {parse_map()};
        auto const& tokens {_tokenizer.Tokens};
        if (_nextTokenIndex >= tokens.size()
            || (_nextTokenIndex == tokens.size() - 1 && tokens.back().Type == token_type::EoF)) {
            return obj;
        }
    }
    return std::nullopt;
}

auto yaml_reader::read_as_array(utf8_string_view txt) -> std::optional<array>
{
    if (_tokenizer.tokenize(txt)) {
        next();
        auto        arr {parse_sequence()};
        auto const& tokens {_tokenizer.Tokens};
        if (_nextTokenIndex >= tokens.size()
            || (_nextTokenIndex == tokens.size() - 1 && tokens.back().Type == token_type::EoF)) {
            return arr;
        }
    }
    return std::nullopt;
}

// yamlMap ::= yamlNormalMap | yamlFlowMap
// yamlNormalMap ::= yamlKey MAP_SEPARATOR (CRLF | COMMENT)* (yamlBlock | yamlScalar)
auto yaml_reader::parse_map() -> std::optional<object>
{
    object  retValue;
    comment currentComment;

    for (;;) {
        entry currentEntry;

        if (check_current(token_type::EoF)) {
            return retValue;
        }

        if (check_current(token_type::Newline)) {
            if (!check_next(token_type::Indent) && _currentIndent > 0) {
                return retValue;
            }
            next();
            continue;
        }

        // check dedent
        if (check_current(token_type::Indent)) {
            usize const newIndent {_currentToken.Value.size()};
            if (newIndent < _currentIndent) {
                return retValue;
            }
            if (newIndent > _currentIndent) {
                break; // ERROR: unexpected indent
            }

            next();
            continue;
        }

        if (check_current(token_type::Whitespace)) {
            next();
            continue;
        }

        // map entry
        if (check_current(token_type::KeyOrScalar)
            && check_next(token_type::MappingValue)) {
            utf8_string const key {_currentToken.Value};
            skip_next();

            // flow
            if (parse_flow_map(currentEntry)) {
                retValue.set_entry(key, currentEntry);
                continue;
            }
            if (parse_flow_sequence(currentEntry)) {
                retValue.set_entry(key, currentEntry);
                continue;
            }

            // alias/anchor
            auto const aliasKey {parse_alias()};
            if (!aliasKey.empty()) {
                retValue.set_entry(key, _anchors[aliasKey]);
                continue;
            }
            auto const anchorKey {parse_anchor()};

            // block/scalar
            if (parse_block(currentEntry)
                || parse_scalar(currentEntry, multiline_style::Normal)) {
                currentEntry.set_comment(currentComment); // FIXME: trailing comments are assigned to following entry
                retValue.set_entry(key, currentEntry);
                if (!anchorKey.empty()) {
                    _anchors[anchorKey] = currentEntry;
                }
                currentComment = {};
                continue;
            }

            break; // ERROR: invalid entry
        }

        // comment
        if (auto comment {parse_comment()}) {
            currentComment = *comment;
            continue;
        }

        break;
    }

    return std::nullopt;
}

// yamlNormalSequence ::= SEQUENCE_ENTRY_INDICATOR (COMMENT | CRLF)* (yamlBlock | yamlScalar)
auto yaml_reader::parse_sequence() -> std::optional<array>
{
    array   retValue;
    comment currentComment;

    for (;;) {
        entry currentEntry;

        if (check_current(token_type::Newline)) {
            if (!check_next(token_type::Indent) && _currentIndent > 0) {
                return retValue;
            }
            next();
            continue;
        }

        // check dedent
        if (check_current(token_type::Indent)) {
            usize const newIndent {_currentToken.Value.size()};
            if (newIndent < _currentIndent) {
                return retValue;
            }
            if (newIndent > _currentIndent) {
                break; // ERROR: unexpected indent
            }

            next();
            continue;
        }

        if (check_current(token_type::Whitespace)) {
            next();
            continue;
        }

        if (!check_current(token_type::Sequence)) {
            return retValue;
        }
        next();

        // flow
        if (parse_flow_map(currentEntry)) {
            retValue.add_entry(currentEntry);
            next();
            continue;
        }
        if (parse_flow_sequence(currentEntry)) {
            retValue.add_entry(currentEntry);
            next();
            continue;
        }

        // alias/anchor
        auto const aliasKey {parse_alias()};
        if (!aliasKey.empty()) {
            retValue.add_entry(_anchors[aliasKey]);
            continue;
        }
        auto const anchorKey {parse_anchor()};

        if (parse_block(currentEntry)
            || parse_scalar(currentEntry, multiline_style::Normal)) {
            currentEntry.set_comment(currentComment);
            retValue.add_entry(currentEntry);
            currentComment = {};
            continue;
        }

        // comment
        if (auto comment {parse_comment()}) {
            currentComment = *comment;
            continue;
        }

        if (check_current(token_type::EoF)) {
            return retValue;
        }

        break;
    }

    return std::nullopt;
}

auto yaml_reader::parse_flow_map(entry& currentEntry) -> bool
{
    if (check_current(token_type::FlowMapping)) {
        auto retValue {json_reader::ReadObject(currentEntry, _currentToken.Value)};
        next();
        return retValue;
    }

    return false;
}

auto yaml_reader::parse_flow_sequence(entry& currentEntry) -> bool
{
    if (check_current(token_type::FlowSequence)) {
        auto retValue {json_reader::ReadArray(currentEntry, _currentToken.Value)};
        next();
        return retValue;
    }

    return false;
}

auto yaml_reader::parse_comment() -> std::optional<comment>
{
    if (check_current(token_type::Comment)) {
        utf8_string cmt;
        next();
        while (!check_current(token_type::Newline)
               && !check_current(token_type::EoF)) {
            cmt += _currentToken.Value;
            next();
        }
        return std::make_optional<comment>(cmt);
    }

    return std::nullopt;
}

// yamlBlock ::= yamlMap | yamlSequence
auto yaml_reader::parse_block(entry& currentEntry) -> bool
{
    multiline_style style {multiline_style::Normal};
    if (check_current(token_type::FoldedStyle)
        && check_next(token_type::Newline)) {
        style = multiline_style::Folded;
        next();
    } else if (check_current(token_type::LiteralStyle)
               && check_next(token_type::Newline)) {
        style = multiline_style::Literal;
        next();
    }

    if (check_current(token_type::Newline)) {
        usize const oldIndent {_currentIndent};
        usize       newIndent {oldIndent};

        next();

        if (check_current(token_type::Indent)) {
            newIndent = _currentToken.Value.size();
            if (oldIndent > newIndent) {
                return false;
            }
            next();
        }

        if (check_current(token_type::KeyOrScalar)) {
            // map
            if (check_next(token_type::MappingValue)) {
                _currentIndent = newIndent;
                if (auto obj {parse_map()}) {
                    _currentIndent = oldIndent;
                    currentEntry.set_value(*obj);
                    return true;
                }

                return false;
            }

            if (parse_scalar(currentEntry, style)) {
                return true;
            }
        }

        // sequence
        if (check_current(token_type::Sequence)) {
            _currentIndent = newIndent;
            if (auto arr {parse_sequence()}) {
                _currentIndent = oldIndent;
                currentEntry.set_value(*arr);
                return true;
            }

            return false;
        }
    }
    return false;
}

auto yaml_reader::parse_scalar(entry& currentEntry, multiline_style style) -> bool
{
    if (check_current(token_type::KeyOrScalar)) {
        utf8_string val {_currentToken.Value};

        next();

        // multiline check
        if (check_next(token_type::Indent)
            && _nextToken.Value.size() > _currentIndent) {
            usize multilineIndent {_nextToken.Value.size()};

            while (check_next(token_type::Indent)
                   && _nextToken.Value.size() == multilineIndent) {
                skip_next();
                if (check_current(token_type::KeyOrScalar)) {
                    if (style == multiline_style::Literal) {
                        val += "\n" + _currentToken.Value;
                    } else {
                        val += " " + _currentToken.Value;
                    }
                    next();
                }
            }
            if (style != multiline_style::Normal) {
                val += "\n";
            }
        }

        convert_scalar(currentEntry, val);

        return true;
    }

    if (check_current(token_type::SingleQuote)
        || check_current(token_type::DoubleQuote)) {
        utf8_string value;
        token_type  type {_currentToken.Type};
        next();
        while (!check_current(type)) {
            if (check_current(token_type::EoF)) {
                return false; // ERROR: unexpected EOF
            }

            value += _currentToken.Value;
            next();
        }

        convert_scalar(currentEntry, value);
        next();
        return true;
    }

    return false;
}

auto yaml_reader::parse_anchor() -> utf8_string
{
    utf8_string retValue;
    if (check_current(token_type::Anchor)
        && check_next(token_type::KeyOrScalar)) {
        retValue = _nextToken.Value;
        skip_next();
    }

    return retValue;
}

auto yaml_reader::parse_alias() -> utf8_string
{
    // alias
    if (check_current(token_type::Alias)
        && check_next(token_type::KeyOrScalar)) {
        utf8_string aliasKey {_nextToken.Value};
        if (!_anchors.contains(aliasKey)) {
            return "";
        }

        skip_next();
        return aliasKey;
    }

    return "";
}

auto yaml_reader::convert_scalar(entry& currentEntry, utf8_string const& value) -> bool
{
    if (auto intVal {helper::to_number<i64>(value)}) {
        currentEntry.set_value(*intVal);
        return true;
    }

    if (auto floatVal {helper::to_number<f64>(value)}) {
        currentEntry.set_value(*floatVal);
        return true;
    }

    // bool
    if (value == "true" || value == "false") {
        currentEntry.set_value(value == "true");
        return true;
    }

    // utf8_string
    currentEntry.set_value(value);
    return true;
}

void yaml_reader::next()
{
    _currentToken = _tokenizer.Tokens[_nextTokenIndex++];
    while (IsIgnored(_currentToken)) {
        _currentToken = _tokenizer.Tokens[_nextTokenIndex++];
    }

    if (!check_current(token_type::EoF)) {
        _nextToken = _tokenizer.Tokens[_nextTokenIndex];
    }
}

void yaml_reader::skip_next()
{
    next();
    next();
}

auto yaml_reader::check_current(token_type current) const -> bool
{
    return current == _currentToken.Type;
}

auto yaml_reader::check_next(token_type next) const -> bool
{
    return next == _nextToken.Type;
}

//////////////////////////////////////////////////////////////////////

auto yaml_writer::write(ostream& stream, object const& obj) -> bool
{
    stream << "---\n";
    return write_object(stream, 0, obj);
}

auto yaml_writer::write(ostream& stream, array const& arr) -> bool
{
    return write_array(stream, 0, arr);
}

auto yaml_writer::write_object(ostream& stream, usize indent, object const& obj) const -> bool
{
    if (obj.empty()) {
        stream << "{}";
        return true;
    }

    utf8_string indentString(indent, ' ');

    for (auto const& [k, v] : obj) {
        write_comment(stream, v);
        stream << indentString << k << ": ";
        write_entry(stream, indent, v);
    }

    stream.seek(-1, io::seek_dir::Current);

    return true;
}

auto yaml_writer::write_array(ostream& stream, usize indent, array const& arr) const -> bool
{
    if (arr.empty()) {
        stream << "[]";
        return true;
    }

    utf8_string indentString(indent, ' ');

    for (auto const& v : arr) {
        write_comment(stream, v);
        stream << indentString << "- ";
        write_entry(stream, indent, v);
    }

    stream.seek(-1, io::seek_dir::Current);

    return true;
}

void yaml_writer::write_entry(ostream& stream, usize indent, entry const& ent) const
{
    if (object csec; ent.try_get(csec)) {
        if (!csec.empty()) { stream << "\n"; }
        write_object(stream, indent + INDENT_SPACES, csec);
    } else if (array carr; ent.try_get(carr)) {
        if (!carr.empty()) { stream << "\n"; }
        write_array(stream, indent + INDENT_SPACES, carr);
    } else {
        write_scalar(stream, ent);
    }
    stream << "\n";
}

void yaml_writer::write_scalar(ostream& stream, entry const& ent) const
{
    stream << ent.as<utf8_string>();
}

void yaml_writer::write_comment(ostream& stream, entry const& ent) const
{
    auto const& comment {ent.get_comment()};
    for (auto const& cl : helper::split(comment.Text, '\n')) {
        stream << "# " << cl << "\n";
    }
}
}
