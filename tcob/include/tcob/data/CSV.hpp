// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Common.hpp"

namespace tcob::data::csv {
////////////////////////////////////////////////////////////

struct settings {
    bool HasHeader {true};
    char Separator {','};
    char Quote {'"'};
};

////////////////////////////////////////////////////////////

class TCOB_API table final {
public:
    std::vector<string>              Header;
    std::vector<std::vector<string>> Rows;

    auto load(path const& file, settings s = {}) noexcept -> load_status;
    auto load(io::istream& in, settings s = {}) noexcept -> load_status;
    auto parse(string const& csv, settings s = {}) -> bool;

    auto save(path const& file, settings s = {}) const -> bool;
    auto save(io::ostream& out, settings s = {}) const -> bool;
};

}
