// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/FlatMap.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Camera.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Material.hpp"
#include "tcob/gfx/Renderer.hpp"
#include "tcob/gfx/Window.hpp"
#include "tcob/gfx/drawables/Drawable.hpp"
#include "tcob/gfx/ui/Layout.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API form : public entity {
public:
    form(string name, window* window);
    form(string name, window* window, rect_f const& bounds);
    ~form() override;

    prop<rect_f> Bounds;
    prop<size_f> Scale;

    prop<style_collection>                Styles;
    prop<control_map>                     Controls;
    prop<flat_map<string, nav_map_entry>> NavMap;

    auto get_name() const -> string const&;

    template <std::derived_from<widget_container> T>
    auto create_container(dock_style dock, string const& name) -> std::shared_ptr<T>;
    template <std::derived_from<tooltip> T>
    auto create_tooltip(string const& name) -> std::shared_ptr<T>;

    auto find_widget_at(point_f pos) const -> std::shared_ptr<widget>;
    auto find_widget_by_name(string const& name) const -> std::shared_ptr<widget>;
    auto get_top_widget() const -> widget*;
    auto get_widgets() const -> std::vector<std::shared_ptr<widget>> const&;
    auto get_all_widgets() const -> std::vector<widget*>;

    auto get_focus_widget() const -> widget*;
    void focus_widget(widget* newFocus);

    void clear();

    void force_redraw(string const& reason);

    auto focus_nav_target(string const& widget, direction dir) -> bool;

    auto get_update_mode() const -> update_mode override;

    template <SubmitTarget Target>
    void submit(Target& target);

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(render_target& target) override;

    void on_key_down(input::keyboard::event& ev) override;
    void on_key_up(input::keyboard::event& ev) override;

    void on_text_input(input::keyboard::text_input_event& ev) override;
    void on_text_editing(input::keyboard::text_editing_event& ev) override;

    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_button_down(input::mouse::button_event& ev) override;
    void on_mouse_button_up(input::mouse::button_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;

    void on_controller_axis_motion(input::controller::axis_event& ev) override;
    void on_controller_button_down(input::controller::button_event& ev) override;
    void on_controller_button_up(input::controller::button_event& ev) override;

    void virtual on_bounds_changed();
    void on_visiblity_changed() override;

private:
    auto get_widgets_by_zorder() const -> std::vector<std::shared_ptr<widget>>;
    void find_top_widget(input::mouse::motion_event& ev);

    auto find_next_tab_widget(std::vector<widget*> const& vec) const -> widget*;
    auto find_prev_tab_widget(std::vector<widget*> const& vec) const -> widget*;

    void on_styles_changed(); // TODO: rename

    auto can_popup_tooltip() const -> bool;
    void hide_tooltip();

    auto scale_mouse(point_i mp) const -> point_i;

    quad_renderer _renderer {buffer_usage_hint::StaticDraw};
    canvas        _canvas {};
    camera        _camera {};

    window* _window;

    widget*                _topWidget {nullptr};
    widget*                _focusWidget {nullptr};
    detail::input_injector _injector;

    bool _redrawWidgets {true};
    bool _updateWidgets {true};

    bool         _isLButtonDown {false};
    bool         _isRButtonDown {false};
    bool         _isTooltipVisible {false};
    i32          _currentTabIndex {-1};
    point_i      _clickPos {};
    milliseconds _mouseOverTime {0};

    tcob::detail::connection_manager _connections {};
    dock_layout                      _layout;

    std::unique_ptr<widget_painter>    _painter {};
    assets::manual_asset_ptr<material> _material {};

    string _name;
};

}

#include "Form.inl"
