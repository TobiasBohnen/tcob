// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <queue>

namespace tcob {
////////////////////////////////////////////////////////////

enum class command_status : u8 {
    Finished,
    Running
};

////////////////////////////////////////////////////////////

class TCOB_API command_queue {
public:
    using func_type = std::function<command_status()>;

    auto is_empty() const -> bool;

    void add(func_type&& func);
    void process();

    static inline char const* service_name {"command_queue"};

private:
    std::queue<func_type> _queue {};
};
}
