// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/UserObject.hpp"
#include "tcob/gfx/ui/component/Icon.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API item {
public:
    utf8_string Text;
    icon        Icon {};
    user_object UserData {};

    auto operator==(item const& other) const -> bool
    {
        return Text == other.Text && Icon == other.Icon; // FIXME
    }
};

}
