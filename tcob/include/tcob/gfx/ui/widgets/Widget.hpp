// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget : public updatable {
    friend class form;
    friend class widget_container;
    friend class layout;
    friend class detail::input_injector;

public:
    ~widget() override = default;

    signal<keyboard_event const>          KeyDown;
    signal<keyboard_event const>          KeyUp;
    signal<widget_event const>            MouseEnter;
    signal<widget_event const>            MouseLeave;
    signal<mouse_button_event const>      MouseDown;
    signal<mouse_button_event const>      MouseUp;
    signal<mouse_motion_event const>      MouseHover;
    signal<mouse_motion_event const>      MouseDrag;
    signal<mouse_wheel_event const>       MouseWheel;
    signal<widget_event const>            Click;
    signal<widget_event const>            DoubleClick;
    signal<controller_button_event const> ControllerButtonDown;
    signal<controller_button_event const> ControllerButtonUp;

    signal<widget_event const> FocusGained;
    signal<widget_event const> FocusLost;

    prop<rect_f>   Bounds;
    prop<string>   Class;
    prop_fn<f32>   Alpha;
    prop<tab_stop> TabStop;

    prop<dimensions> Flex;

    std::shared_ptr<tooltip> Tooltip;

    void focus();
    auto is_focused() const -> bool;

    void show();
    void hide();
    auto is_visible() const -> bool;

    void enable();
    void disable();
    auto is_enabled() const -> bool;

    auto get_parent() const -> widget_container*;
    auto get_form() const -> form*;

    auto get_name() const -> string const&;

    template <std::derived_from<style_base> T>
    auto get_style() const -> std::shared_ptr<T>;

    auto get_global_position() const -> point_f;
    auto get_global_content_bounds() const -> rect_f;
    auto get_content_bounds() const -> rect_f;
    auto get_hit_test_bounds() const -> rect_f;
    auto virtual get_scroll_offset() const -> point_f;

    auto global_to_local(point_i p) const -> point_f;
    auto global_to_parent_local(point_i p) const -> point_f;

    void update(milliseconds deltaTime) override;
    void virtual update_style();

    void paint(widget_painter& painter);

    void virtual force_redraw(string const& reason);

    auto hit_test(point_f pos) const -> bool;

    auto virtual find_child_at(point_f pos) -> std::shared_ptr<widget>;
    auto virtual find_child_by_name(string const& name) -> std::shared_ptr<widget>;

protected:
    struct init {
        form*             Form {nullptr};
        widget_container* Parent {nullptr};
        string            Name {};
    };

    explicit widget(init const& wi);

    void virtual offset_content(rect_f& bounds, bool isHitTest) const;

    void virtual collect_widgets(std::vector<widget*>& vec);

    void virtual on_styles_changed();

    auto get_styles() const -> style_collection const&;

    template <std::derived_from<style_base> T>
    auto get_sub_style(string const& styleClass, flags flags) const -> std::shared_ptr<T>;

    void virtual on_paint(widget_painter& painter) = 0;

    void virtual on_key_down(input::keyboard::event& /* ev */) { }
    void virtual on_key_up(input::keyboard::event& /* ev */) { }

    void virtual on_text_input(input::keyboard::text_input_event& /* ev */) { }
    void virtual on_text_editing(input::keyboard::text_editing_event& /* ev */) { }

    void virtual on_mouse_enter() { }
    void virtual on_mouse_leave() { }
    void virtual on_mouse_hover(input::mouse::motion_event& /* ev */) { }
    void virtual on_mouse_drag(input::mouse::motion_event& /* ev */) { }
    void virtual on_mouse_down(input::mouse::button_event& /* ev */) { }
    void virtual on_mouse_up(input::mouse::button_event& /* ev */) { }
    void virtual on_mouse_wheel(input::mouse::wheel_event& /* ev */) { }

    void virtual on_controller_button_down(input::controller::button_event& /* ev */) { }
    void virtual on_controller_button_up(input::controller::button_event& /* ev */) { }

    void virtual on_click() { }
    void virtual on_double_click() { }

    void virtual on_focus_gained() { }
    void virtual on_focus_lost() { }

    void virtual on_bounds_changed();

    auto virtual get_attributes() const -> widget_attributes;
    auto virtual get_properties() const -> widget_attributes;
    auto virtual get_flags() -> flags;

    auto get_orientation() const -> orientation;

    void set_input_enabled(bool enable);

private:
    void do_key_down(input::keyboard::event& ev);
    void do_key_up(input::keyboard::event& ev);
    void do_text_input(input::keyboard::text_input_event& ev);
    void do_text_editing(input::keyboard::text_editing_event& ev);
    void do_mouse_enter();
    void do_mouse_leave();
    void do_mouse_hover(input::mouse::motion_event& ev);
    void do_mouse_drag(input::mouse::motion_event& ev);
    void do_mouse_down(input::mouse::button_event& ev);
    void do_mouse_up(input::mouse::button_event& ev);
    void do_mouse_wheel(input::mouse::wheel_event& ev);
    void do_controller_button_down(input::controller::button_event& ev);
    void do_controller_button_up(input::controller::button_event& ev);
    void do_click();
    void do_double_click();
    void do_focus_gained();
    void do_focus_lost();

    void activate();
    void deactivate();

    auto can_tab_stop() const -> bool;

    tcob::detail::connection_manager _connections;

    bool                        _visible {true};
    bool                        _enabled {true};
    bool                        _inputEnabled {true};
    flags                       _flags {};
    f32                         _alpha {1.0f};
    form*                       _form {nullptr};
    widget*                     _parent {nullptr};
    std::shared_ptr<style_base> _style;
    string                      _name;
};

////////////////////////////////////////////////////////////

}

#include "Widget.inl"
