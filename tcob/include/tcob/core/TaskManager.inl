// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TaskManager.hpp"

#include <future>

namespace tcob {

template <typename Func>
inline auto task_manager::run_async(Func&& func) -> std::future<std::invoke_result_t<Func>>
{
    return std::async(std::launch::async, [this, func]() mutable {
        _semaphore.acquire();
        auto retValue {func()};
        _semaphore.release();
        return retValue;
    });
}

template <typename Func>
inline void task_manager::run_task(Func&& func, i32 count, i32 minRange)
{
    i32 const numThreads {std::min(_threadCount, count / minRange)};

    if (numThreads <= 1 || count < _threadCount) {
        task_context const ctx {.Start = 0, .End = count, .Thread = 0};
        func(ctx);
    } else {
        i32 const partitionSize {count / numThreads};
        _activeTasks = numThreads;

        for (i32 i {0}; i < numThreads; ++i) {
            task_context const ctx {.Start  = i * partitionSize,
                                    .End    = (i == numThreads - 1) ? count : ctx.Start + partitionSize,
                                    .Thread = i};
            {
                std::scoped_lock lock {_taskMutex};
                _taskQueue.emplace([func, ctx]() { func(ctx); });
            }

            _taskCondition.notify_one();
        }

        while (_activeTasks > 0) { std::this_thread::yield(); }
    }
}

}
