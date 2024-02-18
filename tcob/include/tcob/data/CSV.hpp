// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::data::csv {
////////////////////////////////////////////////////////////

struct settings {
    bool HasHeader {true};
    byte Separator {','};
    byte Quote {'"'};
};

////////////////////////////////////////////////////////////

class TCOB_API table final {
public:
    std::vector<string>              Header;
    std::vector<std::vector<string>> Rows;

    auto load(path const& file, settings s = {}) noexcept -> load_status;
    auto load(istream& in, settings s = {}) noexcept -> load_status;
    auto parse(string const& csv, settings s = {}) -> bool;

    auto save(path const& file, settings s = {}) const -> bool;
    auto save(ostream& out, settings s = {}) const -> bool;
};

}
