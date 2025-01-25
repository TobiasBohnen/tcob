// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TaskManager.hpp"

#include <future>

namespace tcob {

template <typename T>
inline auto task_manager::run_async(async_func<T> const& func) -> std::future<T>
{
    auto task {std::make_shared<std::packaged_task<T()>>(func)};
    auto retValue {task->get_future()};
    if (_threadCount > 0) {
        add_task([task]() { (*task)(); });
    } else {
        (*task)();
    }

    return retValue;
}

}
