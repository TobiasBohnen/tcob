// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <semaphore>

namespace tcob {
////////////////////////////////////////////////////////////

class TCOB_API semaphore {
public:
    explicit semaphore(i32 threads);

    void acquire();
    void release();

    static inline char const* service_name {"semaphore"}; // TODO: remove

private:
    std::counting_semaphore<> _semaphore;
};
}
