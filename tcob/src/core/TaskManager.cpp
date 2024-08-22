// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/TaskManager.hpp"

namespace tcob {

task_manager::task_manager(std::optional<i32> threads)
    : _threads {threads ? *threads : static_cast<i32>(std::thread::hardware_concurrency() * 2)}
    , _semaphore {_threads}
    , _mainThreadID {std::this_thread::get_id()}
{
}

void task_manager::add_to_queue(func_type&& func)
{
    std::scoped_lock lock {_mutex};
    _queue.push(std::move(func));
}

auto task_manager::process_queue() -> command_status
{
    assert(std::this_thread::get_id() == _mainThreadID);
    if (_queue.empty()) { return command_status::Finished; }
    std::scoped_lock lock {_mutex};

    std::queue<func_type> newQueue {};

    while (!_queue.empty()) {
        auto& front {_queue.front()};
        if (front() == command_status::Running) {
            newQueue.push(front);
        }
        _queue.pop();
    }

    _queue.swap(newQueue);
    return command_status::Running;
}

}
