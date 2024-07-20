// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_bsbd.hpp"

#include <algorithm>

namespace tcob::data::config::detail {

constexpr std::array<ubyte, 5> MAGIC {'B', 'S', 'B', 'D', 1};
constexpr u8                   LitIntVal {static_cast<u8>(bsbd::marker_type::LitInt)};

auto bsbd_reader::read_as_object(istream& stream) -> std::optional<object>
{
    std::array<ubyte, 5> buf {};
    stream.read_to<ubyte>(buf);
    if (MAGIC != buf) { return std::nullopt; }

    auto const type {stream.read<bsbd::marker_type>()};
    if (type != bsbd::marker_type::SectionStart) { return std::nullopt; }

    auto retValue {read_section(stream)};
    return stream.is_eof()
        ? retValue
        : std::nullopt;
}

auto bsbd_reader::read_as_array(istream& stream) -> std::optional<array>
{
    std::array<ubyte, 5> buf {};
    stream.read_to<ubyte>(buf);
    if (MAGIC != buf) { return std::nullopt; }

    auto const type {stream.read<bsbd::marker_type>()};
    if (type != bsbd::marker_type::ArrayStart) { return std::nullopt; }

    auto retValue {read_array(stream)};
    return stream.is_eof()
        ? retValue
        : std::nullopt;
}

auto bsbd_reader::read_section(istream& stream) const -> std::optional<object>
{
    object obj {};

    for (;;) {
        if (stream.is_eof()) { return std::nullopt; }
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::SectionEnd) { break; }
        if (!read_section_entry(stream, type, obj)) { return std::nullopt; }
    }

    return obj;
}

auto bsbd_reader::read_section_entry(istream& stream, bsbd::marker_type type, object& obj) const -> bool
{
    auto const name {stream.read_string(stream.read<u8>())};

    if (u8 val {static_cast<u8>(type)}; val >= LitIntVal) {
        obj.set_entry(name, val - LitIntVal);
        return true;
    }

    switch (type) {
    case bsbd::marker_type::Int8: obj.set_entry(name, stream.read<i8>()); break;
    case bsbd::marker_type::Int16: obj.set_entry(name, stream.read<i16>()); break;
    case bsbd::marker_type::Int32: obj.set_entry(name, stream.read<i32>()); break;
    case bsbd::marker_type::UInt8: obj.set_entry(name, stream.read<u8>()); break;
    case bsbd::marker_type::UInt16: obj.set_entry(name, stream.read<u16>()); break;
    case bsbd::marker_type::UInt32: obj.set_entry(name, stream.read<u32>()); break;
    case bsbd::marker_type::Int64: obj.set_entry(name, stream.read<i64>()); break;
    case bsbd::marker_type::Float32: obj.set_entry(name, stream.read<f32>()); break;
    case bsbd::marker_type::Float64: obj.set_entry(name, stream.read<f64>()); break;
    case bsbd::marker_type::BoolTrue: obj.set_entry(name, true); break;
    case bsbd::marker_type::BoolFalse: obj.set_entry(name, false); break;

    case bsbd::marker_type::LongString: {
        obj.set_entry(name, stream.read_string(stream.read<u64>()));
    } break;

    case bsbd::marker_type::ShortString: {
        obj.set_entry(name, stream.read_string(stream.read<u8>()));
    } break;

    case bsbd::marker_type::SectionStart: {
        if (auto subSec {read_section(stream)}) {
            obj.set_entry(name, *subSec);
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        if (auto subArr {read_array(stream)}) {
            obj[name] = *subArr;
        } else {
            return false;
        }
    } break;

    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
    case bsbd::marker_type::LitInt:
        return false;
    }
    return true;
}

auto bsbd_reader::read_array(istream& stream) const -> std::optional<array>
{
    array arr {};

    for (;;) {
        if (stream.is_eof()) { return std::nullopt; }
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::ArrayEnd) { break; }
        if (!read_array_entry(stream, type, arr)) { return std::nullopt; }
    }

    return arr;
}

auto bsbd_reader::read_array_entry(istream& stream, bsbd::marker_type type, array& arr) const -> bool
{
    if (u8 val {static_cast<u8>(type)}; val >= LitIntVal) {
        arr.add_entry(val - LitIntVal);
        return true;
    }

    switch (type) {
    case bsbd::marker_type::Int8: arr.add_entry(stream.read<i8>()); break;
    case bsbd::marker_type::Int16: arr.add_entry(stream.read<i16>()); break;
    case bsbd::marker_type::Int32: arr.add_entry(stream.read<i32>()); break;
    case bsbd::marker_type::UInt8: arr.add_entry(stream.read<u8>()); break;
    case bsbd::marker_type::UInt16: arr.add_entry(stream.read<u16>()); break;
    case bsbd::marker_type::UInt32: arr.add_entry(stream.read<u32>()); break;
    case bsbd::marker_type::Int64: arr.add_entry(stream.read<i64>()); break;
    case bsbd::marker_type::Float32: arr.add_entry(stream.read<f32>()); break;
    case bsbd::marker_type::Float64: arr.add_entry(stream.read<f64>()); break;
    case bsbd::marker_type::BoolTrue: arr.add_entry(true); break;
    case bsbd::marker_type::BoolFalse: arr.add_entry(false); break;

    case bsbd::marker_type::LongString: arr.add_entry(stream.read_string(stream.read<u64>())); break;
    case bsbd::marker_type::ShortString: arr.add_entry(stream.read_string(stream.read<u8>())); break;

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
        return false;
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

auto bsbd_writer::write(ostream& stream, object const& obj) -> bool
{
    stream.write(MAGIC);
    return write_section(stream, obj, "");
}

auto bsbd_writer::write(ostream& stream, array const& arr) -> bool
{
    stream.write(MAGIC);
    return write_array(stream, arr, "");
}

auto bsbd_writer::write_section(ostream& stream, object const& obj, utf8_string const& name) const -> bool
{
    if (write_entry_header(stream, bsbd::marker_type::SectionStart, name)
        && std::ranges::all_of(obj, [&](auto const& pair) { return write_entry(stream, pair.second, pair.first); })) {
        stream.write(bsbd::marker_type::SectionEnd);
        return true;
    }

    return false;
}

auto bsbd_writer::write_array(ostream& stream, array const& arr, utf8_string const& name) const -> bool
{
    if (write_entry_header(stream, bsbd::marker_type::ArrayStart, name)
        && std::ranges::all_of(arr, [&](auto const& v) { return write_entry(stream, v, ""); })) {
        stream.write(bsbd::marker_type::ArrayEnd);
        return true;
    }

    return false;
}

auto bsbd_writer::write_entry(ostream& stream, entry const& ent, utf8_string const& name) const -> bool
{
    if (ent.is<bool>()) {
        if (!write_entry_header(stream, ent.as<bool>() ? bsbd::marker_type::BoolTrue : bsbd::marker_type::BoolFalse, name)) { return false; }
    } else if (ent.is<i64>()) {
        i64 const  val {ent.as<i64>()};
        auto const type {fit_int(val)};
        if (!write_entry_header(stream, type, name)) { return false; }
        switch (type) {
        case bsbd::marker_type::Int8: stream.write(static_cast<i8>(val)); break;
        case bsbd::marker_type::Int16: stream.write(static_cast<i16>(val)); break;
        case bsbd::marker_type::Int32: stream.write(static_cast<i32>(val)); break;
        case bsbd::marker_type::UInt8: stream.write(static_cast<u8>(val)); break;
        case bsbd::marker_type::UInt16: stream.write(static_cast<u16>(val)); break;
        case bsbd::marker_type::UInt32: stream.write(static_cast<u32>(val)); break;
        case bsbd::marker_type::Int64: stream.write(val); break;
        default: break;
        }
    } else if (ent.is<f64>()) {
        f64 const  val {ent.as<f64>()};
        auto const type {fit_float(val)};
        if (!write_entry_header(stream, type, name)) { return false; }
        switch (type) {
        case bsbd::marker_type::Float32: stream.write(static_cast<f32>(val)); break;
        case bsbd::marker_type::Float64: stream.write(val); break;
        default: break;
        }
    } else if (ent.is<utf8_string>()) {
        auto const str {ent.as<utf8_string>()};
        if (str.size() <= std::numeric_limits<u8>::max()) {
            if (!write_entry_header(stream, bsbd::marker_type::ShortString, name)) { return false; }
            stream.write(static_cast<u8>(str.size()));
        } else {
            if (!write_entry_header(stream, bsbd::marker_type::LongString, name)) { return false; }
            stream.write(static_cast<u64>(str.size()));
        }
        stream.write(str);
    } else if (ent.is<array>()) {
        write_array(stream, ent.as<array>(), name);
    } else if (ent.is<object>()) {
        write_section(stream, ent.as<object>(), name);
    }

    return true;
}

auto bsbd_writer::write_entry_header(ostream& stream, bsbd::marker_type type, utf8_string const& name) const -> bool
{
    stream.write(type);
    if (!name.empty()) {
        if (name.size() > std::numeric_limits<u8>::max()) { return false; }
        stream.write(static_cast<u8>(name.size()));
        stream.write(name);
    }

    return true;
}
}
