// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include "tcob/tcob_config.hpp"

namespace tcob::io::magic {

////////////////////////////////////////////////////////////

struct signature {
    struct part {
        i8                 Offset;
        std::vector<ubyte> Bytes;
    };

    std::vector<part> Parts;
    path              Extension;
    string            Group;
};

////////////////////////////////////////////////////////////

TCOB_API void add_signature(signature const& sig);

TCOB_API auto get_extension(io::istream& stream) -> path;
TCOB_API auto get_signature(io::istream& stream) -> std::optional<signature>;
TCOB_API auto get_group(path const& ext) -> path;
}
