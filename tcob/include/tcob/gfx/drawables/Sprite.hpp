// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <vector>

#include <tcob/assets/Resource.hpp>
#include <tcob/core/data/Color.hpp>
#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>

namespace tcob {
class Sprite final : public RectTransformable, public Updatable {
public:
    Sprite();

    auto is_visible() const -> bool;
    void show();
    void hide();

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material, const std::string& texRegion = "default");
    void texture_region(const std::string& texRegion);
    void texture_scroll(const PointF& scroll);

    void color(const Color& color);
    void transparency(f32 trans);

    auto id() const -> u64;

    auto aabb() const -> RectF;

    void update(MilliSeconds deltaTime) override;

private:
    friend class SpriteBatch;

    void update_aabb();

    inline static u64 ID { 0 };
    u64 _id;

    Quad _quad {};
    RectF _aabb { RectF::Zero };
    ResourcePtr<Material> _material;
    Color _color {};

    PointF _textureScroll { PointF::Zero };
    bool _visible { true };
};

////////////////////////////////////////////////////////////

class SpriteBatch final : public Drawable {
public:
    auto create_sprite() -> Sprite&;

    void add_sprite(const Sprite& sprite);

    auto at(isize idx) -> Sprite&;

    auto find_by_id(u64 id) -> Sprite*;

    template <typename Predicate>
    auto find_if(Predicate func) -> Sprite*
    {
        auto it { std::find_if(_children.begin(), _children.end(), func) };
        if (it != _children.end()) {
            return &*it;
        }
        return nullptr;
    }

    template <typename Predicate>
    auto find_if_not(Predicate func) -> Sprite*
    {
        auto it { std::find_if_not(_children.begin(), _children.end(), func) };
        if (it != _children.end()) {
            return &*it;
        }
        return nullptr;
    }

    auto sprite_count() const -> isize;

    void update(MilliSeconds deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    std::vector<Sprite> _children {};
    gl::BatchQuadRenderer _renderer {};
};
}
