// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/html/HtmlDocument.hpp"

#include <memory>

#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"

#include "HtmlContainer.hpp"

#if defined(TCOB_ENABLE_ADDON_GFX_LITEHTML)

    #include <litehtml.h>
    #include <litehtml/el_space.h>
    #include <litehtml/el_text.h>

namespace tcob::gfx::html {

////////////////////////////////////////////////////////////

document::document(config c)
    : _config {std::move(c)}
    , _painter {std::make_unique<element_painter>(_canvas)}
    , _container {std::make_shared<detail::container>(*this, _config, _canvas, *_painter)}
{
    geometry::set_color(_quad, colors::White);
    geometry::set_texcoords(_quad, {render_texture::GetTexcoords(), 0});
    _renderer.set_material(_material);
    _material->Texture = _canvas.get_texture();
}

document::~document() = default;

auto document::get_mouse_position() const -> point_i
{
    return _mousePosition;
}

auto document::is_button_down() const -> bool
{
    return _buttonDown;
}

void document::from_string(string const& html)
{
    string masterCss;
    if (io::is_file(_config.MasterCSSPath)) {
        io::ifstream stream {_config.MasterCSSPath};
        masterCss = stream.read_string(stream.size_in_bytes());
    }

    _lhdoc = litehtml::document::createFromString(html.c_str(), _container.get(), masterCss.c_str());
    force_redraw();
}

auto document::load(path const& file) noexcept -> load_status
{
    if (auto fs {io::ifstream::Open(file)}) {
        from_string(io::read_as_string(file));
        return load_status::Ok;
    }

    return load_status::FileNotFound;
}

void document::change_language(string const& language, string const& culture)
{
    if (!_lhdoc) {
        return;
    }

    _container->change_language(language, culture);
    _lhdoc->lang_changed();
}

void document::force_redraw()
{
    _needsRedraw = true;
}

void document::on_update(milliseconds)
{
    if (_isTransformDirty) {
        geometry::set_position(_quad, Bounds(), get_transform());
        _isTransformDirty = false;
        _needsRedraw      = true;
    }
}

void document::on_fixed_update(milliseconds /* deltaTime */)
{
}

auto document::can_draw() const -> bool
{
    return _lhdoc != nullptr;
}

void document::on_draw_to(render_target& target)
{
    if (!_lhdoc) {
        return;
    }

    if (_needsRedraw) {
        litehtml::position::vector redraw {};
        _lhdoc->root()->find_styles_changes(redraw);

        size_i const size {Bounds->get_size()};

        _canvas.begin_frame(size, 1.0f);
        _lhdoc->render(size.Width);
        litehtml::position const pos {0, 0, size.Width, size.Height};
        _lhdoc->draw(0, 0, 0, &pos);
        _canvas.end_frame();

        _needsRedraw = false;
    }

    _renderer.set_geometry(_quad);
    _renderer.render_to_target(target);
}

void document::on_transform_dirty()
{
    _isTransformDirty = true;
}

void document::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (!_lhdoc) {
        return;
    }

    litehtml::position::vector redraw {};

    rect_i const  bound {Bounds()};
    point_i const mp {convert_screen_to_world(ev.Position)};

    if (bound.contains(mp)) {
        _mousePosition = mp - bound.top_left();
        if (_lhdoc->on_mouse_over(mp.X - bound.X, mp.Y - bound.Y, mp.X, mp.Y, redraw)) {
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

void document::on_mouse_button_down(input::mouse::button_event& ev)
{
    if (!_lhdoc) {
        return;
    }

    if (ev.Button == input::mouse::button::Left && _isMouseOver) {
        _buttonDown = true;

        litehtml::position::vector redraw {};

        rect_i const  bound {Bounds()};
        point_i const mp {convert_screen_to_world(ev.Position)};

        if (_lhdoc->on_lbutton_down(mp.X - bound.X, mp.Y - bound.Y, mp.X, mp.Y, redraw)) {
            force_redraw();
        }
    }
}

void document::on_mouse_button_up(input::mouse::button_event& ev)
{
    if (!_lhdoc) {
        return;
    }

    if (ev.Button == input::mouse::button::Left && _isMouseOver) {
        _buttonDown = false;

        litehtml::position::vector redraw {};

        rect_i const  bound {Bounds()};
        point_i const mp {convert_screen_to_world(ev.Position)};

        if (_lhdoc->on_lbutton_up(mp.X - bound.X, mp.Y - bound.Y, mp.X, mp.Y, redraw)) {
            force_redraw();
        }
    }
}

auto document::convert_screen_to_world(point_i pos) const -> point_i
{
    return point_i {(*_config.Window->Camera).convert_screen_to_world(pos)};
}
}

#endif
