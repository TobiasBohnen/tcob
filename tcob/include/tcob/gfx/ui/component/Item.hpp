// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>

#include "tcob/gfx/ui/component/Icon.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API item {
public:
    utf8_string Text;
    icon        Icon {};
    std::any    UserData {};

    auto operator==(item const& other) const -> bool
    {
        return Text == other.Text && Icon == other.Icon; // FIXME
    }
};

}
