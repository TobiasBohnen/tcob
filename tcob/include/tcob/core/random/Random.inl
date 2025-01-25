// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Random.hpp"

namespace tcob::random {

////////////////////////////////////////////////////////////

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(seed_type seed, auto&&... distArgs)
    : _distribution {distArgs...}
{
    _engine.seed(_state, seed);
}

template <RandomEngine E, typename D>
inline random_number_generator<E, D>::random_number_generator(state_type state, auto&&... distArgs)
    : _state {state}
    , _distribution {distArgs...}
{
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::operator()(auto&&... distArgs)
{
    return _distribution(*this, distArgs...);
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::next() -> result_type
{
    return _engine(_state);
}

template <RandomEngine E, typename D>
inline auto random_number_generator<E, D>::state() const -> state_type const&
{
    return _state;
}

////////////////////////////////////////////////////////////

template <i32 N, RandomEngine E>
inline dice<N, E>::dice(seed_type seed)
    : _random {seed}
{
}

template <i32 N, RandomEngine E>
inline dice<N, E>::dice(state_type state)
    : _random {state}
{
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::roll() -> i32
{
    return _random(1, N);
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::roll_n(usize n) -> std::vector<i32>
{
    std::vector<i32> retValue(n);
    std::generate_n(retValue.begin(), n, [this]() { return roll(); });
    return retValue;
}

template <i32 N, RandomEngine E>
inline auto dice<N, E>::state() const -> state_type const&
{
    return _random.state();
}

////////////////////////////////////////////////////////////

template <typename T, RandomEngine E>
inline shuffle<T, E>::shuffle(seed_type seed)
    : _random {seed}
{
}

template <typename T, RandomEngine E>
inline shuffle<T, E>::shuffle(state_type state)
    : _random {state}
{
}

template <typename T, RandomEngine E>
inline auto shuffle<T, E>::state() const -> state_type const&
{
    return _random.state();
}

template <typename T, RandomEngine E>
inline void shuffle<T, E>::operator()(std::span<T> span)
{
    if (span.size() <= 1) { return; }

    for (usize i {span.size() - 1}; i > 0; --i) {
        usize const j {_random(usize {0}, i)};
        using std::swap;
        swap(span[i], span[j]);
    }
}
}
