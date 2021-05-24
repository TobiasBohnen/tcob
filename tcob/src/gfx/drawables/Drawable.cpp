// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/drawables/Drawable.hpp>

namespace tcob {

auto Drawable::is_visible() const -> bool
{
    return _visible;
}

void Drawable::show()
{
    _visible = true;
}

void Drawable::hide()
{
    _visible = false;
}
}