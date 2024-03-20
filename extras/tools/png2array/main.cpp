// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "../shared/argparse.hpp"
#include <iostream>
#include <sstream>
#include <tcob/tcob.hpp>

using namespace tcob;
namespace io = tcob::io;

auto static convert(std::string const& srcFile) -> std::string
{
    auto const img {gfx::image::Load(srcFile)};
    if (img) {
        std::stringstream ss;
        auto const        size {img->get_info().size_in_bytes()};
        auto const        buf {img->get_data()};

        ss << "constexpr std::array<uint8_t, " << size << "> " << io::get_stem(srcFile);
        ss << " {";

        for (i32 i {0}; i < size; ++i) {
            if (i % 16 == 0) {
                ss << "\n ";
            }
            ss << "0x"
               << std::setfill('0') << std::setw(2)
               << std::hex << static_cast<u32>(buf[i]);

            if (i != size - 1) {
                ss << ", ";
            }
        }

        ss << " };\n";

        return ss.str();
    }

    return "";
}

auto main(int argc, char* argv[]) -> int
{
    argparse::ArgumentParser program("png2array");
    program.add_argument("folder");

    try {
        program.parse_args(argc, argv);
    } catch (std::exception const& err) {
        std::cout << err.what() << '\n';
        std::cout << program;
        return 1;
    }

    auto pl {platform::HeadlessInit(argv[0])};

    std::string const arg {program.get("folder")};
    if (!io::is_folder(arg)) {
        return 1;
    }

    auto const files {io::enumerate(arg, {"*.png"})};

    std::cout << "#include <array>\n";
    std::cout << "#include <cstdint>\n\n";

    for (auto const& file : files) {
        std::cout << convert(file);
    }

    return 0;
}
