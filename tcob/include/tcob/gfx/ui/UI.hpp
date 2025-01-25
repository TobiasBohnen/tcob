// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <variant>

#include "tcob/core/Rect.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx::ui {

class form;

// widgets
class button;
class canvas_widget;
class checkbox;
class color_picker;
class cycle_button;
class drop_down_list;
class grid_view;
class image_box;
class label;
class list_box;
class progress_bar;
class radio_button;
class slider;
class spinner;
class terminal;
class text_box;
class toggle;

// TODO:
// class html_control;
// class video_control;
// class menu;
// class context_menu;
// class range_slider;

// displays
class dot_matrix_display;
class seven_segment_display;

// container
class accordion;
class glass;
class panel;
class tab_container;
class tooltip;

// base
class widget;
class vscroll_widget;
class widget_container;

////////////////////////////////////////////////////////////

struct list_item {
    utf8_string Text;
    std::any    UserData;
};

////////////////////////////////////////////////////////////

struct tab_stop {
    i32  Index {0};
    bool Enabled {true};
};

////////////////////////////////////////////////////////////

struct nav_map_entry {
    string Left;
    string Up;
    string Right;
    string Down;
};

////////////////////////////////////////////////////////////

struct control_map {
    input::mouse::button PrimaryMouseButton {input::mouse::button::Left};
    input::mouse::button SecondaryMouseButton {input::mouse::button::Right};

    input::key_code ActivateKey {input::key_code::SPACE};
    input::key_code SubmitKey {input::key_code::RETURN};
    input::key_code NavLeftKey {input::key_code::LEFT};
    input::key_code NavRightKey {input::key_code::RIGHT};
    input::key_code NavUpKey {input::key_code::UP};
    input::key_code NavDownKey {input::key_code::DOWN};
    input::key_code ForwardDeleteKey {input::key_code::DEL};
    input::key_code BackwardDeleteKey {input::key_code::BACKSPACE};
    input::key_code TabKey {input::key_code::TAB};
    input::key_mod  TabMod {input::key_mod::LeftShift};
    input::key_code CutKey {input::key_code::x};
    input::key_code CopyKey {input::key_code::c};
    input::key_code PasteKey {input::key_code::v};
    input::key_mod  CutCopyPasteMod {input::key_mod::LeftControl};

    input::controller::button ActivateButton {input::controller::button::A};
    input::controller::button NavLeftButton {input::controller::button::DPadLeft};
    input::controller::button NavRightButton {input::controller::button::DPadRight};
    input::controller::button NavUpButton {input::controller::button::DPadUp};
    input::controller::button NavDownButton {input::controller::button::DPadDown};
};

////////////////////////////////////////////////////////////

class TCOB_API length final {
    friend auto operator/(length const& left, f32 right) -> length;

public:
    enum class type : u8 {
        Relative,
        Absolute
    };

    length() = default;
    length(f32 val, type t);

    f32  Value {0};
    type Type {type::Absolute};

    auto calc(f32 min, f32 refSize) const -> f32;
    auto calc(f32 refSize) const -> f32;

    auto operator==(length const& other) const -> bool = default;
    auto operator-() const -> length;

    auto static Lerp(length const& left, length const& right, f64 step) -> length;
};

auto operator/(length const& left, f32 right) -> length;

////////////////////////////////////////////////////////////

class TCOB_API thickness final {
public:
    thickness() = default;
    thickness(length all);
    thickness(length lr, length tb);
    thickness(length l, length r, length t, length b);

    length Left {};
    length Right {};
    length Top {};
    length Bottom {};

    auto operator==(thickness const& other) const -> bool = default;

    auto static Lerp(thickness const& left, thickness const& right, f64 step) -> thickness;
};

auto operator-(rect_f const& left, thickness const& right) -> rect_f;
auto operator-=(rect_f& left, thickness const& right) -> rect_f&;

////////////////////////////////////////////////////////////

class TCOB_API dimensions final {
public:
    length Width {1, length::type::Relative};
    length Height {1, length::type::Relative};

    auto operator==(dimensions const& other) const -> bool = default;

    auto static Lerp(dimensions const& left, dimensions const& right, f64 step) -> dimensions;
};

////////////////////////////////////////////////////////////

enum class dock_style : u8 {
    Left,
    Right,
    Top,
    Bottom,
    Fill
};

////////////////////////////////////////////////////////////

using widget_attribute_types = std::variant<i32, bool, string, orientation>;
using widget_attributes      = std::unordered_map<string, widget_attribute_types>;

////////////////////////////////////////////////////////////

template <typename Target>
concept SubmitTarget = requires(Target target, string const& name, widget_attributes const& properties) {
    { target[name] = properties };
};

////////////////////////////////////////////////////////////

struct widget_flags {
    bool Focus {false};
    bool Active {false};
    bool Hover {false};
    bool Checked {false};
    bool Disabled {false};

    auto operator==(widget_flags const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct linear_gradient {
    degree_f       Angle {0};
    color_gradient Colors;

    auto operator==(linear_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct radial_gradient {
    length         InnerRadius {0.0f, length::type::Relative};
    length         OuterRadius {1.0f, length::type::Relative};
    size_f         Scale {size_f::One};
    color_gradient Colors;

    auto operator==(radial_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct box_gradient {
    length         Radius {0.25f, length::type::Relative};
    length         Feather {0.50f, length::type::Relative};
    color_gradient Colors;

    auto operator==(box_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

struct nine_patch {
    assets::asset_ptr<texture> Texture;
    string                     Region {"default"};
    rect_f                     UV;

    auto operator==(nine_patch const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

using ui_paint = std::variant<color, linear_gradient, radial_gradient, box_gradient, nine_patch>;

////////////////////////////////////////////////////////////

class TCOB_API image_def final {
public:
    assets::asset_ptr<texture> Texture;
    string                     Region;

    image_def() = default;
    image_def(assets::asset_ptr<texture> texture, string region = "default");
};

////////////////////////////////////////////////////////////

class widget_painter;
class form;
class layout;

namespace detail {
    class input_injector;
}

struct widget_event {
    widget* Sender {nullptr};
};

struct keyboard_event {
    widget*                       Sender {nullptr};
    input::keyboard::event const& Event;
};

struct mouse_button_event {
    widget*                           Sender {nullptr};
    point_i                           RelativePosition {point_i::Zero};
    input::mouse::button_event const& Event;
};

struct mouse_motion_event {
    widget*                           Sender {nullptr};
    point_i                           RelativePosition {point_i::Zero};
    input::mouse::motion_event const& Event;
};

struct mouse_wheel_event {
    widget*                          Sender {nullptr};
    input::mouse::wheel_event const& Event;
};

struct controller_button_event {
    widget*                                Sender {nullptr};
    input::controller::button_event const& Event;
};

struct text_event {
    widget*     Sender {nullptr};
    utf8_string Text;
};

struct tooltip_event {
    tooltip* Sender {nullptr};
    widget*  Widget {nullptr};
};

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API input_injector {
    public:
        void on_key_down(widget* widget, input::keyboard::event const& ev) const;
        void on_key_up(widget* widget, input::keyboard::event const& ev) const;

        void on_text_input(widget* widget, input::keyboard::text_input_event const& ev) const;
        void on_text_editing(widget* widget, input::keyboard::text_editing_event const& ev) const;

        void on_mouse_enter(widget* widget) const;
        void on_mouse_leave(widget* widget) const;
        void on_mouse_down(widget* widget, input::mouse::button_event const& ev) const;
        void on_mouse_up(widget* widget, input::mouse::button_event const& ev) const;
        void on_mouse_hover(widget* widget, input::mouse::motion_event const& ev) const;
        void on_mouse_drag(widget* widget, input::mouse::motion_event const& ev) const;
        void on_mouse_wheel(widget* widget, input::mouse::wheel_event const& ev) const;

        void on_controller_button_down(widget* widget, input::controller::button_event const& ev) const;
        void on_controller_button_up(widget* widget, input::controller::button_event const& ev) const;

        void on_click(widget* widget) const;
        void on_double_click(widget* widget) const;

        void on_focus_gained(widget* widget) const;
        void on_focus_lost(widget* widget) const;

    private:
        auto check(widget* widget) const -> bool;
    };
}

}

namespace tcob::literals {
inline auto operator""_pct(long double val) -> gfx::ui::length
{
    return gfx::ui::length {static_cast<f32>(val / 100.0), gfx::ui::length::type::Relative};
}
inline auto operator""_pct(unsigned long long int val) -> gfx::ui::length
{
    return gfx::ui::length {static_cast<f32>(val) / 100.0f, gfx::ui::length::type::Relative};
}
inline auto operator""_px(unsigned long long int val) -> gfx::ui::length
{
    return gfx::ui::length {static_cast<f32>(val), gfx::ui::length::type::Absolute};
}
}

#include "UI.inl"
