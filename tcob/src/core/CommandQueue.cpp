// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/CommandQueue.hpp"

namespace tcob {

void command_queue::add(func_type&& func)
{
    _queue.push(std::move(func));
}

auto command_queue::is_empty() const -> bool
{
    return _queue.empty();
}

void command_queue::process()
{
    if (is_empty()) {
        return;
    }

    std::queue<func_type> newQueue {};

    while (!_queue.empty()) {
        auto& front {_queue.front()};
        if (front() == command_status::Running) {
            newQueue.push(front);
        }
        _queue.pop();
    }

    _queue.swap(newQueue);
}

} // namespace tcob
