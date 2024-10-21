// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Signal.hpp"

namespace tcob {

template <typename EvArgs>
inline void signal<EvArgs>::operator()(EvArgs& args) const
{
    for (auto it {_slots.begin()}; it != _slots.end();) {
        if (it->second) {
            if constexpr (requires { args.Handled; }) {
                if (args.Handled) { break; }
            }
            it->second(args);
            ++it;
        } else {
            it = _slots.erase(it);
        }
    }
}

template <typename EvArgs>
template <typename Func>
inline auto signal<EvArgs>::connect(Func func) const -> connection
{
    if constexpr (std::is_convertible_v<Func, slot_func>) {
        uid const id {get_next_id()};
        _slots.emplace_back(id, func);
        return connection {this, id};
    } else {
        return connect([=](EvArgs&) { func(); });
    }
}

template <typename EvArgs>
template <auto Func, typename T>
inline auto signal<EvArgs>::connect(T* inst) const -> connection
{
    return connect([=](EvArgs& args) { (inst->*Func)(args); });
}

template <typename EvArgs>
inline void signal<EvArgs>::disconnect(uid id) const
{
    for (auto it {_slots.begin()}; it != _slots.end(); ++it) {
        if (it->first == id) {
            _slots.erase(it);
            return;
        }
    }
}

template <typename EvArgs>
inline void signal<EvArgs>::disconnect_all() const
{
    _slots.clear();
}

template <typename EvArgs>
inline auto signal<EvArgs>::get_slot_count() const -> isize
{
    return std::ssize(_slots);
}

////////////////////////////////////////////////////////////

inline void signal<void>::operator()() const
{
    for (auto it {_slots.begin()}; it != _slots.end();) {
        if (it->second) {
            it->second();
            ++it;
        } else {
            it = _slots.erase(it);
        }
    }
}

template <typename Func>
inline auto signal<void>::connect(Func func) const -> connection
{
    if constexpr (std::is_convertible_v<Func, slot_func>) {
        uid const id {get_next_id()};
        _slots.emplace_back(id, func);
        return connection {this, id};
    } else {
        return connect([=]() { func(); });
    }
}

template <auto Func, typename T>
inline auto signal<void>::connect(T* inst) const -> connection
{
    return connect([=]() { (inst->*Func)(); });
}

inline void signal<void>::disconnect(uid id) const
{
    for (auto it {_slots.begin()}; it != _slots.end(); ++it) {
        if (it->first == id) {
            _slots.erase(it);
            return;
        }
    }
}

inline void signal<void>::disconnect_all() const
{
    _slots.clear();
}

inline auto signal<void>::get_slot_count() const -> isize
{
    return std::ssize(_slots);
}

////////////////////////////////////////////////////////////
namespace detail {
    template <typename Signal, typename Func>
    inline void connection_manager::connect(Signal const& sig, Func func)
    {
        _connections.push_back(std::move(sig.connect(func)));
    }

    template <auto Func, typename Signal, typename Ptr>
    inline void connection_manager::connect(Signal const& sig, Ptr ptr)
    {
        _connections.push_back(std::move(sig.template connect<Func>(ptr)));
    }
}
}
