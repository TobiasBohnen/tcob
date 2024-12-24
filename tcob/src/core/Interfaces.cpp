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

void updatable::on_update(milliseconds /* deltaTime */)
{
}

////////////////////////////////////////////////////////////

void hybrid_updatable::fixed_update(milliseconds deltaTime)
{
    on_fixed_update(deltaTime);
}

void hybrid_updatable::on_fixed_update(milliseconds /* deltaTime */)
{
}

}
