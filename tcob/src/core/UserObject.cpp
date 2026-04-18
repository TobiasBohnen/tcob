// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/UserObject.hpp"

#include <memory>
#include <typeindex>

namespace tcob {
////////////////////////////////////////////////////////////

auto user_object::has_value() const -> bool
{
    return _data != nullptr;
}

auto user_object::type() const -> std::type_index
{
    return _type;
}

void user_object::reset()
{
    _data.reset();
    _type = typeid(void);
}

}
