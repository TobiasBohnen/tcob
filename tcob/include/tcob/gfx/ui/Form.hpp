// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/RenderTarget.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/ShaderProgram.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API form_base : public gfx::entity {
public:
    ~form_base() override;

    prop<rect_f> Bounds;

    prop<style_collection>               Styles;
    prop<control_map>                    Controls;
    prop<nav_map>                        NavMap;
    prop<assets::asset_ptr<gfx::shader>> Shader;

    signal<string const> CursorChanged;
    point_f              TooltipOffset {point_f::Zero};

    auto name() const -> string const&;

    template <std::derived_from<tooltip> T>
    auto create_tooltip(string const& name) -> std::shared_ptr<T>;

    auto find_widget_at(point_f pos) const -> std::shared_ptr<widget>;
    auto find_widget_by_name(string const& name) const -> std::shared_ptr<widget>;
    auto top_widget() const -> widget*;
    auto all_widgets() const -> std::vector<widget*>;

    auto focused_widget() const -> widget*;
    void focus_widget(widget* newFocus);

    auto virtual containers() const -> std::vector<std::shared_ptr<widget>> const& = 0;
    void virtual remove_container(widget* widget)                                  = 0;
    void virtual clear_containers()                                                = 0;

    void request_redraw(string const& reason);
    void notify_redraw(string const& reason);

    auto focus_nav_target(string const& widget, direction dir) -> bool;

    template <SubmitTarget Target>
    void submit(Target& target);

    auto virtual get_layout() -> layout*             = 0;
    auto virtual get_layout() const -> layout const* = 0;

protected:
    form_base(string name, rect_f const& bounds);

    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(gfx::render_target& target) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;
    void on_text_editing(input::keyboard::text_editing_event const& ev) override;

    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_controller_axis_motion(input::controller::axis_event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;
    void on_controller_button_up(input::controller::button_event const& ev) override;

    void virtual on_bounds_changed();
    void on_visiblity_changed() override;

    void virtual apply_layout() = 0;

private:
    void on_mouse_hover(input::mouse::motion_event const& ev);

    auto find_next_tab_widget(std::vector<widget*> const& vec) const -> widget*;
    auto find_prev_tab_widget(std::vector<widget*> const& vec) const -> widget*;

    void on_styles_changed();

    auto can_popup_tooltip() const -> bool;
    void hide_tooltip();

    gfx::canvas          _canvas {};
    gfx::canvas_renderer _renderer;

    widget*                             _topWidget {nullptr};
    widget*                             _focusWidget {nullptr};
    detail::input_injector              _injector;
    std::vector<std::weak_ptr<tooltip>> _tooltips;

    bool _redrawWidgets {true};
    bool _clearRedraw {true};
    bool _prepareWidgets {true};

    bool         _isLButtonDown {false};
    bool         _isRButtonDown {false};
    bool         _isTooltipVisible {false};
    i32          _currentTabIndex {-1};
    point_i      _clickPos {};
    milliseconds _mouseOverTime {0};

    tcob::detail::connection_manager _connections {};

    std::unique_ptr<widget_painter> _painter {};

    string _name;
};

////////////////////////////////////////////////////////////

struct form_init {
    string Name;
    rect_i Bounds;
};

template <std::derived_from<layout> Layout = dock_layout>
class form : public form_base {
public:
    form(form_init const& init, auto&&... layoutArgs);

    template <std::derived_from<widget_container> T>
    auto create_container(auto&&... args) -> std::shared_ptr<T>;

    auto containers() const -> std::vector<std::shared_ptr<widget>> const& override;

    void remove_container(widget* widget) override;

    void clear_containers() override;

    auto get_layout() -> layout* override;
    auto get_layout() const -> layout const* override;

protected:
    void apply_layout() override;

private:
    Layout _layout;
};

}

#include "Form.inl"
