// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <vector>

#include "tcob/core/UserObject.hpp"

namespace tcob::ai {
////////////////////////////////////////////////////////////

class TCOB_API fsm final {
public:
    using condition_func = std::function<uid(user_object const&)>;
    using action_func    = std::function<void(user_object&)>;
    using tick_func      = std::function<void(user_object&, milliseconds)>;

    struct transition {
        uid            ID {};
        condition_func Condition {};
        action_func    OnTransition {};
    };

    struct state {
        uid         ID {};
        action_func OnEnter {};
        action_func OnExit {};
        tick_func   OnTick {};

        std::vector<transition> Transitions;
    };

    void add_state(state const& s);

    void start(uid initialStateID, user_object data = {});
    void stop();
    void tick(milliseconds dt);

    auto current_state() const -> uid;
    auto is_running() const -> bool;

    template <typename T>
    auto data() -> T*;
    template <typename T>
    auto data() const -> T const*;

private:
    auto find_state(uid id) const -> state const*;
    void enter_state(uid id);
    void exit_state(uid id);

    std::vector<state> _states {};
    uid                _current {};
    user_object        _data {};
    bool               _running {false};
};

////////////////////////////////////////////////////////////

template <typename T>
inline auto fsm::data() -> T*
{
    return _data.get<T>();
}

template <typename T>
inline auto fsm::data() const -> T const*
{
    return _data.get<T>();
}

}
