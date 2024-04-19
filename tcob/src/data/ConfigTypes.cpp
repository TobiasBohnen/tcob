// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/ConfigTypes.hpp"

#include <algorithm>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/MemoryStream.hpp"

#include "config_parsers/ConfigParser_ini.hpp"

namespace tcob::data::config {

////////////////////////////////////////////////////////////

object::object() noexcept
    : object {std::make_shared<cfg_object_entries>()}
{
    _kvps->reserve(10);
}

object::object(std::shared_ptr<cfg_object_entries> const& entries) noexcept
    : _kvps {entries}
{
}

auto object::operator[](string const& key) -> proxy<object, string>
{
    return proxy<object, string> {*this, std::tuple {key}};
}

auto object::operator[](string const& key) const -> proxy<object const, string>
{
    return proxy<object const, string> {*this, std::tuple {key}};
}

auto object::begin() -> cfg_object_entries::iterator
{
    return _kvps->begin();
}

auto object::begin() const -> cfg_object_entries::const_iterator
{
    return _kvps->begin();
}

auto object::end() -> cfg_object_entries::iterator
{
    return _kvps->end();
}

auto object::end() const -> cfg_object_entries::const_iterator
{
    return _kvps->end();
}

auto object::empty() const -> bool
{
    return _kvps->empty();
}

auto object::get_size() const -> isize
{
    return std::ssize(*_kvps);
}

void object::clear()
{
    _kvps->clear();
}

auto object::get_type(string const& key) const -> type
{
    if (is<string>(key)) {
        return type::String;
    }
    if (is<i64>(key)) {
        return type::Integer;
    }
    if (is<f64>(key)) {
        return type::Float;
    }
    if (is<bool>(key)) {
        return type::Bool;
    }
    if (is<array>(key)) {
        return type::Array;
    }
    if (is<object>(key)) {
        return type::Object;
    }

    return type::Null;
}

auto object::get_entry(string const& key) const -> entry*
{
    for (auto& [k, v] : *_kvps) {
        if (helper::case_insensitive_equals(k, key)) {
            return &v;
        }
    }

    return nullptr;
}

void object::set(string const& key, std::nullptr_t)
{
    std::erase_if(*_kvps, [&key](auto const& p) { return helper::case_insensitive_equals(p.first, key); });
}

void object::add_entry(string const& key, entry const& entry)
{
    _kvps->emplace_back(key, entry);
}

void object::set_entry(string const& key, entry const& entry)
{
    for (auto& [k, v] : *_kvps) {
        if (helper::case_insensitive_equals(k, key)) {
            v = entry;
            return;
        }
    }

    // new key
    add_entry(key, entry);
}

auto object::str() const -> string
{
    io::iomstream stream {};
    detail::ini_writer {}.write_inline_section(stream, *this);
    stream.seek(0, io::seek_dir::Begin);
    return stream.read_string(stream.size_in_bytes());
}

auto object::load(path const& file, bool skipBinary) noexcept -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) {
        return load(*fs, io::get_extension(file), skipBinary);
    }

    return load_status::FileNotFound;
}

auto object::load(istream& in, string const& ext, bool skipBinary) noexcept -> load_status
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_sig_or_ext(in, ext)}) {
            if (auto result {binParser->read_as_object(in)}) {
                _kvps = result->_kvps;
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

auto object::load_async(path const& file, bool skipBinary) noexcept -> std::future<load_status>
{
    return std::async(std::launch::async, [&, file, skipBinary] { return load(file, skipBinary); });
}

auto object::parse(string_view config, string const& ext) noexcept -> bool
{
    std::optional<object> result;
    if (auto txtParser {locate_service<text_reader::factory>().create(ext)}) {
        result = txtParser->read_as_object(config);
    }

    if (result) {
        std::swap(_kvps, result->_kvps);
        return true;
    }

    return false;
}

auto object::save(path const& file) const -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto object::save(ostream& out, string const& ext) const -> bool
{
    if (auto txtWriter {locate_service<text_writer::factory>().create(ext)}) {
        return txtWriter->write(out, *this);
    }

    if (auto binWriter {locate_service<binary_writer::factory>().create(ext)}) {
        return binWriter->write(out, *this);
    }

    return false;
}

auto object::clone(bool deep) const -> object
{
    object retValue;
    if (deep) {
        for (auto const& [k, v] : *_kvps) {
            auto const type {get_type(k)};
            switch (type) {
            case type::Null: break;
            case type::String:
            case type::Float:
            case type::Integer:
            case type::Bool:
                retValue._kvps->emplace_back(k, v);
                break;
            case type::Array:
                retValue._kvps->emplace_back(k, v.as<array>().clone(true));
                break;
            case type::Object:
                retValue._kvps->emplace_back(k, v.as<object>().clone(true));
                break;
            }
        }
    } else {
        for (auto const& entry : *_kvps) {
            retValue._kvps->push_back(entry);
        }
    }

    return retValue;
}

void object::merge(object const& other, bool onConflictTakeOther)
{
    for (auto const& [k, v] : *other._kvps) {
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

auto object::find_key(string const& key) -> cfg_object_entries::iterator
{
    return std::find_if(_kvps->begin(), _kvps->end(), [&key](auto const& p) { return helper::case_insensitive_equals(p.first, key); });
}

auto object::find_key(string const& key) const -> cfg_object_entries::const_iterator
{
    return std::find_if(_kvps->begin(), _kvps->end(), [&key](auto const& p) { return helper::case_insensitive_equals(p.first, key); });
}

////////////////////////////////////////////////////////////

array::array() noexcept
    : _values {std::make_shared<cfg_array_entries>()}
{
    _values->reserve(10);
}

auto array::operator[](isize index) -> proxy<array, isize>
{
    return proxy<array, isize> {*this, std::tuple {index}};
}

auto array::operator[](isize index) const -> proxy<array const, isize>
{
    return proxy<array const, isize> {*this, std::tuple {index}};
}

auto array::load(path const& file, bool skipBinary) -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) {
        return load(*fs, io::get_extension(file), skipBinary);
    }

    return load_status::FileNotFound;
}

auto array::load(istream& in, string const& ext, bool skipBinary) -> load_status
{
    if (!skipBinary) {
        if (auto binParser {locate_service<binary_reader::factory>().create_from_sig_or_ext(in, ext)}) {
            if (auto result {binParser->read_as_array(in)}) {
                _values = result->_values;
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

auto array::load_async(path const& file) -> std::future<load_status>
{
    return std::async(std::launch::async, [&, file] { return load(file); });
}

auto array::parse(string_view config, string const& ext) -> bool
{
    std::optional<array> result;
    if (auto txtParser {locate_service<text_reader::factory>().create(ext)}) {
        result = txtParser->read_as_array(config);
    }

    if (result) {
        std::swap(_values, result->_values);
        return true;
    }

    return false;
}

auto array::save(path const& file) const -> bool
{
    io::ofstream of {file};
    return save(of, io::get_extension(file));
}

auto array::save(ostream& out, string const& ext) const -> bool
{
    if (auto txtWriter {locate_service<text_writer::factory>().create(ext)}) {
        return txtWriter->write(out, *this);
    }

    if (auto binWriter {locate_service<binary_writer::factory>().create(ext)}) {
        return binWriter->write(out, *this);
    }

    return false;
}

auto array::begin() -> cfg_array_entries::iterator
{
    return _values->begin();
}

auto array::begin() const -> cfg_array_entries::const_iterator
{
    return _values->begin();
}

auto array::end() -> cfg_array_entries::iterator
{
    return _values->end();
}

auto array::end() const -> cfg_array_entries::const_iterator
{
    return _values->end();
}

auto array::empty() const -> bool
{
    return _values->empty();
}

auto array::get_size() const -> isize
{
    return std::ssize(*_values);
}

void array::clear()
{
    _values->clear();
}

auto array::get_type(isize index) const -> type
{
    if (is<string>(index)) {
        return type::String;
    }
    if (is<i64>(index)) {
        return type::Integer;
    }
    if (is<f64>(index)) {
        return type::Float;
    }
    if (is<bool>(index)) {
        return type::Bool;
    }
    if (is<array>(index)) {
        return type::Array;
    }
    if (is<object>(index)) {
        return type::Object;
    }

    return type::Null;
}

auto array::get_entry(isize index) const -> entry*
{
    if (index >= std::ssize(*_values)) {
        return nullptr;
    }

    return &_values->at(index);
}

void array::add_entry(entry const& newEntry)
{
    _values->push_back(newEntry);
}

void array::pop_back()
{
    _values->pop_back();
}

auto array::clone(bool deep) const -> array
{
    array retValue;
    if (deep) {
        for (isize i {0}; i < get_size(); ++i) {
            auto const type {get_type(i)};
            switch (type) {
            case type::Null:
            case type::String:
            case type::Float:
            case type::Integer:
            case type::Bool:
                retValue._values->emplace_back(_values->at(i));
                break;
            case type::Array:
                retValue._values->emplace_back(_values->at(i).as<array>().clone(true));
                break;
            case type::Object:
                retValue._values->emplace_back(_values->at(i).as<object>().clone(true));
                break;
            }
        }
    } else {
        for (auto const& entry : *_values) {
            retValue._values->push_back(entry);
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
