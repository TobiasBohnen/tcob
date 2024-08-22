// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "TaskManager.hpp"

#include <future>

namespace tcob {

template <typename T>
inline auto task_manager::run_async(auto&& func) -> std::future<T>
{
    return std::async(std::launch::async, [this, func]() mutable {
        _semaphore.acquire();
        auto retValue {func()};
        _semaphore.release();
        return retValue;
    });
}

}
