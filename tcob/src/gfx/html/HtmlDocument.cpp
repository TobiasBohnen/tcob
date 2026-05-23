// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/html/HtmlDocument.hpp"

#include <memory>
#include <types.h>
#include <utility>

#include <litehtml.h>
#include <litehtml/el_space.h>
#include <litehtml/el_text.h>
#include <litehtml/master_css.h>

#include "HtmlContainer.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/RenderTexture.hpp"

namespace tcob::gfx::html {

////////////////////////////////////////////////////////////

document::document(config c)
    : _config {std::move(c)}
    , _container {std::make_shared<detail::container>(*this, _config, _canvas)}
{
    Bounds.Changed.connect([this](auto const&) { mark_transform_dirty(); });

    geometry::set_color(_quad, colors::White);
    geometry::set_texcoords(_quad, {.UVRect = render_texture::UVRect(), .Level = 0});
    _material->first_pass().Texture = _canvas.get_texture();
}

document::~document() = default;

auto document::mouse_position() const -> point_i
{
    return _mousePosition;
}

auto document::is_button_down() const -> bool
{
    return _buttonDown;
}

auto document::bounds() const -> rect_f
{
    return Bounds;
}

void document::from_string(string const& html)
{
    string css;
    if (io::is_file(_config.MasterCSSPath)) {
        io::ifstream stream {_config.MasterCSSPath};
        css = stream.read_string(stream.size_in_bytes());
    }

    from_string(html, css);
}

void document::from_string(string const& html, string const& css)
{
    _lhdoc = litehtml::document::createFromString(html, _container.get(), litehtml::master_css, css);
    force_redraw();
}

auto document::load(path const& file) noexcept -> bool
{
    io::ifstream fs {file};
    if (!fs) { return false; }
    from_string(io::read_as_string(file));
    return true;
}

void document::force_redraw()
{
    _needsRedraw = true;
}

void document::on_update(milliseconds)
{
    if (_isTransformDirty) {
        geometry::set_position(_quad, Bounds);
        _isTransformDirty = false;
        _needsRedraw      = true;
    }
}

auto document::can_draw() const -> bool
{
    return _lhdoc != nullptr;
}

void document::on_draw_to(render_target& target, transform const& xform)
{
    if (!_lhdoc) { return; }

    if (_needsRedraw) {
        litehtml::position::vector redraw {};
        _lhdoc->root()->find_styles_changes(redraw);

        size_f const size {Bounds->Size};

        _container->set_size(size_i {size});
        _canvas.begin_frame(size_i {size}, 1.0f);
        _lhdoc->render(size.Width);
        litehtml::position const pos {0, 0, size.Width, size.Height};
        _lhdoc->draw(0, 0, 0, &pos);
        _canvas.end_frame();

        _needsRedraw = false;
    }

    _renderer.set_geometry({.Vertices = _quad, .Indices = QuadIndicies, .Type = primitive_type::Triangles}, &_material->first_pass());
    _renderer.render_to_target(target, xform * get_transform());
}

auto document::pivot() const -> point_f
{
    return Bounds->center();
}

void document::on_transform_changed()
{
    _isTransformDirty = true;
}

void document::on_mouse_motion(input::mouse::motion_event const& ev)
{
    if (!_lhdoc) { return; }

    litehtml::position::vector redraw {};

    rect_i const  bound {*Bounds};
    point_i const mp {convert_screen_to_world(ev.Position)};

    if (bound.contains(mp)) {
        _mousePosition = mp - bound.Position;
        if (_lhdoc->on_mouse_over(
                static_cast<litehtml::pixel_t>(mp.X - bound.left()),
                static_cast<litehtml::pixel_t>(mp.Y - bound.top()),
                static_cast<litehtml::pixel_t>(mp.X),
                static_cast<litehtml::pixel_t>(mp.Y),
                redraw)) {
            force_redraw();
        }

        _isMouseOver = true;
    } else {
        _mousePosition = {-1, -1};
        if (_isMouseOver) {
            if (_lhdoc->on_mouse_leave(redraw)) {
                force_redraw();
            }

            _isMouseOver = false;
        }
    }
}

void document::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (!_lhdoc) { return; }

    if (ev.Button == input::mouse::button::Left && _isMouseOver) {
        _buttonDown = true;

        litehtml::position::vector redraw {};

        rect_i const  bound {*Bounds};
        point_i const mp {convert_screen_to_world(ev.Position)};

        if (_lhdoc->on_lbutton_down(
                static_cast<litehtml::pixel_t>(mp.X - bound.left()),
                static_cast<litehtml::pixel_t>(mp.Y - bound.top()),
                static_cast<litehtml::pixel_t>(mp.X),
                static_cast<litehtml::pixel_t>(mp.Y),
                redraw)) {
            force_redraw();
        }
    }
}

void document::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (!_lhdoc) { return; }

    if (ev.Button == input::mouse::button::Left && _isMouseOver) {
        _buttonDown = false;

        litehtml::position::vector redraw {};

        rect_i const  bound {*Bounds};
        point_i const mp {convert_screen_to_world(ev.Position)};

        if (_lhdoc->on_lbutton_up(
                static_cast<litehtml::pixel_t>(mp.X - bound.left()),
                static_cast<litehtml::pixel_t>(mp.Y - bound.top()),
                static_cast<litehtml::pixel_t>(mp.X),
                static_cast<litehtml::pixel_t>(mp.Y),
                redraw)) {
            force_redraw();
        }
    }
}

auto document::convert_screen_to_world(point_i pos) const -> point_i
{
    return point_i {_config.Window->camera().convert_screen_to_world(pos)};
}
}
