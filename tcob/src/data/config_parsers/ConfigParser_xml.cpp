// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_xml.hpp"

#include <algorithm>
#include <charconv>

#include "tcob/core/StringUtils.hpp"

namespace tcob::data::config::detail {

constexpr u32 INDENT_SPACES {2};

auto xml_reader::read_as_object(utf8_string_view txt) -> std::optional<object>
{
    _xml = txt;
    if (auto element {read_element()}) {
        return convert_to_object(*element);
    }

    return std::nullopt;
}

auto xml_reader::read_as_array(utf8_string_view txt) -> std::optional<array>
{
    _xml = txt;
    if (auto element {read_element()}) {
        return convert_to_array(*element);
    }

    return std::nullopt;
}

auto xml_reader::read_element() -> std::unique_ptr<element>
{
    if (auto tag {read_tag()}) {
        auto retValue {std::make_unique<element>()};
        retValue->Tag = *tag;
        if (tag->Type == tag_type::SelfClosing) {
            return retValue;
        }

        if (!read_content(*retValue)) {
            return nullptr;
        }

        if (auto closeTag {read_tag()}) {
            if (closeTag->Name == tag->Name) {
                return retValue;
            }
        }
    }

    return nullptr;
}

auto xml_reader::read_tag() -> std::optional<tag>
{
    skip_whitespace();
    if (read() == '<') {
        tag retValue;
        if (peek() == '/') {
            retValue.Type = tag_type::Closing;
            advance();
        }

        if (!read_tag_name(retValue)) { return std::nullopt; }

        if (retValue.Type != tag_type::Closing) {
            if (peek() != '>' && peek() != '/') {
                if (!read_tag_attributes(retValue)) { return std::nullopt; }
            }

            if (peek() == '/') {
                retValue.Type = tag_type::SelfClosing;
                advance();
            }

            skip_whitespace();
        }

        if (read() != '>') { return std::nullopt; }

        return retValue;
    }

    return std::nullopt;
}

auto xml_reader::read_tag_name(tag& t) -> bool
{
    while (!is_eof()) {
        char const c {read()};
        if (std::isalnum(c) || c == '.' || c == '_' || c == '-') {
            t.Name += c;
        } else if (std::isspace(c) || c == '>' || c == '/') {
            step_back();
            return true;
        } else {
            return false;
        }
    }

    return false;
}

auto xml_reader::read_tag_attributes(tag& t) -> bool
{
    while (!is_eof()) {
        auto const key {read_tag_attribute_key()};
        auto const value {read_tag_attribute_value()};
        if (key.empty() || value.empty()) {
            return false;
        }

        t.Attributes.insert(key, value);

        skip_whitespace();
        if (peek() == '>' || peek() == '/') {
            return true;
        }
    }

    return false;
}

auto xml_reader::read_tag_attribute_key() -> utf8_string
{
    utf8_string retValue;
    skip_whitespace();

    while (!is_eof()) {
        char const c {read()};
        if (std::isspace(c) || c == '=') {
            step_back();
            return retValue;
        }
        retValue += c;
    }
    return "";
}

auto xml_reader::read_tag_attribute_value() -> utf8_string
{
    utf8_string retValue;
    skip_whitespace();
    if (read() != '=') {
        return "";
    }
    skip_whitespace();
    if (read() != '"') {
        return "";
    }

    while (!is_eof()) {
        char const c {read()};
        if (c == '"') {
            return retValue;
        }
        retValue += c;
    }

    return "";
}

auto xml_reader::read_content(element& n) -> bool
{
    skip_whitespace();

    while (!is_eof()) {
        char const c {read()};
        if (c == '<') {
            if (peek() == '/') {
                step_back();
                n.Value = helper::trim(n.Value);
                return true;
            }

            step_back();
            if (auto child {read_element()}) {
                n.Children.push_back(std::move(child));
            } else {
                return false;
            }
        } else {
            n.Value += c;
        }
    }
    return false;
}

auto xml_reader::is_eof() const -> bool
{
    return _xmlIndex >= _xml.size();
}

void xml_reader::skip_whitespace()
{
    while (!is_eof() && std::isspace(peek())) {
        advance();
    }
}

auto xml_reader::peek() -> char
{
    if (is_eof()) {
        return '\0';
    }

    return _xml[_xmlIndex];
}

auto xml_reader::read() -> char
{
    if (is_eof()) {
        return '\0';
    }

    return _xml[_xmlIndex++];
}

void xml_reader::advance()
{
    ++_xmlIndex;
}

void xml_reader::step_back()
{
    --_xmlIndex;
}

auto xml_reader::convert_to_object(element const& n) -> object
{
    object retValue;
    for (auto const& [k, v] : n.Tag.Attributes) {
        entry e;
        convert_value(e, v);
        retValue.set_entry(k, e);
    }
    if (!n.Value.empty()) {
        entry e {n.Value};
        retValue.set_entry("", e);
    }

    for (auto const& el : n.Children) {
        // entry -> childless, single attribute named 'value' or non-empty value
        if (el->Children.empty()
            && ((el->Tag.Attributes.size() == 1 && el->Tag.Attributes.front().first == "value")
                || !el->Value.empty())) {
            entry e;
            if (el->Tag.Attributes.size() == 1) {
                convert_value(e, el->Tag.Attributes.front().first);
            } else {
                convert_value(e, el->Value);
            }

            retValue.set_entry(el->Tag.Name, e);
        } else {
            bool isArray {false};
            // array -> no attributes or value...
            if (el->Tag.Attributes.empty() && el->Value.empty() && !el->Children.empty()) {
                // and children have the same name
                if (el->Children.size() > 1) {
                    auto const& itemName {el->Children[0]->Tag.Name};
                    isArray = std::all_of(el->Children.begin() + 1, el->Children.end(), [&itemName](auto const& entry) {
                        return entry->Tag.Name == itemName;
                    });
                }
                // or an only child named 'item'
                else {
                    isArray = el->Children[0]->Tag.Name == "item";
                }
            }

            if (isArray) {
                entry e {convert_to_array(*el)};
                retValue.set_entry(el->Tag.Name, e);
            } else {
                entry e {convert_to_object(*el)};
                retValue.set_entry(el->Tag.Name, e);
            }
        }
    }

    return retValue;
}

auto xml_reader::convert_to_array(element const& n) -> array
{
    array retValue;
    for (auto const& el : n.Children) {
        if (el->Children.empty()
            && ((el->Tag.Attributes.size() == 1 && el->Tag.Attributes.front().first == "value")
                || !el->Value.empty())) {
            entry entry;
            if (el->Tag.Attributes.size() == 1) {
                convert_value(entry, el->Tag.Attributes.front().first);
            } else {
                convert_value(entry, el->Value);
            }

            retValue.add_entry(entry);
        } else {
            retValue.add_entry(convert_to_object(*el));
        }
    }
    return retValue;
}

void xml_reader::convert_value(entry& currentEntry, utf8_string const& str)
{
    // number
    auto const* valueStrData {str.data()};
    auto const  valueStrSize {str.size()};

    {
        i64 valueInt {0};
        auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, valueInt)};
        if (ec == std::errc {} && p == valueStrData + valueStrSize) {
            currentEntry.set_value(valueInt);
            return;
        }
    }

    {
        f64 valueFloat {0};
        auto [p, ec] {std::from_chars(valueStrData, valueStrData + valueStrSize, valueFloat)};
        if (ec == std::errc {} && p == valueStrData + valueStrSize) {
            currentEntry.set_value(valueFloat);
            return;
        }
    }

    // bool
    if (str == "true" || str == "false") {
        currentEntry.set_value(str == "true");
        return;
    }

    // utf8_string
    currentEntry.set_value(str);
}

//////////////////////////////////////////////////////////////////////

auto xml_writer::write(ostream& stream, object const& obj) -> bool
{
    return write_element(stream, 0, "root", obj);
}

auto xml_writer::write(ostream& stream, array const& arr) -> bool
{
    return write_array(stream, 0, "root", arr);
}

auto xml_writer::write_element(ostream& stream, u32 indent, utf8_string const& name, object const& obj) const -> bool
{
    std::map<utf8_string, object> sections;
    std::map<utf8_string, array>  arrays;

    utf8_string indentString(indent, ' ');
    stream << indentString << "<" << name;

    for (auto const& [k, v] : obj) {
        if (v.is<object>()) {
            sections[k] = v.as<object>();
        } else if (v.is<array>()) {
            arrays[k] = v.as<array>();
        } else {
            write_attribute(stream, k, v);
        }
    }

    if (sections.empty() && arrays.empty()) {
        stream << "/>\n";
    } else {
        stream << ">\n";

        for (auto const& [k, v] : sections) {
            if (!write_element(stream, indent + INDENT_SPACES, k, v)) {
                return false;
            }
        }

        for (auto const& [k, v] : arrays) {
            if (!write_array(stream, indent + INDENT_SPACES, k, v)) {
                return false;
            }
        }

        stream << indentString << "</" << name << ">\n";
    }

    return true;
}

auto xml_writer::write_array(ostream& stream, u32 indent, utf8_string const& name, array const& arr) const -> bool
{
    utf8_string indentArray(indent, ' ');
    utf8_string indentValues(indent + INDENT_SPACES, ' ');
    stream << indentArray << "<" << name << ">\n";

    for (auto const& v : arr) {
        if (v.is<object>()) {
            write_element(stream, indent + INDENT_SPACES, "item", v.as<object>());
        } else if (v.is<array>()) {
            write_array(stream, indent + INDENT_SPACES, "item", v.as<array>());
        } else {
            stream << indentValues << "<item>";
            stream << v.as<utf8_string>();
            stream << "</item>\n";
        }
    }
    stream << indentArray << "</" << name << ">\n";
    return true;
}

void xml_writer::write_attribute(ostream& stream, utf8_string const& name, entry const& ent) const
{
    stream << " " << name << " = \"" << ent.as<utf8_string>() << "\"";
}

}
