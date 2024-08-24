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

class TCOB_API task_manager final {
public:
    using queue_func = std::function<queue_status()>;

    explicit task_manager(std::optional<i32> threads);
    ~task_manager();

    auto get_thread_count() const -> i32;

    template <typename Func>
    auto run_async(Func&& func) -> std::future<std::invoke_result_t<Func>>;

    template <typename Func>
    void run_task(Func&& func, i32 count, i32 minRange = 1);

    void run_deferred(queue_func&& func);

    auto process_queue() -> queue_status;

    static inline char const* service_name {"task_manager"};

private:
    void worker_thread(std::stop_token const& stopToken);

    i32                       _threadCount;
    std::counting_semaphore<> _semaphore;
    std::thread::id           _mainThreadID;

    std::queue<std::function<void()>> _taskQueue;
    std::mutex                        _taskMutex;
    std::vector<std::jthread>         _taskWorkers;
    std::condition_variable_any       _taskCondition;
    std::atomic_int                   _activeTasks;

    std::queue<queue_func>       _deferredQueue {};
    mutable std::recursive_mutex _deferredMutex {};
};

}

#include "TaskManager.inl"
