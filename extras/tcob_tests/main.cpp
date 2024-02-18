
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include <tcob/tcob.hpp>

auto main(int argc, char** argv) -> int
{
    auto pl{tcob::platform::HeadlessInit(argv[0], "tcob_tests.log")};

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run(); // run

    if (context.shouldExit()) {
        return res;
    }

    return res;
}
