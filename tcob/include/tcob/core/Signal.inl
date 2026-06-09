// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Signal.hpp"

namespace tcob {

template <typename EvArgs>
inline void signal<EvArgs>::operator()() const
    requires(IsVoid)
{
    bool needsCleanup {false};
    for (isize i {0}; i < std::ssize(_slots); ++i) {
        auto const& [id, func] {_slots[i]};
        if (func) {
            func();
        } else {
            needsCleanup = true;
        }
    }

    if (needsCleanup) {
        std::erase_if(_slots, [](auto const& slot) { return !slot.second; });
    }
}

template <typename EvArgs>
template <typename S>
inline void signal<EvArgs>::operator()(S&& args) const // NOLINT
    requires(!IsVoid)
{
    bool needsCleanup {false};
    for (isize i {0}; i < std::ssize(_slots); ++i) {
        auto const& [id, func] {_slots[i]};
        if (func) {
            if constexpr (requires { args.Handled; }) {
                if (args.Handled) { break; }
            }
            func(args);
        } else {
            needsCleanup = true;
        }
    }
    if (needsCleanup) {
        std::erase_if(_slots, [](auto const& slot) { return !slot.second; });
    }
}

template <typename EvArgs>
template <typename Func>
inline auto signal<EvArgs>::connect(Func func) const -> connection
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

template <typename EvArgs>
template <auto Func, typename T>
inline auto signal<EvArgs>::connect(T* inst) const -> connection
{
    if constexpr (IsVoid) {
        return connect([inst] { (inst->*Func)(); });
    } else {
        return connect([inst](EvArgs& args) { (inst->*Func)(args); });
    }
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
template <typename Func>
inline auto signal<EvArgs>::operator+=(Func func) const -> connection
{
    return connect(std::move(func));
}

template <typename EvArgs>
inline void signal<EvArgs>::operator-=(connection& c) const
{
    c.disconnect();
}

template <typename EvArgs>
inline void signal<EvArgs>::operator-=(uid c) const
{
    disconnect(c);
}

template <typename EvArgs>
inline auto signal<EvArgs>::slot_count() const -> isize
{
    return std::ssize(_slots);
}

////////////////////////////////////////////////////////////

template <typename EvArgs>
inline signal_view<EvArgs>::signal_view(signal<EvArgs>& sig)
    : _sig {sig}
{
}

template <typename EvArgs>
template <typename Func>
inline auto signal_view<EvArgs>::connect(Func func) const -> connection { return _sig.connect(std::move(func)); }
template <typename EvArgs>
template <auto Func, typename T>
inline auto signal_view<EvArgs>::connect(T* inst) const -> connection { return _sig.connect<Func>(inst); }
template <typename EvArgs>
inline void signal_view<EvArgs>::disconnect(uid id) const { _sig.disconnect(id); }
template <typename EvArgs>
inline void signal_view<EvArgs>::disconnect_all() const { _sig.disconnect_all(); }
template <typename EvArgs>
inline auto signal_view<EvArgs>::operator+=(auto func) const -> connection { return _sig += std::move(func); }
template <typename EvArgs>
inline void signal_view<EvArgs>::operator-=(connection& c) const { _sig -= c; }
template <typename EvArgs>
inline void signal_view<EvArgs>::operator-=(uid id) const { _sig -= id; }
template <typename EvArgs>
inline auto signal_view<EvArgs>::slot_count() const -> isize { return _sig.slot_count(); }

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
