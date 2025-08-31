// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <memory>
#include <unordered_map>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/StyleCollection.hpp"
#include "tcob/gfx/ui/Transition.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget : public updatable {
    friend class form_base;
    friend class widget_container;
    friend class layout;
    friend class detail::input_injector;

public:
    ~widget() override = default;

    signal<keyboard_event const>          KeyDown;
    signal<keyboard_event const>          KeyUp;
    signal<widget_event const>            MouseEnter;
    signal<widget_event const>            MouseLeave;
    signal<mouse_button_event const>      MouseButtonDown;
    signal<mouse_button_event const>      MouseButtonUp;
    signal<mouse_motion_event const>      MouseHover;
    signal<mouse_motion_event const>      MouseDrag;
    signal<mouse_wheel_event const>       MouseWheel;
    signal<widget_event const>            Click;
    signal<widget_event const>            DoubleClick;
    signal<controller_button_event const> ControllerButtonDown;
    signal<controller_button_event const> ControllerButtonUp;

    signal<widget_event const> FocusGained;
    signal<widget_event const> FocusLost;

    std::any UserData;

    prop<rect_f>   Bounds;
    prop<string>   Class;
    prop_fn<f32>   Alpha;
    prop<tab_stop> TabStop;
    prop<isize>    ZOrder;

    prop<dimensions>   Flex;
    prop<milliseconds> TransitionDuration;

    string Cursor {"default"};

    std::shared_ptr<tooltip> Tooltip;

    void focus();
    auto is_focused() const -> bool;

    void show();
    void hide();
    auto is_visible() const -> bool;

    void enable();
    void disable();
    auto is_enabled() const -> bool;

    auto parent() const -> widget_container*;
    auto form() const -> form_base&;
    auto is_top_level() const -> bool;
    auto top_level_widget() -> widget*;

    auto name() const -> string const&;

    auto         form_offset() const -> point_f;
    virtual auto scroll_offset() const -> point_f;

    auto content_bounds() const -> rect_f;
    auto hit_test_bounds() const -> rect_f;

    void update(milliseconds deltaTime) override;

    virtual void draw(widget_painter& painter);

    auto hit_test(point_i pos) const -> bool;

    auto current_style() const -> widget_style const*;

protected:
    struct init {
        form_base*        Form {nullptr};
        widget_container* Parent {nullptr};
        string            Name {};
    };

    explicit widget(init const& wi);

    virtual void offset_content(rect_f& bounds, bool isHitTest) const;

    virtual void on_styles_changed();

    auto styles() const -> style_collection const&;

    template <std::derived_from<widget_style> T>
    void prepare_style(T& style);

    template <std::derived_from<style> T>
    void prepare_sub_style(T& style, isize idx, string const& styleClass, widget_flags flags);
    void reset_sub_style(isize idx, string const& styleClass, widget_flags flags);
    void clear_sub_styles();

    auto controls() const -> control_map const&;

    virtual void on_draw(widget_painter& painter) = 0;
    void         prepare_redraw();
    void         queue_redraw();

    auto draw_background(auto&& style, widget_painter& painter, bool isCircle = false) -> rect_f;

    virtual void set_redraw(bool val);
    auto         needs_redraw() const -> bool;

    virtual void on_prepare_redraw() { }

    virtual void on_key_down(input::keyboard::event const& /* ev */) { }
    virtual void on_key_up(input::keyboard::event const& /* ev */) { }

    virtual void on_text_input(input::keyboard::text_input_event const& /* ev */) { }

    virtual void on_mouse_enter() { }
    virtual void on_mouse_leave() { }
    virtual void on_mouse_hover(input::mouse::motion_event const& /* ev */) { }
    virtual void on_mouse_drag(input::mouse::motion_event const& /* ev */) { }
    virtual void on_mouse_button_down(input::mouse::button_event const& /* ev */) { }
    virtual void on_mouse_button_up(input::mouse::button_event const& /* ev */) { }
    virtual void on_mouse_wheel(input::mouse::wheel_event const& /* ev */) { }

    virtual void on_controller_button_down(input::controller::button_event const& /* ev */) { }
    virtual void on_controller_button_up(input::controller::button_event const& /* ev */) { }

    virtual void on_click() { }
    virtual void on_double_click() { }

    virtual void on_focus_gained() { }
    virtual void on_focus_lost() { }

    virtual void on_bounds_changed();

    virtual auto attributes() const -> widget_attributes;
    virtual auto flags() -> widget_flags;

    auto get_orientation() const -> orientation;

    virtual auto is_inert() const -> bool;

private:
    void do_key_down(input::keyboard::event const& ev);
    void do_key_up(input::keyboard::event const& ev);
    void do_text_input(input::keyboard::text_input_event const& ev);
    void do_mouse_enter();
    void do_mouse_leave();
    void do_mouse_hover(input::mouse::motion_event const& ev);
    void do_mouse_drag(input::mouse::motion_event const& ev);
    void do_mouse_button_down(input::mouse::button_event const& ev);
    void do_mouse_button_up(input::mouse::button_event const& ev);
    void do_mouse_wheel(input::mouse::wheel_event const& ev);
    void do_controller_button_down(input::controller::button_event const& ev);
    void do_controller_button_up(input::controller::button_event const& ev);
    void do_click();
    void do_double_click();
    void do_focus_gained();
    void do_focus_lost();

    void activate();
    void deactivate();

    auto can_tab_stop(i32 high, i32 low) const -> bool;

    bool _redraw {true};

    bool              _visible {true};
    bool              _enabled {true};
    widget_flags      _flags {};
    f32               _alpha {1.0f};
    form_base*        _form {nullptr};
    widget_container* _parent {nullptr};
    string            _name;

    widget_style_selectors _lastSelectors;
    widget_style*          _currentStyle {nullptr};

    transition<widget_style>                     _transition;
    std::unordered_map<isize, transition<style>> _subStyleTransitions;
};

////////////////////////////////////////////////////////////

}

#include "Widget.inl"
