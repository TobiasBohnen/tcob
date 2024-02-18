// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <iostream>
#include <tcob/tcob.hpp>

using namespace tcob;
namespace io = tcob::io;

auto inline print_error(std::string const& err) -> int
{
    std::cout << err;
    return 1;
}