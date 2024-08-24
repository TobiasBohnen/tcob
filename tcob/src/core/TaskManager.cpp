// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/TaskManager.hpp"

namespace tcob {

task_manager::task_manager(std::optional<i32> threads)
    : _threadCount {(threads && *threads > 0) ? *threads : static_cast<i32>(std::thread::hardware_concurrency() * 2)}
    , _semaphore {_threadCount}
    , _mainThreadID {std::this_thread::get_id()}
{
    for (i32 i {0}; i < _threadCount; ++i) {
        _taskWorkers.emplace_back([this](std::stop_token const& stopToken) { worker_thread(stopToken); });
    }
}

task_manager::~task_manager()
{
    for (auto& worker : _taskWorkers) {
        worker.request_stop();
    }
    _taskCondition.notify_all();
    for (auto& worker : _taskWorkers) {
        worker.join();
    }
}

auto task_manager::get_thread_count() const -> i32
{
    return _threadCount;
}

void task_manager::run_deferred(queue_func&& func)
{
    std::scoped_lock lock {_deferredMutex};
    _deferredQueue.push(std::move(func));
}

auto task_manager::process_queue() -> queue_status
{
    assert(std::this_thread::get_id() == _mainThreadID);
    if (_deferredQueue.empty()) { return queue_status::Finished; }
    std::scoped_lock lock {_deferredMutex};

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

void task_manager::worker_thread(std::stop_token const& stopToken)
{
    while (!stopToken.stop_requested()) {
        std::function<void()> task;

        {
            std::unique_lock lock {_taskMutex};
            _taskCondition.wait(lock, stopToken, [this] { return !_taskQueue.empty(); });
            if (stopToken.stop_requested()) { return; }
            if (_taskQueue.empty()) { continue; }

            task = std::move(_taskQueue.front());
            _taskQueue.pop();
        }

        _semaphore.acquire();
        task();
        _semaphore.release();
        --_activeTasks;
    }
}

}
