// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/FSM.hpp"
#include "tcob/core/UserObject.hpp"

#include <cassert>
#include <utility>

namespace tcob::ai {

void fsm::add_state(state const& s)
{
    assert(s.ID != INVALID_ID);
    assert(!_states.contains(s.ID));
    _states[s.ID] = s;
}

void fsm::add_global_transition(transition const& t)
{
    _globalTransitions.push_back(t);
}

void fsm::start(uid initialStateID, user_object data)
{
    if (_running) { return; }

    _data     = std::move(data);
    _previous = INVALID_ID;
    _running  = true;
    enter_state(initialStateID);

    StateChanged({.From = INVALID_ID, .To = initialStateID});
}

void fsm::stop()
{
    if (!_running) { return; }

    auto const from {_current};
    exit_current_state();
    _previous = from;
    _running  = false;

    StateChanged({.From = from, .To = INVALID_ID});
}

void fsm::update(milliseconds dt)
{
    if (!_running) { return; }
    _timeInState += dt;

    for (auto const& t : _globalTransitions) {
        if (t.Condition && t.Condition(_data)) {
            apply_transition(t);
            return;
        }
    }

    if (auto const* s {find_state(_current)}) {
        for (auto const& t : s->Transitions) {
            if (t.Condition && t.Condition(_data)) {
                apply_transition(t);
                return;
            }
        }

        if (s->OnUpdate) { s->OnUpdate(_data, dt); }
    }
}

void fsm::force_state(uid stateID)
{
    auto const from {_current};
    exit_current_state();
    _previous = from;
    enter_state(stateID);
    StateChanged({.From = from, .To = stateID});
}

auto fsm::current_state() const -> uid { return _current; }

auto fsm::previous_state() const -> uid { return _previous; }

auto fsm::time_in_state() const -> milliseconds { return _timeInState; }

auto fsm::is_running() const -> bool { return _running; }

auto fsm::find_state(uid id) const -> state const*
{
    auto it {_states.find(id)};
    return it != _states.end() ? &it->second : nullptr;
}

void fsm::enter_state(uid id)
{
    if (auto const* s {find_state(id)}) {
        _current     = id;
        _timeInState = milliseconds::zero();
        if (s->OnEnter) { s->OnEnter(_data); }
    }
}

void fsm::exit_current_state()
{
    if (auto const* s {find_state(_current)}) {
        if (s->OnExit) { s->OnExit(_data); }
    }
    _current = INVALID_ID;
}

void fsm::apply_transition(transition const& t)
{
    auto const from {_current};
    auto const to {t.TargetStateID};

    exit_current_state();

    _previous = from;

    if (t.OnTransition) { t.OnTransition(_data); }

    enter_state(to);

    StateChanged({.From = from, .To = to});
}

}
