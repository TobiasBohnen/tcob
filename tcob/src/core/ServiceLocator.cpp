// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/ServiceLocator.hpp"

namespace tcob {

service_locator::service_locator() = default;

service_locator::~service_locator() = default;

auto service_locator::GetInstance() -> service_locator&
{
    static service_locator instance;
    return instance;
}

}
