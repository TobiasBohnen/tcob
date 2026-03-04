// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Shape.hpp"

#include <memory>

namespace tcob::gfx {

template <DerivedFrom<shape> T>
inline auto shape_batch::create_shape() -> T&
{
    _isDirty = true;
    return static_cast<T&>(*_children.emplace_back(std::make_unique<T>()));
}

}
