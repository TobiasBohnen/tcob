// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Semaphore.hpp"

namespace tcob {

semaphore::semaphore(i32 threads)
    : _semaphore {threads}
{
}

void semaphore::acquire()
{
    _semaphore.acquire();
}

void semaphore::release()
{
    _semaphore.release();
}

}
