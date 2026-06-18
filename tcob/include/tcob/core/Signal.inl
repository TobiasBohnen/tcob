// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Signal.hpp"

namespace tcob {

template <typename EvArgs, typename Emitter>
inline void signal<EvArgs, Emitter>::operator()() const
    requires(IsVoid)
{
    for (isize i {0}; i < std::ssize(_slots); ++i) {
        auto const& [id, func] {_slots[i]};
        if (func) {
            func();
        }
    }
}

template <typename EvArgs, typename Emitter>
template <typename S>
inline void signal<EvArgs, Emitter>::operator()(S&& args) const // NOLINT
    requires(!IsVoid)
{
    for (isize i {0}; i < std::ssize(_slots); ++i) {
        auto const& [id, func] {_slots[i]};
        if (func) {
            if constexpr (requires { args.Handled; }) {
                if (args.Handled) { break; }
            }
            func(args);
        }
    }
}

template <typename EvArgs, typename Emitter>
template <typename Func>
inline auto signal<EvArgs, Emitter>::connect(Func func) const -> connection
{
    if constexpr (std::is_convertible_v<Func, slot_func>) {
        uid const id {next_id()};
        _slots.emplace_back(id, func);
        return connection {this, id};
    } else {
        if constexpr (IsVoid) {
            return connect([func] { func(); });
        } else {
            return connect([func](EvArgs&) { func(); });
        }
    }
}

template <typename EvArgs, typename Emitter>
template <auto Func, typename T>
inline auto signal<EvArgs, Emitter>::connect(T* inst) const -> connection
{
    if constexpr (IsVoid) {
        return connect([inst] { (inst->*Func)(); });
    } else {
        return connect([inst](EvArgs& args) { (inst->*Func)(args); });
    }
}

template <typename EvArgs, typename Emitter>
inline void signal<EvArgs, Emitter>::disconnect(uid id) const
{
    for (auto it {_slots.begin()}; it != _slots.end(); ++it) {
        if (it->first == id) {
            _slots.erase(it);
            return;
        }
    }
}

template <typename EvArgs, typename Emitter>
inline void signal<EvArgs, Emitter>::disconnect_all() const
{
    _slots.clear();
}

template <typename EvArgs, typename Emitter>
template <typename Func>
inline auto signal<EvArgs, Emitter>::operator+=(Func func) const -> connection
{
    return connect(std::move(func));
}

template <typename EvArgs, typename Emitter>
inline void signal<EvArgs, Emitter>::operator-=(connection& c) const
{
    c.disconnect();
}

template <typename EvArgs, typename Emitter>
inline void signal<EvArgs, Emitter>::operator-=(uid c) const
{
    disconnect(c);
}

template <typename EvArgs, typename Emitter>
inline auto signal<EvArgs, Emitter>::slot_count() const -> isize
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
