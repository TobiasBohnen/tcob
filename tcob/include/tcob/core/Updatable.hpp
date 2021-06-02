// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob {
class Updatable {
public:
    virtual ~Updatable() = default;

    virtual void update(f64 deltaTime) = 0;
};
}