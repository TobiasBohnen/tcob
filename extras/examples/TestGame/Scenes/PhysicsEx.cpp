// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "PhysicsEx.hpp"
#include <iomanip>
#include <iostream>

////////////////////////////////////////////////////////////

PhysicsEx::PhysicsEx(game& game)
    : scene(game)
{
}

void PhysicsEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    case scan_code::D1:
        get_game().push_scene(std::make_shared<Box2DEx>(get_game()));
        break;
    case scan_code::D2:
        get_game().push_scene(std::make_shared<Chipmunk2DEx>(get_game()));
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////

Box2DEx::Box2DEx(game& game)
    : scene(game)
{
    _world.Gravity     = {0, 50.f};
    _material->Texture = _canvas.get_texture();
}

Box2DEx::~Box2DEx() = default;

void Box2DEx::on_start()
{
    _world.BeginContact.connect<&Box2DEx::on_begin_contact>(this);
    _world.EndContact.connect<&Box2DEx::on_end_contact>(this);

    auto* resGrp {locate_service<assets::library>().get_group("res")};

    _boxMat    = resGrp->get<material>("mat-solid");
    _circleMat = resGrp->get<material>("mat-circle");

    // obstacles
    rng r;
    _obstacles = _world.create_body({}, {});
    for (i32 i {0}; i < 25; i++) {
        create_obstacle({r(0.f, 66.f), r(2.f, 35.f), r(1.f, 5.f), r(1.f, 5.f)});
    }

    create_edge({0, 45}, {70, 45});

    auto sprite1 {_layer2.create_sprite()};
    sprite1->Material = _material;
    sprite1->Bounds   = {{0.f, 0.f}, {800.f, 600.f}};
}

void Box2DEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);

    _canvas.begin_frame({800, 600}, 1);
    _canvas.set_global_alpha(0.5f);
    _canvas.set_fill_style(colors::White);
    _canvas.begin_path();
    _canvas.rect({0, 0, 800, 600});
    _canvas.fill();
    _world.do_debug_draw(B2DDebugDraw {_canvas});
    _canvas.end_frame();

    _layer2.draw_to(target);
}

void Box2DEx::on_update(milliseconds deltaTime)
{
    for (auto& box : _objects) {
        box.Sprite->Center   = box.Body->Transform().Position * 12;
        box.Sprite->Rotation = box.Body->Transform().Angle;
    }

    _layer1.update(deltaTime);
    _layer2.update(deltaTime);
}

void Box2DEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();

    _world.update(deltaTime);
    if (_forceOn) {
        for (auto& obj : _objects) {
            obj.Body->apply_force_to_center({10.f, -300.f});
        }
    }
}

void Box2DEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::D1:
        _forceOn = !_forceOn;
        break;
    case scan_code::D2:
        for (auto& obj : _objects) {
            obj.Body->apply_linear_impulse_to_center({10.f, -300.f});
        }
        break;
    case scan_code::D3:
        for (auto& obj : _objects) {
            obj.Body->apply_torgue(500);
        }
        break;
    case scan_code::D4:
        for (auto& obj : _objects) {
            obj.Body->apply_angular_impulse(25);
        }
        break;
    case scan_code::G: {
        for (auto& obj : _objects) {
            obj.Body->wake_up();
        }

        _world.Gravity = -_world.Gravity;
    } break;
    case scan_code::C:
        _colorizeContact = !_colorizeContact;
        break;
    case scan_code::J: {
        if (_objects.size() >= 2) {
            auto                           b0 {_objects[_objects.size() - 2].Body};
            auto                           b1 {_objects[_objects.size() - 1].Body};
            box2d::distance_joint_settings def;
            def.initialize(b0, b1, b0->get_center(), b1->get_center());

            _world.create_joint(def);
        }
    } break;
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void Box2DEx::on_mouse_button_up(mouse::button_event& ev)
{
    point_f pos {ev.Position};
    pos /= point_f {12.f, 12.f};

    if (ev.Button == mouse::button::Left) {
        create_box({pos});
    } else if (ev.Button == mouse::button::Right) {
        create_circle({pos});
    }
}

void Box2DEx::create_box(point_f pos)
{
    Box2DExObject obj {};
    rect_f        rect {pos, {3, 3}};
    obj.Body = _world.create_body({.Position = rect.get_center()}, {.Type = body_type::Dynamic});

    box2d::polygon_shape shape;
    shape.set_as_box(rect.get_size());

    box2d::fixture_settings fixtureDef {
        .Friction = 0.3f,
        .Density  = 0.5f,
    };

    obj.Body->create_fixture(shape, fixtureDef);

    obj.Sprite           = _layer1.create_sprite();
    obj.Sprite->Material = _boxMat;
    obj.Sprite->Bounds   = rect * 12;
    obj.Sprite->Color    = colors::Red;

    _objects.push_back(obj);
}

void Box2DEx::create_circle(point_f pos)
{
    Box2DExObject obj {};
    rect_f        rect {pos, {3, 3}};
    obj.Body = _world.create_body({.Position = rect.get_center()}, {.Type = body_type::Dynamic});

    box2d::circle_shape shape;
    shape.Radius = rect.get_size().Width / 2;

    box2d::fixture_settings fixtureDef {
        .Friction = 0.3f,
        .Density  = 0.75f,
    };

    obj.Body->create_fixture(shape, fixtureDef);

    obj.Sprite           = _layer1.create_sprite();
    obj.Sprite->Material = _circleMat;
    obj.Sprite->Bounds   = rect * 12;
    obj.Sprite->Color    = colors::Yellow;

    _objects.push_back(obj);
}

void Box2DEx::create_obstacle(rect_f const& rect)
{
    box2d::polygon_shape shape;
    shape.set_as_box(rect, 0.0f);

    _obstacles->create_fixture(shape, {});

    auto sprite {_layer1.create_sprite()};
    sprite->Material = _boxMat;
    sprite->Bounds   = rect * 12;
    sprite->Color    = colors::Green;
}

void Box2DEx::create_edge(point_f pos0, point_f pos1)
{
    box2d::edge_shape shape;
    shape.set_two_sided(pos0, pos1);

    auto sprite {_layer1.create_sprite()};
    sprite->Material = _boxMat;
    sprite->Bounds   = rect_f::FromLTRB(pos0.X, pos0.Y, pos1.X, pos1.Y + 5) * 12;
    sprite->Color    = colors::Blue;

    _obstacles->create_fixture(shape, {});
}

void Box2DEx::on_begin_contact(box2d::contact_event const& ev)
{
    if (!_colorizeContact) {
        return;
    }

    auto& bodyA {ev.FixtureA->get_body()};
    auto& bodyB {ev.FixtureB->get_body()};
    if (*_obstacles == bodyA || *_obstacles == bodyB) {
        return;
    }

    for (auto& obj : _objects) {
        if (*obj.Body == bodyA) {
            obj.Sprite->Color = colors::Beige;
        }
        if (*obj.Body == bodyB) {
            obj.Sprite->Color = colors::Aquamarine;
        }
    }
}

void Box2DEx::on_end_contact(box2d::contact_event const& ev)
{
    if (!_colorizeContact) {
        return;
    }

    auto& bodyA {ev.FixtureA->get_body()};
    auto& bodyB {ev.FixtureB->get_body()};
    if (*_obstacles == bodyA || *_obstacles == bodyB) {
        return;
    }

    for (auto& obj : _objects) {
        if (*obj.Body == bodyA) {
            obj.Sprite->Color = colors::Black;
        }
        if (*obj.Body == bodyB) {
            obj.Sprite->Color = colors::Black;
        }
    }
}

////////////////////////////////////////////////////////////

Chipmunk2DEx::Chipmunk2DEx(game& game)
    : scene(game)
{
    _space.Gravity = {0, 50.f};
}

Chipmunk2DEx::~Chipmunk2DEx() = default;

void Chipmunk2DEx::on_start()
{
    _space.BeginContact.connect<&Chipmunk2DEx::on_begin_contact>(this);
    _space.EndContact.connect<&Chipmunk2DEx::on_end_contact>(this);

    auto* resGrp {locate_service<assets::library>().get_group("res")};

    _boxMat    = resGrp->get<material>("mat-solid");
    _circleMat = resGrp->get<material>("mat-circle");

    // obstacles
    rng r;
    _obstacles       = _space.create_body();
    _obstacles->Type = body_type::Static;
    _obstacles->activate();

    for (i32 i {0}; i < 25; i++) {
        create_obstacle({r(0.f, 66.f), r(2.f, 35.f), r(1.f, 5.f), r(1.f, 5.f)});
    }

    create_edge({0, 45}, {70, 45});
}

void Chipmunk2DEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);
}

void Chipmunk2DEx::on_update(milliseconds deltaTime)
{
    for (auto& box : _objects) {
        box.Sprite->Center   = point_f {box.Body->Position() * 12};
        box.Sprite->Rotation = degree_f {box.Body->Angle()};
    }

    _layer1.update(deltaTime);
}

void Chipmunk2DEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();

    _space.update(deltaTime);
    if (_forceOn) {
        for (auto& obj : _objects) {
            obj.Body->apply_force_at_local_point({10.f, -300.f}, {0, 0});
        }
    }
}

void Chipmunk2DEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::D1:
        _forceOn = !_forceOn;
        break;
    case scan_code::D2:
        for (auto& obj : _objects) {
            obj.Body->apply_impulse_at_local_point({10.f, -50.f}, {0, 0});
        }
        break;
    case scan_code::D3:
        for (auto& obj : _objects) {
            obj.Body->Torque = 500;
        }
        break;
    case scan_code::D4:
        for (auto& obj : _objects) {
            obj.Body->AngularVelocity = 25;
        }
        break;
    case scan_code::G: {
        for (auto& obj : _objects) {
            obj.Body->activate();
        }

        _space.Gravity = -_space.Gravity;
    } break;
    case scan_code::C:
        _colorizeContact = !_colorizeContact;
        break;
    case scan_code::J: {

        if (_objects.size() >= 2) {
            chipmunk2d::pivot_joint_settings def;
            def.A     = _objects[_objects.size() - 2].Body;
            def.B     = _objects[_objects.size() - 1].Body;
            def.Pivot = def.A->Position();

            _space.create_constraint(def);
        }
    } break;
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void Chipmunk2DEx::on_mouse_button_up(mouse::button_event& ev)
{
    point_f pos {ev.Position};
    pos /= point_f {12.f, 12.f};

    if (ev.Button == mouse::button::Left) {
        create_box(point_f {pos});
    } else if (ev.Button == mouse::button::Right) {
        create_circle(point_f {pos});
    }
}

void Chipmunk2DEx::create_box(point_f pos)
{
    Chipmunk2DExObject obj {};
    rect_f             rect {pos, {3, 3}};
    obj.Body           = _space.create_body();
    obj.Body->Position = rect.get_center();
    obj.Body->Type     = body_type::Dynamic;
    obj.Body->Mass     = 1;
    obj.Body->Moment   = chipmunk2d::moment_for_box(1, rect);

    auto shape = obj.Body->create_shape(
        chipmunk2d::box_shape_settings {rect.get_size(), 0});
    shape->Friction = 0.3f;

    obj.Sprite           = _layer1.create_sprite();
    obj.Sprite->Material = _boxMat;
    obj.Sprite->Bounds   = rect * 12;
    obj.Sprite->Color    = colors::Red;

    _objects.push_back(obj);
}

void Chipmunk2DEx::create_circle(point_f pos)
{
    Chipmunk2DExObject obj {};
    rect_f             rect {pos, {3, 3}};
    obj.Body           = _space.create_body();
    obj.Body->Position = rect.get_center();
    obj.Body->Type     = body_type::Dynamic;
    obj.Body->Mass     = 1;
    obj.Body->Moment   = chipmunk2d::moment_for_circle(1, 0, rect.Width / 2, point_f::Zero);

    auto shape = obj.Body->create_shape(
        chipmunk2d::circle_shape_settings {rect.get_size().Width / 2, point_f::Zero});
    shape->Friction = 0.3f;

    obj.Sprite           = _layer1.create_sprite();
    obj.Sprite->Material = _circleMat;
    obj.Sprite->Bounds   = rect * 12;
    obj.Sprite->Color    = colors::Yellow;

    _objects.push_back(obj);
}

void Chipmunk2DEx::create_obstacle(rect_f const& rect)
{
    auto shape = _obstacles->create_shape(
        chipmunk2d::box_shape_settings2 {rect, 0});

    shape->Friction = 1;

    auto sprite {_layer1.create_sprite()};
    sprite->Material = _boxMat;
    sprite->Bounds   = rect * 12;
    sprite->Color    = colors::Green;
}

void Chipmunk2DEx::create_edge(point_f pos0, point_f pos1)
{
    rect_f rect {rect_f::FromLTRB(pos0.X, pos0.Y, pos1.X, pos1.Y + 5)};

    auto shape = _obstacles->create_shape(
        chipmunk2d::box_shape_settings2 {rect, 0});
    shape->Friction = 1;

    auto sprite {_layer1.create_sprite()};
    sprite->Material = _boxMat;
    sprite->Bounds   = rect * 12;
    sprite->Color    = colors::Blue;
}

void Chipmunk2DEx::on_begin_contact(chipmunk2d::contact_event const& ev)
{
    if (!_colorizeContact) {
        return;
    }

    auto bodyA {ev.BodyA};
    auto bodyB {ev.BodyB};
    if (_obstacles == bodyA || _obstacles == bodyB) {
        return;
    }

    for (auto& obj : _objects) {
        if (obj.Body == bodyA) {
            obj.Sprite->Color = colors::Beige;
        }
        if (obj.Body == bodyB) {
            obj.Sprite->Color = colors::Aquamarine;
        }
    }
}

void Chipmunk2DEx::on_end_contact(chipmunk2d::contact_event const& ev)
{
    if (!_colorizeContact) {
        return;
    }

    auto bodyA {ev.BodyA};
    auto bodyB {ev.BodyB};
    if (_obstacles == bodyA || _obstacles == bodyB) {
        return;
    }

    for (auto& obj : _objects) {
        if (obj.Body == bodyA) {
            obj.Sprite->Color = colors::Black;
        }
        if (obj.Body == bodyB) {
            obj.Sprite->Color = colors::Black;
        }
    }
}

////////////////////////////////////////////////////////////

B2DDebugDraw::B2DDebugDraw(canvas& canvas)
    : _canvas {canvas}
{
}
void B2DDebugDraw::draw_polygon(std::span<point_f const> vertices, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);

    std::vector<point_f> verts;
    for (auto const vert : vertices) {
        verts.push_back(vert * point_f {12, 12});
    }
    _canvas.stroke_lines(verts);
}

void B2DDebugDraw::draw_solid_polygon(std::span<point_f const> vertices, color color)
{
    _canvas.set_fill_style(color);

    std::vector<point_f> verts;
    for (auto const vert : vertices) {
        verts.push_back(vert * point_f {12, 12});
    }
    _canvas.fill_lines(verts);
}

void B2DDebugDraw::draw_circle(point_f center, f32 radius, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);
    _canvas.begin_path();
    _canvas.circle(center * point_f {12, 12}, radius * 12);
    _canvas.stroke();
}

void B2DDebugDraw::draw_solid_circle(point_f center, f32 radius, point_f axis, color color)
{
    _canvas.set_fill_style(color);
    _canvas.begin_path();
    _canvas.circle(center * point_f {12, 12}, radius * 12);
    _canvas.fill();
}

void B2DDebugDraw::draw_segment(point_f p1, point_f p2, color color)
{
    _canvas.set_stroke_width(3);
    _canvas.set_stroke_style(color);
    _canvas.stroke_line(p1 * point_f {12, 12}, p2 * point_f {12, 12});
}

void B2DDebugDraw::draw_transform(box2d::body_transform const& xf)
{
}

void B2DDebugDraw::draw_point(point_f p, f32 size, color color)
{
    _canvas.set_fill_style(color);
    _canvas.begin_path();
    _canvas.circle(p * point_f {12, 12}, size);
    _canvas.fill();
}
