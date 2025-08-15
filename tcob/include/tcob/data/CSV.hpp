// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Grid.hpp"

namespace tcob::data {
////////////////////////////////////////////////////////////

class TCOB_API csv_table final {
public:
    struct settings {
        bool HasHeader {true};
        char Separator {','};
        char Quote {'"'};
    };

    std::vector<string> Header;
    grid<string>        Rows;

    auto load(path const& file, settings s) noexcept -> bool;
    auto load(io::istream& in, settings s) noexcept -> bool;
    auto parse(string const& csv, settings s) -> bool;

    auto save(path const& file, settings s) const -> bool;
    auto save(io::ostream& out, settings s) const -> bool;
};

}
