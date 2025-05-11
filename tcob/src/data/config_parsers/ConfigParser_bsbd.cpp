// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_bsbd.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <ios>
#include <limits>
#include <optional>

#include "tcob/core/io/Stream.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::config::detail {

constexpr std::array<ubyte, 5> MAGIC {'B', 'S', 'B', 'D', 1};
constexpr u8                   LitIntVal {static_cast<u8>(bsbd::marker_type::LitInt)};

using pool_element_size = u8;

auto bsbd_reader::read_as_object(io::istream& stream) -> std::optional<object>
{
    std::array<ubyte, 5> buf {};
    stream.read_to<ubyte>(buf);
    if (MAGIC != buf) { return std::nullopt; }

    read_string_pool(stream);

    auto const type {stream.read<bsbd::marker_type>()};
    if (type != bsbd::marker_type::SectionStart) { return std::nullopt; }

    auto retValue {read_section(stream)};
    return stream.is_eof()
        ? retValue
        : std::nullopt;
}

auto bsbd_reader::read_as_array(io::istream& stream) -> std::optional<array>
{
    std::array<ubyte, 5> buf {};
    stream.read_to<ubyte>(buf);
    if (MAGIC != buf) { return std::nullopt; }

    read_string_pool(stream);

    auto const type {stream.read<bsbd::marker_type>()};
    if (type != bsbd::marker_type::ArrayStart) { return std::nullopt; }

    auto retValue {read_array(stream)};
    return stream.is_eof()
        ? retValue
        : std::nullopt;
}

auto bsbd_reader::read_section(io::istream& stream) const -> std::optional<object>
{
    object retValue {};

    for (;;) {
        if (stream.is_eof()) { return std::nullopt; }
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::SectionEnd) { break; }
        if (!read_section_entry(stream, type, retValue)) { return std::nullopt; }
    }

    return retValue;
}

auto bsbd_reader::read_section_entry(io::istream& stream, bsbd::marker_type type, object& obj) const -> bool
{
    auto const keyIdx {stream.read<pool_size>()};
    if (keyIdx >= _stringPool.size()) { return false; }
    auto const& key {_stringPool[keyIdx]};

    if (u8 val {static_cast<u8>(type)}; val >= LitIntVal) {
        obj.set_entry(key, val - LitIntVal);
        return true;
    }

    switch (type) {
    case bsbd::marker_type::Int8:       obj.set_entry(key, stream.read<i8>()); break;
    case bsbd::marker_type::Int16:      obj.set_entry(key, stream.read<i16>()); break;
    case bsbd::marker_type::Int32:      obj.set_entry(key, stream.read<i32>()); break;
    case bsbd::marker_type::UInt8:      obj.set_entry(key, stream.read<u8>()); break;
    case bsbd::marker_type::UInt16:     obj.set_entry(key, stream.read<u16>()); break;
    case bsbd::marker_type::UInt32:     obj.set_entry(key, stream.read<u32>()); break;
    case bsbd::marker_type::Int64:      obj.set_entry(key, stream.read<i64>()); break;
    case bsbd::marker_type::Float32:    obj.set_entry(key, stream.read<f32>()); break;
    case bsbd::marker_type::Float64:    obj.set_entry(key, stream.read<f64>()); break;
    case bsbd::marker_type::BoolTrue:   obj.set_entry(key, true); break;
    case bsbd::marker_type::BoolFalse:  obj.set_entry(key, false); break;

    case bsbd::marker_type::LongString: {
        obj.set_entry(key, stream.read_string(static_cast<std::streamsize>(stream.read<u64>())));
    } break;

    case bsbd::marker_type::ShortString: {
        obj.set_entry(key, stream.read_string(stream.read<u8>()));
    } break;

    case bsbd::marker_type::SectionStart: {
        if (auto subSec {read_section(stream)}) {
            obj.set_entry(key, *subSec);
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        if (auto subArr {read_array(stream)}) {
            obj[key] = *subArr;
        } else {
            return false;
        }
    } break;

    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
    case bsbd::marker_type::LitInt:
    case bsbd::marker_type::StringPool:
        return false;
    }
    return true;
}

auto bsbd_reader::read_array(io::istream& stream) const -> std::optional<array>
{
    array retValue {};

    for (;;) {
        if (stream.is_eof()) { return std::nullopt; }
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::ArrayEnd) { break; }
        if (!read_array_entry(stream, type, retValue)) { return std::nullopt; }
    }

    return retValue;
}

auto bsbd_reader::read_array_entry(io::istream& stream, bsbd::marker_type type, array& arr) const -> bool
{
    if (u8 val {static_cast<u8>(type)}; val >= LitIntVal) {
        arr.add_entry(val - LitIntVal);
        return true;
    }

    switch (type) {
    case bsbd::marker_type::Int8:         arr.add_entry(stream.read<i8>()); break;
    case bsbd::marker_type::Int16:        arr.add_entry(stream.read<i16>()); break;
    case bsbd::marker_type::Int32:        arr.add_entry(stream.read<i32>()); break;
    case bsbd::marker_type::UInt8:        arr.add_entry(stream.read<u8>()); break;
    case bsbd::marker_type::UInt16:       arr.add_entry(stream.read<u16>()); break;
    case bsbd::marker_type::UInt32:       arr.add_entry(stream.read<u32>()); break;
    case bsbd::marker_type::Int64:        arr.add_entry(stream.read<i64>()); break;
    case bsbd::marker_type::Float32:      arr.add_entry(stream.read<f32>()); break;
    case bsbd::marker_type::Float64:      arr.add_entry(stream.read<f64>()); break;
    case bsbd::marker_type::BoolTrue:     arr.add_entry(true); break;
    case bsbd::marker_type::BoolFalse:    arr.add_entry(false); break;

    case bsbd::marker_type::LongString:   arr.add_entry(stream.read_string(static_cast<std::streamsize>(stream.read<u64>()))); break;
    case bsbd::marker_type::ShortString:  arr.add_entry(stream.read_string(stream.read<u8>())); break;

    case bsbd::marker_type::SectionStart: {
        if (auto subSec {read_section(stream)}) {
            arr.add_entry(*subSec);
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        if (auto subArr {read_array(stream)}) {
            arr.add_entry(*subArr);
        } else {
            return false;
        }
    } break;

    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
    case bsbd::marker_type::LitInt:
    case bsbd::marker_type::StringPool:
        return false;
    }
    return true;
}

void bsbd_reader::read_string_pool(io::istream& stream)
{
    auto const marker {stream.read<bsbd::marker_type>()};
    if (marker != bsbd::marker_type::StringPool) {
        stream.seek(-1, io::seek_dir::Current);
        return;
    }
    auto const poolSize {stream.read<pool_size>()};
    _stringPool.resize(poolSize);
    for (pool_size i {0}; i < poolSize; ++i) {
        auto const len {stream.read<pool_element_size>()};
        _stringPool[i] = stream.read_string(len);
    }
}

//////////////////////////////////////////////////////////////////////

template <typename T>
auto static is_within_limits(i64 value) -> bool
{
    return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
}

auto static fit_int(i64 value) -> bsbd::marker_type
{
    if (value >= 0 && value <= std::numeric_limits<u8>::max() - LitIntVal) { return static_cast<bsbd::marker_type>(value + LitIntVal); }

    if (is_within_limits<i8>(value)) { return bsbd::marker_type::Int8; }
    if (is_within_limits<u8>(value)) { return bsbd::marker_type::UInt8; }
    if (is_within_limits<i16>(value)) { return bsbd::marker_type::Int16; }
    if (is_within_limits<u16>(value)) { return bsbd::marker_type::UInt16; }
    if (is_within_limits<i32>(value)) { return bsbd::marker_type::Int32; }
    if (is_within_limits<u32>(value)) { return bsbd::marker_type::UInt32; }

    return bsbd::marker_type::Int64;
}

auto static fit_float(f64 value) -> bsbd::marker_type
{
    return std::fabs(static_cast<f32>(value) - value) > std::numeric_limits<f32>::epsilon() ? bsbd::marker_type::Float64 : bsbd::marker_type::Float32;
}

auto bsbd_writer::write(io::ostream& stream, object const& obj) -> bool
{
    stream.write(MAGIC);
    collect_strings(obj);
    return write_string_pool(stream) && write_section(stream, obj, "");
}

auto bsbd_writer::write(io::ostream& stream, array const& arr) -> bool
{
    stream.write(MAGIC);
    collect_strings(arr);
    return write_string_pool(stream) && write_array(stream, arr, "");
}

auto bsbd_writer::write_section(io::ostream& stream, object const& obj, utf8_string const& name) -> bool
{
    if (write_key(stream, bsbd::marker_type::SectionStart, name)
        && std::ranges::all_of(obj, [&](auto const& pair) { return write_entry(stream, pair.second, pair.first); })) {
        stream.write(bsbd::marker_type::SectionEnd);
        return true;
    }

    return false;
}

auto bsbd_writer::write_array(io::ostream& stream, array const& arr, utf8_string const& name) -> bool
{
    if (write_key(stream, bsbd::marker_type::ArrayStart, name)
        && std::ranges::all_of(arr, [&](auto const& v) { return write_entry(stream, v, ""); })) {
        stream.write(bsbd::marker_type::ArrayEnd);
        return true;
    }

    return false;
}

auto bsbd_writer::write_entry(io::ostream& stream, entry const& ent, utf8_string const& name) -> bool
{
    if (ent.is<bool>()) {
        if (!write_key(stream, ent.as<bool>() ? bsbd::marker_type::BoolTrue : bsbd::marker_type::BoolFalse, name)) { return false; }
    } else if (ent.is<i64>()) {
        i64 const  val {ent.as<i64>()};
        auto const type {fit_int(val)};
        if (!write_key(stream, type, name)) { return false; }
        switch (type) {
        case bsbd::marker_type::Int8:   stream.write(static_cast<i8>(val)); break;
        case bsbd::marker_type::Int16:  stream.write(static_cast<i16>(val)); break;
        case bsbd::marker_type::Int32:  stream.write(static_cast<i32>(val)); break;
        case bsbd::marker_type::UInt8:  stream.write(static_cast<u8>(val)); break;
        case bsbd::marker_type::UInt16: stream.write(static_cast<u16>(val)); break;
        case bsbd::marker_type::UInt32: stream.write(static_cast<u32>(val)); break;
        case bsbd::marker_type::Int64:  stream.write(val); break;
        default:                        break;
        }
    } else if (ent.is<f64>()) {
        f64 const  val {ent.as<f64>()};
        auto const type {fit_float(val)};
        if (!write_key(stream, type, name)) { return false; }
        switch (type) {
        case bsbd::marker_type::Float32: stream.write(static_cast<f32>(val)); break;
        case bsbd::marker_type::Float64: stream.write(val); break;
        default:                         break;
        }
    } else if (ent.is<utf8_string>()) {
        auto const str {ent.as<utf8_string>()};
        if (str.size() <= std::numeric_limits<u8>::max()) {
            if (!write_key(stream, bsbd::marker_type::ShortString, name)) { return false; }
            stream.write(static_cast<u8>(str.size()));
        } else {
            if (!write_key(stream, bsbd::marker_type::LongString, name)) { return false; }
            stream.write(static_cast<u64>(str.size()));
        }
        stream.write(str);
    } else if (ent.is<array>()) {
        return write_array(stream, ent.as<array>(), name);
    } else if (ent.is<object>()) {
        return write_section(stream, ent.as<object>(), name);
    }

    return true;
}

auto bsbd_writer::write_key(io::ostream& stream, bsbd::marker_type type, utf8_string const& name) -> bool
{
    stream.write(type);
    if (!name.empty()) {
        stream.write<pool_size>(_stringPool.at(name));
    }

    return true;
}

void bsbd_writer::collect_strings(object const& obj)
{
    for (auto const& [k, v] : obj) {
        if (!_stringPool.contains(k)) {
            auto const nextIdx {static_cast<pool_size>(_stringPool.size())};
            _stringPool[k]      = nextIdx;
            _stringIdx[nextIdx] = k;
        }
        if (v.is<object>()) {
            collect_strings(v.as<object>());
        } else if (v.is<array>()) {
            collect_strings(v.as<array>());
        }
    }
}

void bsbd_writer::collect_strings(array const& arr)
{
    for (auto const& v : arr) {
        if (v.is<object>()) {
            collect_strings(v.as<object>());
        } else if (v.is<array>()) {
            collect_strings(v.as<array>());
        }
    }
}

auto bsbd_writer::write_string_pool(io::ostream& stream) const -> bool
{
    if (_stringIdx.empty()) { return true; }

    stream.write(bsbd::marker_type::StringPool);
    stream.write<pool_size>(static_cast<pool_size>(_stringIdx.size()));
    for (auto const& s : _stringIdx) {
        if (s.second.size() > std::numeric_limits<pool_element_size>::max()) { return false; }

        stream.write<pool_element_size>(static_cast<pool_element_size>(s.second.size()));
        stream.write(s.second);
    }

    return true;
}
}
