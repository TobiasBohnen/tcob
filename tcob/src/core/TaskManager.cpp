// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/TaskManager.hpp"

namespace tcob {

task_manager::task_manager(std::optional<i32> threads)
    : _threads {(threads && *threads > 0) ? *threads : static_cast<i32>(std::thread::hardware_concurrency() * 2)}
    , _semaphore {_threads}
    , _mainThreadID {std::this_thread::get_id()}
{
}

void task_manager::run_deferred(queue_func&& func)
{
    std::scoped_lock lock {_mutex};
    _deferredQueue.push(std::move(func));
}

auto task_manager::process_queue() -> queue_status
{
    assert(std::this_thread::get_id() == _mainThreadID);
    if (_deferredQueue.empty()) { return queue_status::Finished; }
    std::scoped_lock lock {_mutex};

    std::queue<queue_func> newQueue {};

    while (!_deferredQueue.empty()) {
        auto& front {_deferredQueue.front()};
        if (front() == queue_status::Running) {
            newQueue.push(front);
        }
        _deferredQueue.pop();
    }

    _deferredQueue.swap(newQueue);
    return queue_status::Running;
}

}
