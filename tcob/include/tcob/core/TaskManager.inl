// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TaskManager.hpp"

#include <future>

namespace tcob {

inline auto task_manager::run_async(auto&& func) -> std::future<std::invoke_result_t<decltype(func)>>
{
    return std::async(std::launch::async, [this, func]() mutable {
        _semaphore.acquire();
        auto retValue {func()};
        _semaphore.release();
        return retValue;
    });
}

inline void task_manager::run_task(auto&& func, i32 count)
{
    // TODO thread pool
    std::atomic_int counter {_threads};

    i32 const partitionSize {count / _threads};

    for (i32 i {0}; i < _threads; ++i) {
        task_context const ctx {.Start  = i * partitionSize,
                                .End    = (i == _threads - 1) ? count : ctx.Start + partitionSize,
                                .Thread = i};
        std::thread {[this, func, ctx, &counter]() {
            _semaphore.acquire();
            func(ctx);
            _semaphore.release();
            --counter;
        }}.detach();
    }

    while (counter > 0) {
        std::this_thread::yield();
    }
}

}
