// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/TaskManager.hpp"

#include <atomic>

namespace tcob {

task_manager::task_manager(i32 threads)
    : _threadCount {threads}
    , _mainThreadID {std::this_thread::get_id()}
{
    for (i32 i {0}; i < _threadCount; ++i) {
        _taskWorkers.emplace_back([this](std::stop_token const& stopToken) { worker_thread(stopToken); });
    }
}

task_manager::~task_manager()
{
    for (auto& worker : _taskWorkers) { worker.request_stop(); }
    _taskCondition.notify_all();
    for (auto& worker : _taskWorkers) { worker.join(); }
}

void task_manager::run_parallel(par_func const& func, isize count, isize minRange)
{
    isize const numThreads {std::min(_threadCount, count / minRange)};

    if (numThreads <= 1 || count < _threadCount) {
        task_context const ctx {.Start = 0, .End = count, .Thread = 0};
        func(ctx);
    } else {
        isize const        partitionSize {count / numThreads};
        std::atomic<isize> activeTasks {numThreads};

        for (isize i {0}; i < numThreads; ++i) {
            task_context const ctx {.Start  = i * partitionSize,
                                    .End    = (i == numThreads - 1) ? count : ctx.Start + partitionSize,
                                    .Thread = i};
            add_task([func, ctx, &activeTasks]() {
                func(ctx);
                --activeTasks;
            });
        }

        while (activeTasks > 0) { std::this_thread::yield(); }
    }
}

void task_manager::run_deferred(def_func&& func)
{
    std::scoped_lock lock {_deferredMutex};
    _deferredQueue.push(std::move(func));
}

auto task_manager::get_thread_count() const -> isize
{
    return _threadCount;
}

void task_manager::add_task(task_func&& func)
{
    {
        std::scoped_lock lock {_taskMutex};
        _taskQueue.emplace(std::move(func));
    }
    _taskCondition.notify_one();
}

auto task_manager::process_queue() -> bool
{
    assert(std::this_thread::get_id() == _mainThreadID);
    if (_deferredQueue.empty()) { return true; }
    std::scoped_lock lock {_deferredMutex};

    std::queue<def_func> newQueue {};

    while (!_deferredQueue.empty()) {
        if (auto& front {_deferredQueue.front()}; front() == task_status::Running) { newQueue.push(front); }
        _deferredQueue.pop();
    }

    _deferredQueue.swap(newQueue);
    return false;
}

void task_manager::worker_thread(std::stop_token const& stopToken)
{
    while (!stopToken.stop_requested()) {
        task_func task;

        {
            std::unique_lock lock {_taskMutex};
            _taskCondition.wait(lock, stopToken, [this] { return !_taskQueue.empty(); });
            if (stopToken.stop_requested()) { return; }
            if (_taskQueue.empty()) { continue; }

            task = std::move(_taskQueue.front());
            _taskQueue.pop();
        }

        task();
    }
}

}
