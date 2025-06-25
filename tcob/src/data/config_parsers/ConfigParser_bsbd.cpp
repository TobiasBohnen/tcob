// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_bsbd.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <ios>
#include <limits>
#include <map>
#include <optional>

#include "tcob/core/io/Stream.hpp"
#include "tcob/data/ConfigConversions.hpp"
#include "tcob/data/ConfigTypes.hpp"

namespace tcob::data::detail {

constexpr std::array<byte, 5> MAGIC {'B', 'S', 'B', 'D', 1};
constexpr u8                  LitIntVal {static_cast<u8>(bsbd::marker_type::LitInt)};

using short_string_size = u8;
using long_string_size  = u32;

auto bsbd_reader::read_as_object(io::istream& stream) -> std::optional<object>
{
    std::array<byte, 5> buf {};
    stream.read_to<byte>(buf);
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
    std::array<byte, 5> buf {};
    stream.read_to<byte>(buf);
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
    // key
    u32 keyIdx {0};
    switch (_stringPoolSize) {
    case bsbd::marker_type::UInt8:  keyIdx = stream.read<u8>(); break;
    case bsbd::marker_type::UInt16: keyIdx = stream.read<u16, std::endian::little>(); break;
    case bsbd::marker_type::UInt32: keyIdx = stream.read<u32, std::endian::little>(); break;
    default:                        return false;
    }
    if (keyIdx >= _stringPool.size()) { return false; }
    auto const& key {_stringPool[keyIdx]};

    // value
    if (u8 val {static_cast<u8>(type)}; val >= LitIntVal) {
        obj.set_entry(key, entry {val - LitIntVal});
        return true;
    }

    entry entry;
    switch (type) {
    case bsbd::marker_type::Int8:         entry = stream.read<i8>(); break;
    case bsbd::marker_type::Int16:        entry = stream.read<i16, std::endian::little>(); break;
    case bsbd::marker_type::Int32:        entry = stream.read<i32, std::endian::little>(); break;
    case bsbd::marker_type::UInt8:        entry = stream.read<u8>(); break;
    case bsbd::marker_type::UInt16:       entry = stream.read<u16, std::endian::little>(); break;
    case bsbd::marker_type::UInt32:       entry = stream.read<u32, std::endian::little>(); break;
    case bsbd::marker_type::Int64:        entry = stream.read<i64, std::endian::little>(); break;
    case bsbd::marker_type::Float32:      entry = stream.read<f32, std::endian::little>(); break;
    case bsbd::marker_type::Float64:      entry = stream.read<f64, std::endian::little>(); break;
    case bsbd::marker_type::BoolTrue:     entry = true; break;
    case bsbd::marker_type::BoolFalse:    entry = false; break;

    case bsbd::marker_type::LongString:   entry = stream.read_string(stream.read<long_string_size, std::endian::little>()); break;
    case bsbd::marker_type::ShortString:  entry = stream.read_string(stream.read<short_string_size, std::endian::little>()); break;

    case bsbd::marker_type::SectionStart: {
        if (auto subSec {read_section(stream)}) {
            entry = *subSec;
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        if (auto subArr {read_array(stream)}) {
            entry = *subArr;
        } else {
            return false;
        }
    } break;

    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
    case bsbd::marker_type::LitInt:
        return false;
    }

    obj.set_entry(key, entry);
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
        arr.add_entry(entry {val - LitIntVal});
        return true;
    }

    entry entry;
    switch (type) {
    case bsbd::marker_type::Int8:         entry = stream.read<i8>(); break;
    case bsbd::marker_type::Int16:        entry = stream.read<i16, std::endian::little>(); break;
    case bsbd::marker_type::Int32:        entry = stream.read<i32, std::endian::little>(); break;
    case bsbd::marker_type::UInt8:        entry = stream.read<u8>(); break;
    case bsbd::marker_type::UInt16:       entry = stream.read<u16, std::endian::little>(); break;
    case bsbd::marker_type::UInt32:       entry = stream.read<u32, std::endian::little>(); break;
    case bsbd::marker_type::Int64:        entry = stream.read<i64, std::endian::little>(); break;
    case bsbd::marker_type::Float32:      entry = stream.read<f32, std::endian::little>(); break;
    case bsbd::marker_type::Float64:      entry = stream.read<f64, std::endian::little>(); break;
    case bsbd::marker_type::BoolTrue:     entry = true; break;
    case bsbd::marker_type::BoolFalse:    entry = false; break;

    case bsbd::marker_type::LongString:   entry = stream.read_string(stream.read<long_string_size, std::endian::little>()); break;
    case bsbd::marker_type::ShortString:  entry = stream.read_string(stream.read<short_string_size, std::endian::little>()); break;

    case bsbd::marker_type::SectionStart: {
        if (auto subSec {read_section(stream)}) {
            entry = *subSec;
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        if (auto subArr {read_array(stream)}) {
            entry = *subArr;
        } else {
            return false;
        }
    } break;

    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
    case bsbd::marker_type::LitInt:
        return false;
    }

    arr.add_entry(entry);
    return true;
}

auto bsbd_reader::read_string_pool(io::istream& stream) -> bool
{
    _stringPoolSize = stream.read<bsbd::marker_type>();

    u64 poolSize {0};
    switch (_stringPoolSize) {
    case bsbd::marker_type::UInt8:  poolSize = stream.read<u8>(); break;
    case bsbd::marker_type::UInt16: poolSize = stream.read<u16, std::endian::little>(); break;
    case bsbd::marker_type::UInt32: poolSize = stream.read<u32, std::endian::little>(); break;
    default:                        return false;
    }

    auto const element {stream.read<bsbd::marker_type>()};
    _stringPool.resize(poolSize);
    for (u64 i {0}; i < poolSize; ++i) {
        std::streamsize len {0};
        switch (element) {
        case bsbd::marker_type::UInt8:  len = stream.read<u8>(); break;
        case bsbd::marker_type::UInt16: len = stream.read<u16, std::endian::little>(); break;
        case bsbd::marker_type::UInt32: len = stream.read<u32, std::endian::little>(); break;
        default:                        return false;
        }
        _stringPool[i] = stream.read_string(len);
    }

    return true;
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
    if (!write_string_pool(stream)) { return false; }

    write_section(stream, obj, "");
    return true;
}

auto bsbd_writer::write(io::ostream& stream, array const& arr) -> bool
{
    stream.write(MAGIC);

    collect_strings(arr);
    if (!write_string_pool(stream)) { return false; }

    write_array(stream, arr, "");
    return true;
}

void bsbd_writer::write_section(io::ostream& stream, object const& obj, utf8_string_view name)
{
    write_key(stream, bsbd::marker_type::SectionStart, name);
    for (auto const& pair : obj) { write_entry(stream, pair.second, pair.first); }
    stream.write(bsbd::marker_type::SectionEnd);
}

void bsbd_writer::write_array(io::ostream& stream, array const& arr, utf8_string_view name)
{
    write_key(stream, bsbd::marker_type::ArrayStart, name);
    for (auto const& v : arr) { write_entry(stream, v, ""); }
    stream.write(bsbd::marker_type::ArrayEnd);
}

void bsbd_writer::write_entry(io::ostream& stream, entry const& ent, utf8_string_view name)
{
    if (ent.is<bool>()) {
        write_key(stream, ent.as<bool>() ? bsbd::marker_type::BoolTrue : bsbd::marker_type::BoolFalse, name);
    } else if (ent.is<i64>()) {
        i64 const  val {ent.as<i64>()};
        auto const type {fit_int(val)};
        write_key(stream, type, name);
        switch (type) {
        case bsbd::marker_type::Int8:   stream.write(static_cast<i8>(val)); break;
        case bsbd::marker_type::Int16:  stream.write<i16, std::endian::little>(static_cast<i16>(val)); break;
        case bsbd::marker_type::Int32:  stream.write<i32, std::endian::little>(static_cast<i32>(val)); break;
        case bsbd::marker_type::UInt8:  stream.write(static_cast<u8>(val)); break;
        case bsbd::marker_type::UInt16: stream.write<u16, std::endian::little>(static_cast<u16>(val)); break;
        case bsbd::marker_type::UInt32: stream.write<u32, std::endian::little>(static_cast<u32>(val)); break;
        case bsbd::marker_type::Int64:  stream.write<i64, std::endian::little>(val); break;
        default:                        break;
        }
    } else if (ent.is<f64>()) {
        f64 const  val {ent.as<f64>()};
        auto const type {fit_float(val)};
        write_key(stream, type, name);
        switch (type) {
        case bsbd::marker_type::Float32: stream.write<f32, std::endian::little>(static_cast<f32>(val)); break;
        case bsbd::marker_type::Float64: stream.write<f64, std::endian::little>(val); break;
        default:                         break;
        }
    } else if (ent.is<utf8_string>()) {
        auto const str {ent.as<utf8_string>()};
        if (str.size() <= std::numeric_limits<short_string_size>::max()) {
            write_key(stream, bsbd::marker_type::ShortString, name);
            stream.write<short_string_size, std::endian::little>(static_cast<short_string_size>(str.size()));
        } else {
            // TODO: size check
            write_key(stream, bsbd::marker_type::LongString, name);
            stream.write<long_string_size, std::endian::little>(static_cast<long_string_size>(str.size()));
        }
        stream.write(str);
    } else if (ent.is<array>()) {
        write_array(stream, ent.as<array>(), name);
    } else if (ent.is<object>()) {
        write_section(stream, ent.as<object>(), name);
    }
}

void bsbd_writer::write_key(io::ostream& stream, bsbd::marker_type type, utf8_string_view name)
{
    stream.write(type);
    if (!name.empty()) {
        auto const val {_stringPool.at(name)};
        switch (_stringPoolSize) {
        case bsbd::marker_type::UInt8:  stream.write(static_cast<u8>(val)); break;
        case bsbd::marker_type::UInt16: stream.write<u16, std::endian::little>(static_cast<u16>(val)); break;
        case bsbd::marker_type::UInt32: stream.write<u32, std::endian::little>(static_cast<u32>(val)); break;
        default:                        break;
        }
    }
}

void bsbd_writer::collect_strings(object const& obj)
{
    for (auto const& [k, v] : obj) {
        if (!_stringPool.contains(k)) {
            _stringPool[k] = _stringPool.size();
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

auto bsbd_writer::write_string_pool(io::ostream& stream) -> bool
{
    // pool size
    auto const poolSize {_stringPool.size()};
    if (poolSize <= std::numeric_limits<u8>::max()) {
        _stringPoolSize = bsbd::marker_type::UInt8;
        stream.write(_stringPoolSize);
        stream.write(static_cast<u8>(poolSize));
    } else if (poolSize <= std::numeric_limits<u16>::max()) {
        _stringPoolSize = bsbd::marker_type::UInt16;
        stream.write(_stringPoolSize);
        stream.write<u16, std::endian::little>(static_cast<u16>(poolSize));
    } else if (poolSize <= std::numeric_limits<u32>::max()) {
        _stringPoolSize = bsbd::marker_type::UInt32;
        stream.write(_stringPoolSize);
        stream.write<u32, std::endian::little>(static_cast<u32>(poolSize));
    } else {
        return false;
    }

    // pool elements
    usize                             maxSize {0};
    std::map<usize, utf8_string_view> stringIdx;
    for (auto const& s : _stringPool) {
        stringIdx[s.second] = s.first;
        maxSize             = std::max(maxSize, s.first.size());
    }

    bsbd::marker_type element {};
    if (maxSize <= std::numeric_limits<u8>::max()) {
        element = bsbd::marker_type::UInt8;
    } else if (maxSize <= std::numeric_limits<u16>::max()) {
        element = bsbd::marker_type::UInt16;
    } else if (maxSize <= std::numeric_limits<u32>::max()) {
        element = bsbd::marker_type::UInt32;
    } else {
        return false;
    }

    stream.write(element);

    for (auto const& s : stringIdx) {
        switch (element) {
        case bsbd::marker_type::UInt8:  stream.write(static_cast<u8>(s.second.size())); break;
        case bsbd::marker_type::UInt16: stream.write<u16, std::endian::little>(static_cast<u16>(s.second.size())); break;
        case bsbd::marker_type::UInt32: stream.write<u32, std::endian::little>(static_cast<u32>(s.second.size())); break;
        default:                        break;
        }
        stream.write(s.second);
    }

    return true;
}
}
