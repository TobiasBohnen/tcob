#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <tcob/tcob.hpp>

int main(int argc, char** argv)
{
    tcob::Game game { argv[0], "tcob_tests" };
    return Catch::Session().run(argc, argv);
}