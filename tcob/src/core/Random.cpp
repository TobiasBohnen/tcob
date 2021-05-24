// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/Random.hpp>

#include <algorithm>
#include <chrono>

namespace tcob {
Random::Random()
    : _gen {}
{
}

Random::Random(u64 seed)
    : _gen { seed }
{
}

////////////////////////////////////////////////////////////
Xoroshiro128Plus::Xoroshiro128Plus()
{
    _state[0] = static_cast<u64>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

Xoroshiro128Plus::Xoroshiro128Plus(u64 seed)
{
    _state[0] = seed;
}

}