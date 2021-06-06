// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/core/data/Rect.hpp>
#include <tcob/gfx/Image.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob {
struct TextureRegion final {
    RectF UVRect;
    u32 Level { 0 };
};
}

namespace tcob::gl {
enum class TextureFormat : u8 {
    R8,
    RGB8,
    RGBA8
};

enum class TextureFiltering : u8 {
    Linear,
    NearestNeighbor
};

enum class TextureWrap : u8 {
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat,
    Repeat,
    MirrorClampToEdge
};

class Texture : public Object {
public:
    ~Texture() override;

    void bind_texture_unit(u32 tu = 0) const;

    auto size() const -> SizeU;

    void filtering(TextureFiltering filter) const;
    void wrapping(TextureWrap wrap) const;
    void wrapping(TextureWrap wrapS, TextureWrap wrapT) const;

    virtual auto copy_to_image() const -> Image;

    virtual auto format() -> TextureFormat = 0;

    auto regions() -> std::unordered_map<std::string, TextureRegion>&;

protected:
    Texture() = default;

    void size(const SizeU& size);

    void do_destroy() override;

private:
    std::unordered_map<std::string, TextureRegion> _regions {};
    SizeU _size { SizeU::Zero };
};

class Texture1D : public Texture {
public:
    Texture1D();

    void create(u32 texsize);

    void update(i32 offsetX, i32 width, const void* data) const;

    auto format() -> TextureFormat override;
};

class Texture2D : public Texture {
public:
    Texture2D();

    void create(const SizeU& texsize, TextureFormat format = TextureFormat::RGBA8);

    void update(const PointU& origin, const SizeU& size, const void* data, i32 rowLength = 0, i32 alignment = 4) const;

    auto format() -> TextureFormat override;

private:
    i32 _format { 0 };
};

class Texture2DArray : public Texture {
public:
    Texture2DArray();

    void create(const SizeU& texsize, u32 depth, TextureFormat format = TextureFormat::RGBA8);

    void update(const PointU& origin, const SizeU& size, const void* data, u32 depth, i32 rowLength = 0, i32 alignment = 4) const;

    auto format() -> TextureFormat override;

    auto depth() const -> u32;

    auto copy_to_image() const -> Image override;

private:
    i32 _format { 0 };
    u32 _depth { 0 };
};
}