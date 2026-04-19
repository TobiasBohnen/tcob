// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/FSM.hpp"
#include "tcob/core/UserObject.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace tcob::ai {

void fsm::add_state(state const& s) { _states.push_back(s); }

void fsm::start(uid initialStateID, user_object data)
{
    if (_running) { return; }
    _data    = std::move(data);
    _running = true;
    enter_state(initialStateID);
}

void fsm::stop()
{
    if (!_running) { return; }
    exit_state(_current);
    _current = INVALID_ID;
    _running = false;
}

void fsm::tick(milliseconds dt)
{
    if (!_running) { return; }

    if (auto const* s {find_state(_current)}) {
        for (auto const& t : s->Transitions) {
            if (!t.Condition) { continue; }

            uid const next {t.Condition(_data)};
            if (next != INVALID_ID) {
                exit_state(_current);
                if (t.OnTransition) { t.OnTransition(_data); }
                enter_state(next);
                return;
            }
        }

        if (s->OnTick) { s->OnTick(_data, dt); }
    }
}

auto fsm::current_state() const -> uid { return _current; }
auto fsm::is_running() const -> bool { return _running; }

auto fsm::find_state(uid id) const -> state const*
{
    auto it {std::ranges::find(_states, id, &state::ID)};
    return it != _states.end() ? &*it : nullptr;
}

void fsm::enter_state(uid id)
{
    _current = id;
    if (auto const* s {find_state(id)}) {
        if (s->OnEnter) { s->OnEnter(_data); }
    }
}

void fsm::exit_state(uid id)
{
    if (auto const* s {find_state(id)}) {
        if (s->OnExit) { s->OnExit(_data); }
    }
}
}
