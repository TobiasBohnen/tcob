// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <thread>

namespace tcob {
////////////////////////////////////////////////////////////

enum class queue_status : u8 {
    Finished,
    Running
};

struct task_context {
    i32 Start {0};
    i32 End {0};
    i32 Thread {0};
};

class TCOB_API task_manager {
public:
    using queue_func = std::function<queue_status()>;

    explicit task_manager(std::optional<i32> threads);

    template <typename Func>
    auto run_async(Func&& func) -> std::future<std::invoke_result_t<Func>>;

    template <typename Func>
    void run_task(Func&& func, i32 count);

    void run_deferred(queue_func&& func);

    auto process_queue() -> queue_status;

    static inline char const* service_name {"task_manager"};

private:
    i32                          _threads;
    std::counting_semaphore<>    _semaphore;
    std::thread::id              _mainThreadID;
    std::queue<queue_func>       _deferredQueue {};
    mutable std::recursive_mutex _mutex {};
};

}

#include "TaskManager.inl"
