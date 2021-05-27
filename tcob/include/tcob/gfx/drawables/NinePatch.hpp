// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/tcob_config.hpp>

#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

namespace tcob {
class NinePatch final : public RectTransformable, public Drawable {
public:
    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material, const std::string& texRegion = "default");

    void texture_region(const std::string& texRegion);
    void define_center(const PointF& posTopLeft, const PointF& posBottomRight, const PointF& uvTopLeft, const PointF& uvBottomRight);

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    gl::DynamicQuadRenderer _renderer {};

    ResourcePtr<Material> _material;
    TextureRegion _texRegion {};

    PointF _posTopLeft { PointF::Zero };
    PointF _posBottomRight { PointF::Zero };
    PointF _uvTopLeft { PointF::Zero };
    PointF _uvBottomRight { PointF::Zero };

    bool _isDirty { true };
};
}
