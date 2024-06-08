// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ConfigParser_bsbd.hpp"

namespace tcob::data::config::detail {

constexpr std::array<ubyte, 5> MAGIC {'B', 'S', 'B', 'D', 1};

auto bsbd_reader::read_as_object(istream& stream) -> std::optional<object>
{
    std::array<ubyte, 5> buf {};
    stream.read_to<ubyte>(buf);
    if (MAGIC != buf) { return std::nullopt; }

    object retValue {};
    for (;;) {
        auto const type {stream.read<bsbd::marker_type>()};
        if (type != bsbd::marker_type::SectionStart) {
            break;
        }

        auto const name {stream.read_string_until('\0')};
        if (auto obj {read_section(stream)}) {
            if (name.empty()) {
                retValue = *obj;
            } else {
                retValue[name] = *obj;
            }
        } else {
            return std::nullopt;
        }
    }

    return stream.is_eof()
        ? std::optional {retValue}
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

auto bsbd_reader::read_section(istream& stream) -> std::optional<object>
{
    object obj {};

    for (;;) {
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::SectionEnd) {
            break;
        }

        if (!read_section_entry(stream, type, obj)) { return std::nullopt; }
        if (stream.is_eof()) { return std::nullopt; }
    }

    return obj;
}

auto bsbd_reader::read_section_entry(istream& stream, bsbd::marker_type type, object& obj) -> bool
{
    auto const name {stream.read_string_until('\0')};
    switch (type) {
    case bsbd::marker_type::String:
        obj[name] = stream.read_string_until('\0');
        break;
    case bsbd::marker_type::Int8:
        obj[name] = stream.read<i8>();
        break;
    case bsbd::marker_type::Int16:
        obj[name] = stream.read<i16>();
        break;
    case bsbd::marker_type::Int32:
        obj[name] = stream.read<i32>();
        break;
    case bsbd::marker_type::UInt8:
        obj[name] = stream.read<u8>();
        break;
    case bsbd::marker_type::UInt16:
        obj[name] = stream.read<u16>();
        break;
    case bsbd::marker_type::UInt32:
        obj[name] = stream.read<u32>();
        break;
    case bsbd::marker_type::Int64:
        obj[name] = stream.read<i64>();
        break;
    case bsbd::marker_type::Float32:
        obj[name] = stream.read<f32>();
        break;
    case bsbd::marker_type::Float64:
        obj[name] = stream.read<f64>();
        break;
    case bsbd::marker_type::BoolTrue:
        obj[name] = true;
        break;
    case bsbd::marker_type::BoolFalse:
        obj[name] = false;
        break;
    case bsbd::marker_type::SectionStart: {
        auto subSec {read_section(stream)};
        if (subSec) {
            obj[name] = *subSec;
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        auto subArr {read_array(stream)};
        if (subArr) {
            obj[name] = *subArr;
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
        return false;
    }
    return true;
}

auto bsbd_reader::read_array(istream& stream) -> std::optional<array>
{
    array arr {};

    for (;;) {
        auto const type {stream.read<bsbd::marker_type>()};
        if (type == bsbd::marker_type::ArrayEnd) {
            break;
        }

        if (!read_array_entry(stream, type, arr)) { return std::nullopt; }
        if (stream.is_eof()) { return std::nullopt; }
    }

    return arr;
}

auto bsbd_reader::read_array_entry(istream& stream, bsbd::marker_type type, array& arr) -> bool
{
    switch (type) {
    case bsbd::marker_type::String:
        arr.add(stream.read_string_until('\0'));
        break;
    case bsbd::marker_type::Int8:
        arr.add(stream.read<i8>());
        break;
    case bsbd::marker_type::Int16:
        arr.add(stream.read<i16>());
        break;
    case bsbd::marker_type::Int32:
        arr.add(stream.read<i32>());
        break;
    case bsbd::marker_type::UInt8:
        arr.add(stream.read<u8>());
        break;
    case bsbd::marker_type::UInt16:
        arr.add(stream.read<u16>());
        break;
    case bsbd::marker_type::UInt32:
        arr.add(stream.read<u32>());
        break;
    case bsbd::marker_type::Int64:
        arr.add(stream.read<i64>());
        break;
    case bsbd::marker_type::Float32:
        arr.add(stream.read<f32>());
        break;
    case bsbd::marker_type::Float64:
        arr.add(stream.read<f64>());
        break;
    case bsbd::marker_type::BoolTrue:
        arr.add(true);
        break;
    case bsbd::marker_type::BoolFalse:
        arr.add(false);
        break;
    case bsbd::marker_type::SectionStart: {
        auto subSec {read_section(stream)};
        if (subSec) {
            arr.add(*subSec);
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayStart: {
        auto subArr {read_array(stream)};
        if (subArr) {
            arr.add(*subArr);
        } else {
            return false;
        }
    } break;
    case bsbd::marker_type::ArrayEnd:
    case bsbd::marker_type::SectionEnd:
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

auto static fit_int(i64 value) -> bsbd::marker_type
{
    if (value >= std::numeric_limits<i8>::min() && value <= std::numeric_limits<i8>::max()) {
        return bsbd::marker_type::Int8;
    }
    if (value >= std::numeric_limits<u8>::min() && value <= std::numeric_limits<u8>::max()) {
        return bsbd::marker_type::UInt8;
    }
    if (value >= std::numeric_limits<i16>::min() && value <= std::numeric_limits<i16>::max()) {
        return bsbd::marker_type::Int16;
    }
    if (value >= std::numeric_limits<u16>::min() && value <= std::numeric_limits<u16>::max()) {
        return bsbd::marker_type::UInt16;
    }
    if (value >= std::numeric_limits<i32>::min() && value <= std::numeric_limits<i32>::max()) {
        return bsbd::marker_type::Int32;
    }
    if (value >= std::numeric_limits<u32>::min() && value <= std::numeric_limits<u32>::max()) {
        return bsbd::marker_type::UInt32;
    }

    return bsbd::marker_type::Int64;
}

auto static fit_float(f64 value) -> bsbd::marker_type
{
    return std::fabs(static_cast<f32>(value) - value) > std::numeric_limits<f32>::epsilon() ? bsbd::marker_type::Float64 : bsbd::marker_type::Float32;
}

auto bsbd_writer::write(ostream& stream, object const& obj) -> bool
{
    stream.write(MAGIC);

    stream.write(bsbd::marker_type::SectionStart);
    stream.write('\0');

    for (auto const& [k, v] : obj) {
        write_entry(stream, v, k);
    }

    stream.write(bsbd::marker_type::SectionEnd);

    return true;
}

auto bsbd_writer::write(ostream& stream, array const& arr) -> bool
{
    stream.write(MAGIC);

    stream.write(bsbd::marker_type::ArrayStart);
    stream.write('\0');

    for (auto const& v : arr) {
        write_entry(stream, v, "");
    }

    stream.write(bsbd::marker_type::ArrayEnd);

    return true;
}

void bsbd_writer::write_section(ostream& stream, object const& obj, utf8_string const& name) const
{
    write_entry_header(stream, bsbd::marker_type::SectionStart, name);

    for (auto const& [k, v] : obj) {
        // TODO: comments
        write_entry(stream, v, k);
    }

    stream.write(bsbd::marker_type::SectionEnd);
}

void bsbd_writer::write_array(ostream& stream, array const& arr, utf8_string const& name) const
{
    write_entry_header(stream, bsbd::marker_type::ArrayStart, name);

    for (auto const& v : arr) {
        write_entry(stream, v, "");
    }

    stream.write(bsbd::marker_type::ArrayEnd);
}

void bsbd_writer::write_entry(ostream& stream, entry const& ent, utf8_string const& name) const
{
    if (ent.is<bool>()) {
        write_entry_header(stream, ent.as<bool>() ? bsbd::marker_type::BoolTrue : bsbd::marker_type::BoolFalse, name);
    } else if (ent.is<i64>()) {
        i64 const  val {ent.as<i64>()};
        auto const type {fit_int(val)};
        write_entry_header(stream, type, name);
        switch (type) {
        case bsbd::marker_type::Int8:
            stream.write(static_cast<i8>(val));
            break;
        case bsbd::marker_type::Int16:
            stream.write(static_cast<i16>(val));
            break;
        case bsbd::marker_type::Int32:
            stream.write(static_cast<i32>(val));
            break;
        case bsbd::marker_type::UInt8:
            stream.write(static_cast<u8>(val));
            break;
        case bsbd::marker_type::UInt16:
            stream.write(static_cast<u16>(val));
            break;
        case bsbd::marker_type::UInt32:
            stream.write(static_cast<u32>(val));
            break;
        case bsbd::marker_type::Int64:
            stream.write(val);
            break;
        default: break;
        }
    } else if (ent.is<f64>()) {
        f64 const  val {ent.as<f64>()};
        auto const type {fit_float(val)};
        write_entry_header(stream, type, name);
        switch (type) {
        case bsbd::marker_type::Float32:
            stream.write(static_cast<f32>(val));
            break;
        case bsbd::marker_type::Float64:
            stream.write(val);
            break;
        default: break;
        }
    } else if (ent.is<utf8_string>()) {
        write_entry_header(stream, bsbd::marker_type::String, name);
        stream.write(ent.as<utf8_string>());
        stream.write('\0');
    } else if (ent.is<array>()) {
        write_array(stream, ent.as<array>(), name);
    } else if (ent.is<object>()) {
        write_section(stream, ent.as<object>(), name);
    }
}

void bsbd_writer::write_entry_header(ostream& stream, bsbd::marker_type type, utf8_string const& name) const
{
    stream.write(type);
    if (!name.empty()) {
        stream.write(name);
        stream.write('\0');
    }
}

}
