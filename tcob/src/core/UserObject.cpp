// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/UserObject.hpp"

#include <typeindex>

namespace tcob {
////////////////////////////////////////////////////////////

auto user_object::has_value() const -> bool
{
    return _data.index() != 0;
}

auto user_object::type() const -> std::type_index
{
    return _type;
}

void user_object::reset()
{
    _data = {};
    _type = typeid(void);
}

}
