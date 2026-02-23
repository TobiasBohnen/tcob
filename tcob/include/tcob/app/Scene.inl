// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Scene.hpp"

#include <ranges>

#include "tcob/core/Point.hpp"
#include "tcob/core/Transform.hpp"

namespace tcob {

inline void scene::handle_input_event(auto&& event, auto&& handler, transform const& xform)
{
    // first the root node
    if (_rootNode) {
        _rootNode->handle_input_event(event, handler, xform);
        if (event.Handled) { return; }
    }

    // then the scene itself
    (this->*handler)(event);
}

inline void scene_node::handle_input_event(auto&& event, auto&& handler, transform const& xform)
{
    transform const world {xform * get_transform()};

    // first nodes
    for (auto& child : _children | std::views::reverse) {
        child->handle_input_event(event, handler, world);
        if (event.Handled) { return; }
    }

    // then entity
    if (*Entity && Entity->is_visible()) {
        if constexpr (requires { event.Position; }) {
            auto ev {event};
            ev.Position = point_i {world.as_inverted() * point_f {ev.Position}};
            ((*Entity).get()->*handler)(ev);
        } else {
            ((*Entity).get()->*handler)(event);
        }
    }
}

}
