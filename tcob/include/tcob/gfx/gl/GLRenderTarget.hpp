// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Color.hpp>
#include <tcob/core/data/Size.hpp>
#include <tcob/gfx/Camera.hpp>
#include <tcob/gfx/Image.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/gl/GLFramebuffer.hpp>

namespace tcob::gl {
class RenderTarget {
public:
    RenderTarget();
    virtual ~RenderTarget() = default;
    RenderTarget(const RenderTarget&) = delete;
    auto operator=(const RenderTarget& other) -> RenderTarget& = delete;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    auto camera() -> Camera&;
    void camera(const Camera& camera);

    auto convert_world_to_screen(const PointF& point) -> PointI;
    auto convert_world_to_screen(const RectF& rect) -> RectI;
    auto convert_screen_to_world(const PointI& point) -> PointF;
    auto convert_screen_to_world(const RectI& rect) -> RectF;

    virtual auto size() const -> SizeU = 0;

    virtual void clear(const Color& c) const;
    void clear(const Color& c, const RectI& rect) const;

    void enable_scissor(const RectI& rect) const;
    void disable_scissor() const;

    auto read_pixel(const PointI& pos) const -> Color;
    auto create_screenshot() -> Image;

    virtual void setup_render(bool debug = false);
    virtual void finish_render() const;

protected:
    auto texture() const -> Texture2D*;

    void setup_framebuffer(const SizeU& size);

    void setup_ubo(bool debug);

private:
    std::unique_ptr<Framebuffer> _frameBuffer;

    std::shared_ptr<Texture2D> _texture;
    std::shared_ptr<Material> _material;
    ResourcePtr<Material> _matRes;

    Camera _camera;
};

////////////////////////////////////////////////////////////

class DefaultRenderTarget : public RenderTarget {
public:
    auto size() const -> SizeU override { return _size; }
    void size(const SizeU& newsize) { _size = newsize; }
    void setup_render(bool) override
    {
        Framebuffer::BindDefault();
        setup_ubo(false);
    }
    void finish_render() const override { Framebuffer::BindDefault(); }

private:
    SizeU _size { SizeU::Zero };
};
}