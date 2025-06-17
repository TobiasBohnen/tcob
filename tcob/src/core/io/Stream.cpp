// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/Stream.hpp>

#include <ios>

namespace tcob::io {

auto istream::read_string(std::streamsize length) -> string
{
    if (length <= 0) { return ""; }

    string retValue;
    retValue.resize(static_cast<usize>(length));
    retValue.resize(static_cast<usize>(read_bytes(retValue.data(), length)));
    return retValue;
}

auto istream::read_string_until(char delim) -> string
{
    string retValue;
    while (!is_eof()) {
        char const c {read<char>()};
        if (c == delim) { break; }
        retValue += c;
    }

    return retValue;
}

istream::operator bool() const
{
    return is_valid();
}

auto istream::is_valid() const -> bool
{
    return true;
}

////////////////////////////////////////////////////////////

auto ostream::write_string(string_view s) -> std::streamsize
{
    return write_bytes(s.data(), static_cast<std::streamsize>(s.size()));
}

} // namespace io
