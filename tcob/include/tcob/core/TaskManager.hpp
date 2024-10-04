// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace tcob {
////////////////////////////////////////////////////////////

enum class task_status : u8 {
    Finished,
    Running
};

struct task_context {
    isize Start {0};
    isize End {0};
    isize Thread {0};
};

////////////////////////////////////////////////////////////

class TCOB_API task_manager final {
    friend class game; // loop -> process_queue
    using task_func = std::function<void()>;

public:
    template <typename T>
    using async_func = std::function<T()>;
    using par_func   = std::function<void(task_context const&)>;
    using def_func   = std::function<task_status()>;

    explicit task_manager(i32 threads);
    ~task_manager();

    template <typename T>
    auto run_async(async_func<T> const& func) -> std::future<T>;

    void run_parallel(par_func const& func, isize count, isize minRange = 1);

    void run_deferred(def_func&& func);

    auto get_thread_count() const -> isize;

    static inline char const* service_name {"task_manager"};

private:
    void add_task(task_func&& func);

    auto process_queue() -> bool;

    void worker_thread(std::stop_token const& stopToken);

    isize           _threadCount;
    std::thread::id _mainThreadID;

    std::queue<task_func>       _taskQueue;
    std::mutex                  _taskMutex;
    std::vector<std::jthread>   _taskWorkers;
    std::condition_variable_any _taskCondition;

    std::queue<def_func> _deferredQueue {};
    std::recursive_mutex _deferredMutex {};
};

}

#include "TaskManager.inl"
