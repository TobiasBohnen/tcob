// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Scene.hpp"

#include <ranges>

namespace tcob {

void scene::handle_input_event(auto&& event, auto&& handler)
{
    // TODO: add 'focused' entity
    _rootNode->handle_input_event(event, handler);
    if (event.Handled) { return; }

    // then the scene itself
    (this->*handler)(event);
}

inline void scene_node::handle_input_event(auto&& event, auto&& handler)
{
    // first nodes
    for (auto& ir : _children | std::views::reverse) {
        ir->handle_input_event(event, handler);
        if (event.Handled) { return; }
    }

    // then entity
    if (_entity && _entity->is_visible()) {
        ((_entity.get())->*handler)(event);
        if (event.Handled) { return; }
    }
}

}
