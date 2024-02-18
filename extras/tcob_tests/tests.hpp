#pragma once

#ifdef _MSC_VER
    #pragma warning(disable : 4706)
#endif

#include <doctest/doctest.h>
#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::gfx;
using namespace tcob::gfx::ui;

using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;
using Approx = doctest::Approx;

void inline PrepareFile(std::string const& file)
{
    io::delete_file(file);
    REQUIRE_FALSE(io::exists(file));
    io::create_file(file);
    REQUIRE(io::exists(file));
}
