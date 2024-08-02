#include "tests.hpp"

#include "PngFiles.hpp"

#define TEST_IMAGE(imageName, ext)              \
    SUBCASE(#imageName)                         \
    {                                           \
        test_image(#imageName, ext, imageName); \
    }                                           \
    static_cast<void>(0)

void test_image(std::string const& imageName, std::string const& ext, auto&& array)
{
    std::string imageFile {"testfiles/" + ext + "/" + imageName + "." + ext};
    REQUIRE(io::is_file(imageFile));
    auto img {image::Load(imageFile)};
    REQUIRE(img.has_value());

    {
        io::ifstream in {imageFile};
        if (auto sig {io::magic::get_signature(in)}) {
            REQUIRE(sig->Extension == "." + ext);
        }
    }

    auto const info {img->get_info()};
    u32 const  size {static_cast<u32>(info.size_in_bytes())};
    auto const buf {img->get_data()};

    if (info.bytes_per_pixel() == 4) {
        REQUIRE(size == std::ssize(array));
        for (u32 i {0}; i < array.size(); ++i) {
            REQUIRE_MESSAGE(buf[i] == array[i], std::format("{} {}:{}", i, buf[i], array[i]));
        }
    } else if (info.bytes_per_pixel() == 3) {
        REQUIRE(size == std::ssize(array) / 4 * 3);
        for (u32 i {0}, x {0}; i < size; i += 3, x++) {
            REQUIRE(buf[i + 0] == array[i + 0 + x]);
            REQUIRE(buf[i + 1] == array[i + 1 + x]);
            REQUIRE(buf[i + 2] == array[i + 2 + x]);
        }
    }
}

#define TEST_IMAGE_PNG(imageName) TEST_IMAGE(imageName, "png")

TEST_CASE("GFX.Decoder.PNG")
{
    TEST_IMAGE_PNG(basi0g01);
    TEST_IMAGE_PNG(basi0g02);
    TEST_IMAGE_PNG(basi0g04);
    TEST_IMAGE_PNG(basi0g08);
    TEST_IMAGE_PNG(basi0g16);
    TEST_IMAGE_PNG(basi2c08);
    TEST_IMAGE_PNG(basi2c16);
    TEST_IMAGE_PNG(basi3p01);
    TEST_IMAGE_PNG(basi3p02);
    TEST_IMAGE_PNG(basi3p04);
    TEST_IMAGE_PNG(basi3p08);
    TEST_IMAGE_PNG(basi4a08);
    TEST_IMAGE_PNG(basi4a16);
    TEST_IMAGE_PNG(basi6a08);
    TEST_IMAGE_PNG(basi6a16);
    TEST_IMAGE_PNG(basn0g01);
    TEST_IMAGE_PNG(basn0g02);
    TEST_IMAGE_PNG(basn0g04);
    TEST_IMAGE_PNG(basn0g08);
    TEST_IMAGE_PNG(basn0g16);
    TEST_IMAGE_PNG(basn2c08);
    TEST_IMAGE_PNG(basn2c16);
    TEST_IMAGE_PNG(basn3p01);
    TEST_IMAGE_PNG(basn3p02);
    TEST_IMAGE_PNG(basn3p04);
    TEST_IMAGE_PNG(basn3p08);
    TEST_IMAGE_PNG(basn4a08);
    TEST_IMAGE_PNG(basn4a16);
    TEST_IMAGE_PNG(basn6a08);
    TEST_IMAGE_PNG(basn6a16);

    TEST_IMAGE_PNG(tbbn2c16);
    TEST_IMAGE_PNG(tbbn3p08);
    TEST_IMAGE_PNG(tbgn2c16);
    TEST_IMAGE_PNG(tbgn3p08);
    TEST_IMAGE_PNG(tbrn2c08);
    TEST_IMAGE_PNG(tbwn0g16);
    TEST_IMAGE_PNG(tbwn3p08);
    TEST_IMAGE_PNG(tbyn3p08);
    TEST_IMAGE_PNG(tm3n3p02);
    TEST_IMAGE_PNG(tp0n0g08);
    TEST_IMAGE_PNG(tp0n2c08);
    TEST_IMAGE_PNG(tp0n3p08);
    TEST_IMAGE_PNG(tp1n3p08);

    TEST_IMAGE_PNG(cdfn2c08);
    TEST_IMAGE_PNG(cdhn2c08);
    TEST_IMAGE_PNG(cdsn2c08);

    TEST_IMAGE_PNG(s01i3p01);
    TEST_IMAGE_PNG(s01n3p01);
    TEST_IMAGE_PNG(s02i3p01);
    TEST_IMAGE_PNG(s02n3p01);
    TEST_IMAGE_PNG(s03i3p01);
    TEST_IMAGE_PNG(s03n3p01);
    TEST_IMAGE_PNG(s04i3p01);
    TEST_IMAGE_PNG(s04n3p01);
    TEST_IMAGE_PNG(s05i3p02);
    TEST_IMAGE_PNG(s05n3p02);
    TEST_IMAGE_PNG(s06i3p02);
    TEST_IMAGE_PNG(s06n3p02);
    TEST_IMAGE_PNG(s07i3p02);
    TEST_IMAGE_PNG(s07n3p02);
    TEST_IMAGE_PNG(s08i3p02);
    TEST_IMAGE_PNG(s08n3p02);
    TEST_IMAGE_PNG(s09i3p02);
    TEST_IMAGE_PNG(s09n3p02);
    TEST_IMAGE_PNG(s32i3p04);
    TEST_IMAGE_PNG(s32n3p04);
    TEST_IMAGE_PNG(s33i3p04);
    TEST_IMAGE_PNG(s33n3p04);
    TEST_IMAGE_PNG(s34i3p04);
    TEST_IMAGE_PNG(s34n3p04);
    TEST_IMAGE_PNG(s35i3p04);
    TEST_IMAGE_PNG(s35n3p04);
    TEST_IMAGE_PNG(s36i3p04);
    TEST_IMAGE_PNG(s36n3p04);
    TEST_IMAGE_PNG(s37i3p04);
    TEST_IMAGE_PNG(s37n3p04);
    TEST_IMAGE_PNG(s38i3p04);
    TEST_IMAGE_PNG(s38n3p04);
    TEST_IMAGE_PNG(s39i3p04);
    TEST_IMAGE_PNG(s39n3p04);
    TEST_IMAGE_PNG(s40i3p04);
    TEST_IMAGE_PNG(s40n3p04);

    TEST_IMAGE_PNG(f00n0g08);
    TEST_IMAGE_PNG(f00n2c08);
    TEST_IMAGE_PNG(f01n0g08);
    TEST_IMAGE_PNG(f01n2c08);
    TEST_IMAGE_PNG(f02n0g08);
    TEST_IMAGE_PNG(f02n2c08);
    TEST_IMAGE_PNG(f03n0g08);
    TEST_IMAGE_PNG(f03n2c08);
    TEST_IMAGE_PNG(f04n0g08);
    TEST_IMAGE_PNG(f04n2c08);
    TEST_IMAGE_PNG(f99n0g04);

    TEST_IMAGE_PNG(z00n2c08);
    TEST_IMAGE_PNG(z03n2c08);
    TEST_IMAGE_PNG(z06n2c08);
    TEST_IMAGE_PNG(z09n2c08);
}

#define TEST_IMAGE_GIF(imageName) TEST_IMAGE(imageName, "gif")

TEST_CASE("GFX.Decoder.GIF")
{
    TEST_IMAGE_GIF(basn0g01);
    TEST_IMAGE_GIF(basn0g02);
    TEST_IMAGE_GIF(basn0g04);
    TEST_IMAGE_GIF(basn0g08);
    TEST_IMAGE_GIF(basn3p01);
    TEST_IMAGE_GIF(basn3p02);
    TEST_IMAGE_GIF(basn3p04);
    TEST_IMAGE_GIF(basn3p08);
}

#define TEST_IMAGE_PCX(imageName) TEST_IMAGE(imageName, "pcx")

TEST_CASE("GFX.Decoder.PCX")
{
    TEST_IMAGE_PCX(basi3p02);
}

#define TEST_IMAGE_TGA(imageName) TEST_IMAGE(imageName, "tga")

TEST_CASE("GFX.Decoder.TGA")
{
    TEST_IMAGE_TGA(basi3p02);
}

#define TEST_IMAGE_BSI(imageName) TEST_IMAGE(imageName, "bsi")

TEST_CASE("GFX.Decoder.BSI")
{
    TEST_IMAGE_BSI(basi3p02);
}

#define TEST_IMAGE_BMP(imageName) TEST_IMAGE(imageName, "bmp")

TEST_CASE("GFX.Decoder.BMP")
{
    TEST_IMAGE_BMP(basi0g16);
    TEST_IMAGE_BMP(basi3p01);
    TEST_IMAGE_BMP(basi3p02);
    TEST_IMAGE_BMP(basi6a08);
    TEST_IMAGE_BMP(basn2c16);
    TEST_IMAGE_BMP(s07i3p02);
    TEST_IMAGE_BMP(s37i3p04);
}

#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    #define TEST_IMAGE_WEBP(imageName) TEST_IMAGE(imageName, "webp")

TEST_CASE("GFX.Decoder.webp")
{
    TEST_IMAGE_WEBP(basi3p02);
}
#endif

#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    #define TEST_IMAGE_QOI(imageName) TEST_IMAGE(imageName, "qoi")

TEST_CASE("GFX.Decoder.QOI")
{
    TEST_IMAGE_QOI(basi3p02);
}
#endif

TEST_CASE("GFX.Encoder.RGBA")
{
    image src {image::CreateEmpty({5, 7}, image::format::RGBA)};
    auto  srcData {src.get_data()};
    for (u8 i {0}; i < srcData.size(); ++i) {
        srcData[i] = i;
    }

    auto compareRGBA {[srcData](std::span<u8> s1) {
        REQUIRE(srcData.size_bytes() == s1.size_bytes());
        for (usize i {0}; i < srcData.size(); ++i) {
            REQUIRE(srcData[i] == s1[i]);
        }
    }};

    auto test {[&src](string const& ext) {
        io::iomstream str {};
        REQUIRE(src.save(str, ext));
        str.seek(0, io::seek_dir::Begin);
        auto dst {image::Load(str, ext)};
        REQUIRE(dst);
        return dst;
    }};

    SUBCASE("png")
    {
        compareRGBA(test(".png")->get_data());
    }

    SUBCASE("tga")
    {
        compareRGBA(test(".tga")->get_data());
    }

    SUBCASE("pcx")
    {
        auto compareRGB {[srcData](std::span<u8> s1) {
            REQUIRE(srcData.size_bytes() / 4 * 3 == s1.size_bytes());
            for (u32 i {0}, x {0}; i < s1.size(); i += 3, x++) {
                REQUIRE(srcData[i + 0 + x] == s1[i + 0]);
                REQUIRE(srcData[i + 1 + x] == s1[i + 1]);
                REQUIRE(srcData[i + 2 + x] == s1[i + 2]);
            }
        }};

        compareRGB(test(".pcx")->get_data());
    }

    SUBCASE("bmp")
    {
        compareRGBA(test(".tga")->get_data());
    }

    SUBCASE("bsi")
    {
        compareRGBA(test(".bsi")->get_data());
    }

#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    SUBCASE("webp")
    {
        compareRGBA(test(".webp")->get_data());
    }
#endif

#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    SUBCASE("qoi")
    {
        compareRGBA(test(".qoi")->get_data());
    }
#endif
}

TEST_CASE("GFX.Encoder.RGB")
{
    image src {image::CreateEmpty({5, 7}, image::format::RGB)};
    auto  srcData {src.get_data()};
    for (u8 i {0}; i < srcData.size(); ++i) {
        srcData[i] = i;
    }

    auto compareRGB {[srcData](std::span<u8> s1) {
        REQUIRE(srcData.size_bytes() == s1.size_bytes());
        for (usize i {0}; i < srcData.size(); ++i) {
            REQUIRE(srcData[i] == s1[i]);
        }
    }};

    auto test {[&src](string const& ext) {
        io::iomstream str {};
        REQUIRE(src.save(str, ext));
        str.seek(0, io::seek_dir::Begin);
        auto dst {image::Load(str, ext)};
        REQUIRE(dst);
        return dst;
    }};

    SUBCASE("png")
    {
        auto compareRGBA {[srcData](std::span<u8> s1) {
            REQUIRE(srcData.size_bytes() / 3 * 4 == s1.size_bytes());
            for (u32 i {0}, x {0}; i < srcData.size(); i += 3, x++) {
                REQUIRE(srcData[i + 0] == s1[i + 0 + x]);
                REQUIRE(srcData[i + 1] == s1[i + 1 + x]);
                REQUIRE(srcData[i + 2] == s1[i + 2 + x]);
            }
        }};

        compareRGBA(test(".png")->get_data());
    }

    SUBCASE("tga")
    {
        compareRGB(test(".tga")->get_data());
    }

    SUBCASE("pcx")
    {
        compareRGB(test(".pcx")->get_data());
    }

    SUBCASE("bmp")
    {
        compareRGB(test(".tga")->get_data());
    }

    SUBCASE("bsi")
    {
        compareRGB(test(".bsi")->get_data());
    }

#if defined(TCOB_ENABLE_FILETYPES_GFX_WEBP)
    SUBCASE("webp")
    {
        compareRGB(test(".webp")->get_data());
    }
#endif

#if defined(TCOB_ENABLE_FILETYPES_GFX_QOI)
    SUBCASE("qoi")
    {
        compareRGB(test(".qoi")->get_data());
    }
#endif
}
