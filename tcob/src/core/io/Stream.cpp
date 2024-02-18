// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/Stream.hpp>

namespace tcob::io {

auto istream::read_string(std::streamsize length) -> string
{
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
        if (c == delim) {
            break;
        }

        retValue += c;
    }

    return retValue;
}

////////////////////////////////////////////////////////////

auto ostream::write_string(string_view s) -> std::streamsize
{
    return write_bytes(s.data(), static_cast<std::streamsize>(s.size()));
}

} // namespace io
