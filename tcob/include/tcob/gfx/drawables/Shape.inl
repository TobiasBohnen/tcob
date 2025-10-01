// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Shape.hpp"

#include <memory>

namespace tcob::gfx {

template <std::derived_from<shape> T>
inline auto shape_batch::create_shape() -> T&
{
    return static_cast<T&>(*_children.emplace_back(std::make_unique<T>()));
}

}
