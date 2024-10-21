// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/random/Random.hpp"

namespace tcob {

auto GetRandomID() -> uid
{
    static rng rand {0x1badbad1};
    return rand(uid {0}, std::numeric_limits<uid>::max() - 0xff);
}

}
