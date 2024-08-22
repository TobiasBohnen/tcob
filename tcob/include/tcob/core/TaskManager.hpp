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

enum class command_status : u8 {
    Finished,
    Running
};

class TCOB_API task_manager {
public:
    using func_type = std::function<command_status()>;

    explicit task_manager(std::optional<i32> threads);

    template <typename T>
    auto run_async(auto&& func) -> std::future<T>;

    void add_to_queue(func_type&& func);
    auto process_queue() -> command_status;

    static inline char const* service_name {"task_manager"};

private:
    i32                       _threads;
    std::counting_semaphore<> _semaphore;
    std::thread::id           _mainThreadID;

    std::queue<func_type>        _queue {};
    mutable std::recursive_mutex _mutex {};
};

}

#include "TaskManager.inl"
