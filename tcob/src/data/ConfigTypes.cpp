// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/ConfigTypes.hpp"

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "config_formats/Config_ini.hpp"

#include "tcob/core/Proxy.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/MemoryStream.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigConversions.hpp"

namespace tcob::data {

////////////////////////////////////////////////////////////

object::object() noexcept
    : object {std::make_shared<cfg_object_entries>()}
{
    reserve(10);
}

object::object(std::initializer_list<std::pair<utf8_string, cfg_value>> items)
    : object {}
{
    reserve(items.size());
    for (auto const& item : items) {
        set(item.first, item.second);
    }
}

object::object(std::shared_ptr<cfg_object_entries> const& entries) noexcept
    : base_type<object, cfg_object_entries> {entries}
{
}

void object::set(string_view key, std::nullptr_t)
{
    remove_entry(key);
}

auto object::str() const -> string
{
    io::iomstream stream {};
    detail::ini_writer {}.write_inline_section(stream, *this, 1000);
    stream.seek(0, io::seek_dir::Begin);
    return stream.read_string(stream.size_in_bytes());
}

auto object::on_load(io::istream& in, string const& ext, bool skipBinary) noexcept -> bool
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_magic(in, ext)}) {
            if (auto result {binParser->read_as_object(in)}) {
                swap(*result);
                return true;
            }
            return false;
        }
    }

    if (!parse(in.read_string(in.size_in_bytes()), ext)) {
        return false;
    }

    return true;
}

auto object::parse(string_view config, string const& ext) noexcept -> bool
{
    std::optional<object> result;
    if (auto txtParser {locate_service<text_reader::factory>().create(ext)}) {
        result = txtParser->read_as_object(config);
    }

    if (result) {
        swap(*result);
        return true;
    }

    return false;
}

auto object::clone(bool deep) const -> object
{
    object retValue;

    if (deep) {
        for (auto const& [k, v] : *values()) {
            auto const type {get_type(k)};
            switch (type) {
            case type::Null: break;
            case type::String:
            case type::Float:
            case type::Integer:
            case type::Bool:
                retValue.add_entry(k, v);
                break;
            case type::Array:
                retValue.add_entry(k, entry {v.as<array>().clone(true)});
                break;
            case type::Object:
                retValue.add_entry(k, entry {v.as<object>().clone(true)});
                break;
            }
        }
    } else {
        for (auto const& entry : *values()) {
            retValue.add_entry(entry.first, entry.second);
        }
    }

    return retValue;
}

void object::merge(object const& other, bool onConflictTakeOther)
{
    for (auto const& [k, v] : *other.values()) {
        if (!has(k)) {
            set_entry(k, v);
        } else {
            if (v.is<object>()) {
                as<object>(k).merge(v.as<object>(), onConflictTakeOther);
            } else if (onConflictTakeOther) {
                set_entry(k, v);
            }
        }
    }
}

auto object::Parse(string_view config, string const& ext) -> std::optional<object>
{
    object retValue;
    return retValue.parse(config, ext) ? std::optional {retValue} : std::nullopt;
}

auto object::get_entry(string_view key) -> entry*
{
    if (auto it {find(key)}; it != values()->end()) {
        return &it->second;
    }

    return nullptr;
}

auto object::get_entry(string_view key) const -> entry const*
{
    if (auto const it {find(key)}; it != values()->end()) {
        return &it->second;
    }

    return nullptr;
}

void object::set_entry(string_view key, entry const& entry)
{
    if (auto it {find(key)}; it != values()->end()) {
        it->second = entry;
        return;
    }

    // new key
    add_entry(key, entry);
}

void object::add_entry(string_view key, entry const& entry)
{
    values()->emplace_back(key, entry);
}

void object::remove_entry(string_view key)
{
    values()->erase(find(key));
}

auto object::find(string_view key) -> cfg_object_entries::iterator
{
    return std::ranges::find_if(*this, [&key](auto const& p) { return p.first == key; });
}

auto object::find(string_view key) const -> cfg_object_entries::const_iterator
{
    return std::ranges::find_if(*this, [&key](auto const& p) { return p.first == key; });
}

////////////////////////////////////////////////////////////

array::array() noexcept
    : base_type<array, cfg_array_entries> {std::make_shared<cfg_array_entries>()}
{
}

auto array::operator[](isize index) -> proxy<array, isize>
{
    return proxy<array, isize> {*this, std::tuple {index}};
}

auto array::operator[](isize index) const -> proxy<array const, isize>
{
    return proxy<array const, isize> {*this, std::tuple {index}};
}

auto array::on_load(io::istream& in, string const& ext, bool skipBinary) noexcept -> bool
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_magic(in, ext)}) {
            if (auto result {binParser->read_as_array(in)}) {
                swap(*result);
                return true;
            }
            return false;
        }
    }

    if (!parse(in.read_string(in.size_in_bytes()), ext)) {
        return false;
    }

    return true;
}

auto array::parse(string_view config, string const& ext) -> bool
{
    std::optional<array> result;
    if (auto txtParser {locate_service<text_reader::factory>().create(ext)}) {
        result = txtParser->read_as_array(config);
    }

    if (result) {
        swap(*result);
        return true;
    }

    return false;
}

void array::pop_back()
{
    values()->pop_back();
}

auto array::clone(bool deep) const -> array
{
    array retValue;
    auto* dst {retValue.values()};
    auto* src {values()};

    if (deep) {
        for (isize i {0}; i < size(); ++i) {
            auto const type {get_type(i)};
            switch (type) {
            case type::Null:
            case type::String:
            case type::Float:
            case type::Integer:
            case type::Bool:
                dst->emplace_back(src->at(i));
                break;
            case type::Array:
                dst->emplace_back(src->at(i).as<array>().clone(true));
                break;
            case type::Object:
                dst->emplace_back(src->at(i).as<object>().clone(true));
                break;
            }
        }
    } else {
        for (auto const& entry : *src) {
            dst->push_back(entry);
        }
    }

    return retValue;
}

auto array::str() const -> string
{
    io::iomstream stream {};
    detail::ini_writer {}.write(stream, *this);
    stream.seek(0, io::seek_dir::Begin);
    return stream.read_string(stream.size_in_bytes());
}

auto array::Parse(string_view config, string const& ext) -> std::optional<array>
{
    array retValue;
    return retValue.parse(config, ext) ? std::optional {retValue} : std::nullopt;
}

auto array::get_entry(isize index) const -> entry*
{
    if (index >= size()) { return nullptr; }

    return &values()->at(index);
}

void array::add_entry(entry const& newEntry)
{
    values()->push_back(newEntry);
}

////////////////////////////////////////////////////////////

auto entry::get_comment() const -> comment const&
{
    return _comment;
}

void entry::set_comment(comment const& comment)
{
    _comment = comment;
}

}

auto tcob::literals::operator""_ini(char const* str, usize) -> tcob::data::object
{
    tcob::data::object retValue {};
    retValue.parse(string {str}, ".ini");
    return retValue;
}

auto tcob::literals::operator""_json(char const* str, usize) -> tcob::data::object
{
    tcob::data::object retValue {};
    retValue.parse(string {str}, ".json");
    return retValue;
}

auto tcob::literals::operator""_xml(char const* str, usize) -> tcob::data::object
{
    tcob::data::object retValue {};
    retValue.parse(string {str}, ".xml");
    return retValue;
}

auto tcob::literals::operator""_yaml(char const* str, usize) -> tcob::data::object
{
    tcob::data::object retValue {};
    retValue.parse(string {str}, ".yaml");
    return retValue;
}
