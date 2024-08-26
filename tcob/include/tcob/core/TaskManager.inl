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
    using result_type = std::invoke_result_t<Func>;

    auto task {std::make_shared<std::packaged_task<result_type()>>(std::forward<Func>(func))};
    auto retValue {task->get_future()};
    if (_threadCount > 0) {
        {
            std::scoped_lock lock {_taskMutex};
            _taskQueue.emplace([task]() { (*task)(); });
        }
        _taskCondition.notify_one();
    }

    return retValue;
}

template <typename Func>
inline void task_manager::run_task(Func&& func, i32 count, i32 minRange)
{
    i32 const numThreads {std::min(_threadCount, count / minRange)};

    if (numThreads <= 1 || count < _threadCount) {
        task_context const ctx {.Start = 0, .End = count, .Thread = 0};
        func(ctx);
    } else {
        i32 const       partitionSize {count / numThreads};
        std::atomic_int activeTasks {numThreads};

        for (i32 i {0}; i < numThreads; ++i) {
            task_context const ctx {.Start  = i * partitionSize,
                                    .End    = (i == numThreads - 1) ? count : ctx.Start + partitionSize,
                                    .Thread = i};
            {
                std::scoped_lock lock {_taskMutex};
                _taskQueue.emplace([func, ctx, &activeTasks]() {
                    func(ctx);
                    --activeTasks;
                });
            }

            _taskCondition.notify_one();
        }

        while (activeTasks > 0) { std::this_thread::yield(); }
    }
}

}
