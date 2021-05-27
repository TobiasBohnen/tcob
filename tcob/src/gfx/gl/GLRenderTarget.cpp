// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/gl/GLRenderTarget.hpp>

#include <glad/gl.h>

#include <tcob/core/data/Color.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>
#include <tcob/gfx/gl/GLUniformBuffer.hpp>

namespace tcob::gl {
static auto GlobalUBO() -> gl::UniformBuffer&
{
    /*
    layout(std140, binding = 0)uniform Globals
    {
        mat4 camera;
        uvec2 view_size;
        bool debug; 
    };
    */

    static gl::UniformBuffer globalUniformBuffer { sizeof(mat4) + sizeof(uvec2) + sizeof(bool) };
    return globalUniformBuffer;
}

RenderTarget::RenderTarget()
    : _material { std::make_shared<Material>() }
    , _matRes { std::make_shared<Resource<Material>>(_material) }
{
}

auto RenderTarget::material() const -> ResourcePtr<Material>
{
    return _matRes;
}

void RenderTarget::material(ResourcePtr<Material> material)
{
    _matRes = std::move(material);

    if (_matRes) {
        _matRes->Texture = { std::make_shared<Resource<TextureBase>>(_texture) };
        _material = _matRes.get()->object_ptr();
    }
}

auto RenderTarget::camera() -> Camera&
{
    return _camera;
}

void RenderTarget::camera(const Camera& camera)
{
    _camera = camera;
    _camera.aspect_ratio(static_cast<f32>(size().Width) / size().Height);
}

auto RenderTarget::convert_world_to_screen(const PointF& point) -> PointI
{
    const PointF screen { _camera.convert_world_to_screen(point) };
    const SizeF s { size() };
    const i32 left { static_cast<i32>(std::round(screen.X * s.Width)) };
    const i32 top { static_cast<i32>(std::round(screen.Y * s.Height)) };
    return { left, top };
}

auto RenderTarget::convert_world_to_screen(const RectF& rect) -> RectI
{
    const RectF screen { _camera.convert_world_to_screen(rect) };
    const SizeF s { size() };
    const i32 left { static_cast<i32>(std::round(screen.Left * s.Width)) };
    const i32 top { static_cast<i32>(std::round(screen.Top * s.Height)) };
    const i32 width { static_cast<i32>(std::round(screen.Width * s.Width)) };
    const i32 height { static_cast<i32>(std::round(screen.Height * s.Height)) };
    return { left, top, width, height };
}

auto RenderTarget::convert_screen_to_world(const PointI& point) -> PointF
{
    const SizeF s { size() };
    const f32 left { static_cast<f32>(point.X) / s.Width };
    const f32 top { static_cast<f32>(point.Y) / s.Height };
    return _camera.convert_screen_to_world({ left, top });
}

auto RenderTarget::convert_screen_to_world(const RectI& rect) -> RectF
{
    const SizeF s { size() };
    const f32 left { static_cast<f32>(rect.Left) / s.Width };
    const f32 top { static_cast<f32>(rect.Top) / s.Height };
    const f32 width { static_cast<f32>(rect.Width) / s.Width };
    const f32 height { static_cast<f32>(rect.Height) / s.Height };
    return _camera.convert_screen_to_world({ left, top, width, height });
}

void RenderTarget::setup_render(bool debug)
{
    _frameBuffer.bind();
    const SizeI s { size() };
    glViewport(0, 0, s.Width, s.Height);

    // update uniform buffer
    setup_ubo(debug);
}

void RenderTarget::finish_render() const
{
    Framebuffer::BindDefault();
}

auto RenderTarget::texture() const -> Texture2D*
{
    return _texture.get();
}

void RenderTarget::clear(const Color& c, const RectI& rect) const
{
    enable_scissor(rect);
    clear(c);
    disable_scissor();
}

void RenderTarget::enable_scissor(const RectI& rect) const
{
    if (rect.Width < 0 || rect.Height < 0) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.Left, size().Height - rect.Top - rect.Height, rect.Width, rect.Height);
}

void RenderTarget::disable_scissor() const
{
    glDisable(GL_SCISSOR_TEST);
}

auto RenderTarget::read_pixel(const PointI& pos) const -> Color
{
    _frameBuffer.bind();
    std::array<u8, 4> data {};
    glReadPixels(pos.X, size().Height - pos.Y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    Framebuffer::BindDefault();
    return Color { data[0], data[1], data[2], data[3] };
}

auto RenderTarget::create_screenshot() -> Image
{
    _frameBuffer.bind();

    auto [width, height] = size();
    std::vector<u8> pixels;
    pixels.reserve(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    Framebuffer::BindDefault();

    return Image::CreateFromBuffer(size(), 4, pixels.data());
}

void RenderTarget::setup_ubo(bool debug)
{
    auto& buffer { GlobalUBO() };
    isize offset { 0 };

    auto matrix { _camera.matrix() };
    buffer.update(&matrix, sizeof(matrix), offset);
    offset += sizeof(matrix);

    auto s { size() };
    buffer.update(&s, sizeof(s), offset);
    offset += sizeof(s);

    buffer.update(&debug, sizeof(debug), offset);
    offset += sizeof(debug);

    buffer.bind_base(0);
}

void RenderTarget::clear(const Color& c) const
{
    _frameBuffer.clear(c);
}

void RenderTarget::create_framebuffer()
{
    _frameBuffer.create();
    _texture = std::make_shared<Texture2D>();
    _material->Texture = { std::make_shared<Resource<TextureBase>>(_texture) };
}

void RenderTarget::resize_framebuffer(const SizeU& size)
{
    // TODO: test this (maybe change texture to mutable)
    _texture->create(size);
    _frameBuffer.attach_texture(_texture.get());
}
}