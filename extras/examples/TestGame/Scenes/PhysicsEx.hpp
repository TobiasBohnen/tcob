// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "../Common.hpp"

////////////////////////////////////////////////////////////

class PhysicsEx : public scene {
public:
    PhysicsEx(game& game);

protected:
    void on_draw_to(render_target& target) override { }
    void on_update(milliseconds deltaTime) override { }
    void on_fixed_update(milliseconds deltaTime) override { }

    void on_key_down(keyboard::event& ev) override;
};

////////////////////////////////////////////////////////////

class B2DDebugDraw : public box2d::debug_draw {
public:
    B2DDebugDraw(canvas& canvas);

    void draw_polygon(std::span<point_f const> vertices, color color) override;

    /// Draw a solid closed polygon provided in CCW order.
    void draw_solid_polygon(std::span<point_f const> vertices, color color) override;

    /// Draw a circle.
    void draw_circle(point_f center, f32 radius, color color) override;

    /// Draw a solid circle.
    void draw_solid_circle(point_f center, f32 radius, point_f axis, color color) override;

    /// Draw a line segment.
    void draw_segment(point_f p1, point_f p2, color color) override;

    /// Draw a transform. Choose your own length scale.
    /// @param xf a transform.
    void draw_transform(box2d::body_transform const& xf) override;

    /// Draw a point.
    void draw_point(point_f p, f32 size, color color) override;

private:
    canvas& _canvas;
};

class Box2DEx : public scene {
public:
    Box2DEx(game& game);
    ~Box2DEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_button_up(mouse::button_event& ev) override;

private:
    struct Box2DExObject {
        std::shared_ptr<sprite>      Sprite;
        std::shared_ptr<box2d::body> Body;
    };

    void create_box(point_f pos);
    void create_edge(point_f pos0, point_f pos1);
    void create_circle(point_f pos);
    void create_obstacle(rect_f const& rect);

    void on_begin_contact(box2d::contact_event const& ev);
    void on_end_contact(box2d::contact_event const& ev);

    std::shared_ptr<box2d::body> _obstacles {};
    std::vector<Box2DExObject>   _objects {};

    assets::asset_ptr<material> _boxMat;
    assets::asset_ptr<material> _circleMat;

    box2d::world _world {};

    sprite_batch _layer1;
    bool         _forceOn {false};
    bool         _colorizeContact {false};

    canvas                             _canvas;
    sprite_batch                       _layer2;
    assets::manual_asset_ptr<material> _material {};
};

////////////////////////////////////////////////////////////

class Chipmunk2DEx : public scene {
public:
    Chipmunk2DEx(game& game);
    ~Chipmunk2DEx() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_button_up(mouse::button_event& ev) override;

private:
    struct Chipmunk2DExObject {
        std::shared_ptr<sprite>           Sprite;
        std::shared_ptr<chipmunk2d::body> Body;
    };

    void create_box(point_f pos);
    void create_edge(point_f pos0, point_f pos1);
    void create_circle(point_f pos);
    void create_obstacle(rect_f const& rect);

    void on_begin_contact(chipmunk2d::contact_event const& ev);
    void on_end_contact(chipmunk2d::contact_event const& ev);

    std::shared_ptr<chipmunk2d::body> _obstacles {};
    std::vector<Chipmunk2DExObject>   _objects {};

    assets::asset_ptr<material> _boxMat;
    assets::asset_ptr<material> _circleMat;

    chipmunk2d::space _space {};
    bool              _forceOn {false};
    bool              _colorizeContact {false};
    sprite_batch      _layer1;
};