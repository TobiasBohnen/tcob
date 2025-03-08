// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/Magic.hpp"

#include <algorithm>
#include <ios>
#include <optional>
#include <span>
#include <vector>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io::magic {

constexpr u8 BUFFER_LENGTH {128};

auto static GetSignatures() -> std::vector<signature>&
{
    static std::vector<signature> signatures {};
    return signatures;
}

void add_signature(signature const& sig)
{
    GetSignatures().push_back(sig);
}

auto get_signature(istream& stream) -> std::optional<signature>
{
    auto const offset {stream.tell()};
    auto const startBuffer {stream.read_n<ubyte>(BUFFER_LENGTH)};
    stream.seek(-std::min(stream.size_in_bytes(), static_cast<std::streamsize>(BUFFER_LENGTH)), seek_dir::End);
    auto const endBuffer {stream.read_n<ubyte>(BUFFER_LENGTH)};
    stream.seek(offset, seek_dir::Begin);

    for (auto const& sig : GetSignatures()) {
        bool found {true};
        for (auto const& part : sig.Parts) {
            std::span<ubyte const> slice;

            if (part.Offset >= 0) {
                usize const startIndex {static_cast<usize>(part.Offset)};
                usize const endIndex {startIndex + part.Bytes.size()};

                if (startIndex < startBuffer.size() && endIndex <= startBuffer.size()) {
                    slice = {startBuffer.data() + startIndex, part.Bytes.size()};
                }
            } else {
                usize const startIndex {static_cast<usize>(endBuffer.size() + part.Offset)};
                usize const endIndex {startIndex + part.Bytes.size()};

                if (startIndex < endBuffer.size() && endIndex <= endBuffer.size()) {
                    slice = {endBuffer.data() + startIndex, part.Bytes.size()};
                }
            }

            // Compare the slice with the signature part
            if (!std::equal(slice.begin(), slice.end(), part.Bytes.begin())) {
                found = false;
                break;
            }
        }

        if (found) { return sig; }
    }

    return std::nullopt;
}

auto get_group(path const& ext) -> path
{
    for (auto const& sig : GetSignatures()) {
        if (sig.Extension == ext) { return sig.Group; }
    }

    return "";
}

auto get_extension(istream& stream) -> path
{
    auto const sig {get_signature(stream)};
    return sig ? sig->Extension : "";
}

} // namespace magic
