// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <iostream>
#include <tcob/tcob.hpp>

using namespace tcob;
namespace io = tcob::io;

auto convert_audio(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst, std::string const& ctx) -> int;
auto convert_config(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int;
auto convert_image(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int;
auto convert_misc(std::shared_ptr<io::ifstream>& in, std::string const& src, std::string const& srcExt, std::string const& dst) -> int;

auto inline print_error(std::string const& err) -> int
{
    std::cout << err;
    return 1;
}
