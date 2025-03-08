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

#include "config_parsers/ConfigParser_ini.hpp"

#include "tcob/core/Common.hpp"
#include "tcob/core/Proxy.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/MemoryStream.hpp"
#include "tcob/core/io/Stream.hpp"
#include "tcob/data/Config.hpp"
#include "tcob/data/ConfigConversions.hpp"

namespace tcob::data::config {

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

auto object::operator[](string const& key) -> proxy<object, string>
{
    return proxy<object, string> {*this, std::tuple {key}};
}

auto object::operator[](string const& key) const -> proxy<object const, string> const
{
    return proxy<object const, string> {*this, std::tuple {key}};
}

void object::set(string_view key, std::nullptr_t)
{
    values()->erase(find(key));
}

auto object::str() const -> string
{
    io::iomstream stream {};
    detail::ini_writer {}.write_inline_section(stream, *this);
    stream.seek(0, io::seek_dir::Begin);
    return stream.read_string(stream.size_in_bytes());
}

auto object::on_load(io::istream& in, string const& ext, bool skipBinary) noexcept -> load_status
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_sig_or_ext(in, ext)}) {
            if (auto result {binParser->read_as_object(in)}) {
                swap(*result);
                return load_status::Ok;
            }
            return load_status::Error;
        }
    }

    if (!parse(in.read_string(in.size_in_bytes()), ext)) {
        return load_status::Error;
    }

    return load_status::Ok;
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
    auto*  dst {retValue.values()};

    if (deep) {
        for (auto const& [k, v] : *values()) {
            auto const type {get_type(k)};
            switch (type) {
            case type::Null: break;
            case type::String:
            case type::Float:
            case type::Integer:
            case type::Bool:
                dst->emplace_back(k, v);
                break;
            case type::Array:
                dst->emplace_back(k, v.as<array>().clone(true));
                break;
            case type::Object:
                dst->emplace_back(k, v.as<object>().clone(true));
                break;
            }
        }
    } else {
        for (auto const& entry : *values()) {
            dst->push_back(entry);
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

auto object::get_entry(string_view key) const -> entry*
{
    for (auto& [k, v] : *values()) {
        if (k == key) { return &v; }
    }

    return nullptr;
}

void object::set_entry(string_view key, entry const& entry)
{
    for (auto& [k, v] : *values()) {
        if (k == key) {
            v = entry;
            return;
        }
    }

    // new key
    add_entry(key, entry);
}

void object::add_entry(string_view key, entry const& entry)
{
    values()->emplace_back(key, entry);
}

auto object::find(string_view key) -> cfg_object_entries::iterator
{
    return std::find_if(begin(), end(), [&key](auto const& p) { return p.first == key; }); // NOLINT(modernize-use-ranges)
}

auto object::find(string_view key) const -> cfg_object_entries::const_iterator
{
    return std::find_if(begin(), end(), [&key](auto const& p) { return p.first == key; }); // NOLINT(modernize-use-ranges)
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

auto array::on_load(io::istream& in, string const& ext, bool skipBinary) noexcept -> load_status
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_sig_or_ext(in, ext)}) {
            if (auto result {binParser->read_as_array(in)}) {
                swap(*result);
                return load_status::Ok;
            }
            return load_status::Error;
        }
    }

    if (!parse(in.read_string(in.size_in_bytes()), ext)) {
        return load_status::Error;
    }

    return load_status::Ok;
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

entry::entry() noexcept = default;

auto entry::get_comment() const -> comment const&
{
    return _comment;
}

void entry::set_comment(comment const& comment)
{
    _comment = comment;
}

}

auto tcob::literals::operator""_ini(char const* str, usize) -> tcob::data::config::object
{
    tcob::data::config::object retValue {};
    retValue.parse(string {str}, ".ini");
    return retValue;
}

auto tcob::literals::operator""_json(char const* str, usize) -> tcob::data::config::object
{
    tcob::data::config::object retValue {};
    retValue.parse(string {str}, ".json");
    return retValue;
}

auto tcob::literals::operator""_xml(char const* str, usize) -> tcob::data::config::object
{
    tcob::data::config::object retValue {};
    retValue.parse(string {str}, ".xml");
    return retValue;
}

auto tcob::literals::operator""_yaml(char const* str, usize) -> tcob::data::config::object
{
    tcob::data::config::object retValue {};
    retValue.parse(string {str}, ".yaml");
    return retValue;
}
