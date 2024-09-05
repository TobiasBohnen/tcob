// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Shape.hpp"

namespace tcob::gfx {

template <std::derived_from<shape> T>
inline auto shape_batch::create_shape() -> std::shared_ptr<T>
{
    return std::static_pointer_cast<T>(_children.emplace_back(std::make_shared<T>()));
}

}
