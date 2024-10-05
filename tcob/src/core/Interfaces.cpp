// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Interfaces.hpp"

namespace tcob {

void updatable::update(milliseconds deltaTime)
{
    on_update(deltaTime);
}

}
