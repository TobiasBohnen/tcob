#include "tests.hpp"

TEST_CASE("GFX.Image.FlipHorizontally")
{
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
        std::vector<u8> target {
            12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3,
            28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19,
            44, 45, 46, 47, 40, 41, 42, 43, 36, 37, 38, 39, 32, 33, 34, 35,
            60, 61, 62, 63, 56, 57, 58, 59, 52, 53, 54, 55, 48, 49, 50, 51};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("4x4 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
        std::vector<u8> target {
            9, 10, 11, 6, 7, 8, 3, 4, 5, 0, 1, 2,
            21, 22, 23, 18, 19, 20, 15, 16, 17, 12, 13, 14,
            33, 34, 35, 30, 31, 32, 27, 28, 29, 24, 25, 26,
            45, 46, 47, 42, 43, 44, 39, 40, 41, 36, 37, 38};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
        std::vector<u8> target {
            8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3,
            20, 21, 22, 23, 16, 17, 18, 19, 12, 13, 14, 15,
            32, 33, 34, 35, 28, 29, 30, 31, 24, 25, 26, 27};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26};
        std::vector<u8> target {
            6, 7, 8, 3, 4, 5, 0, 1, 2,
            15, 16, 17, 12, 13, 14, 9, 10, 11,
            24, 25, 26, 21, 22, 23, 18, 19, 20};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
}

TEST_CASE("GFX.Image.FlipVertically")
{
    SUBCASE("5x5 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {5, 5};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
            40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
            60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
            80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
        std::vector<u8> target {
            80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
            60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
            40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
        std::vector<u8> target {
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
        std::vector<u8> target {
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26};
        std::vector<u8> target {
            18, 19, 20, 21, 22, 23, 24, 25, 26,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            0, 1, 2, 3, 4, 5, 6, 7, 8};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.get_data().begin(), image.get_data().end()};
        REQUIRE(data1 == target);
    }
}

TEST_CASE("GFX.Image.GetSetPixel")
{
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {};

        auto image {image::Create(size, image::format::RGBA, source)};

        REQUIRE(image.get_pixel({0, 0}) == color {0, 0, 0, 0});
        image.set_pixel({0, 0}, colors::GoldenRod);
        REQUIRE(image.get_pixel({0, 0}) == colors::GoldenRod);

        REQUIRE(image.get_pixel({3, 3}) == color {0, 0, 0, 0});
        image.set_pixel({3, 3}, colors::SandyBrown);
        REQUIRE(image.get_pixel({3, 3}) == colors::SandyBrown);

        REQUIRE(image.get_pixel({2, 2}) == color {0, 0, 0, 0});
        image.set_pixel({2, 2}, colors::RebeccaPurple);
        REQUIRE(image.get_pixel({2, 2}) == colors::RebeccaPurple);
    }
    SUBCASE("4x4 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {};

        auto image {image::Create(size, image::format::RGB, source)};

        REQUIRE(image.get_pixel({0, 0}) == color {0, 0, 0, 255});
        image.set_pixel({0, 0}, colors::GoldenRod);
        REQUIRE(image.get_pixel({0, 0}) == colors::GoldenRod);

        REQUIRE(image.get_pixel({3, 3}) == color {0, 0, 0, 255});
        image.set_pixel({3, 3}, colors::SandyBrown);
        REQUIRE(image.get_pixel({3, 3}) == colors::SandyBrown);

        REQUIRE(image.get_pixel({2, 2}) == color {0, 0, 0, 255});
        image.set_pixel({2, 2}, colors::RebeccaPurple);
        REQUIRE(image.get_pixel({2, 2}) == colors::RebeccaPurple);
    }
}
