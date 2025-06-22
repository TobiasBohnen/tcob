// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/TaskManager.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <mutex>
#include <stop_token>
#include <thread>
#include <utility>

#include "tcob/core/Common.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob {

task_manager::task_manager(isize threads)
    : _threadCount {threads}
    , _mainThreadID {std::this_thread::get_id()}
{
    for (isize i {0}; i < _threadCount; ++i) {
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

    if (numThreads <= 1) {
        par_task const ctx {.Start = 0, .End = count, .Thread = 0};
        func(ctx);
    } else {
        isize const        partitionSize {count / numThreads};
        std::atomic<isize> activeTasks {numThreads};

        for (isize i {0}; i < numThreads; ++i) {
            par_task const ctx {.Start  = i * partitionSize,
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

auto task_manager::run_deferred(def_func const& func) -> uid
{
    uid const id {get_random_ID()};

    std::scoped_lock lock {_deferredMutex};
    _deferredQueueFront.emplace_back(func, id);
    return id;
}

void task_manager::drop_deferred(uid id)
{
    if (id == INVALID_ID) { return; }

    std::scoped_lock lock {_deferredMutex};

    helper::erase_first(_deferredQueueFront, [id](auto const& ctx) { return ctx.second == id; });
    helper::erase_first(_deferredQueueBack, [id](auto const& ctx) { return ctx.second == id; });
}

auto task_manager::thread_count() const -> isize
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

auto task_manager::process_queue(milliseconds deltaTime, bool abort) -> bool
{
    assert(std::this_thread::get_id() == _mainThreadID);
    if (_deferredQueueFront.empty()) { return true; }

    std::scoped_lock lock {_deferredMutex};
    std::swap(_deferredQueueFront, _deferredQueueBack);

    for (auto& task : _deferredQueueBack) {
        def_task const ctx {.DeltaTime = deltaTime, .AbortRequested = abort};
        task.first(ctx);
        if (!ctx.Finished) {
            _deferredQueueFront.emplace_back(std::move(task));
        }
    }
    _deferredQueueBack.clear();

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
