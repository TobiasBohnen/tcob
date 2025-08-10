// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <any>
#include <unordered_map>
#include <variant>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////
// forward declarations

class style;
class widget_style;

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
class range_slider;
class slider;
class spinner;
class terminal;
class text_box;
class toggle;

// displays
class dot_matrix_display;
class seven_segment_display;

// container
class accordion;
class glass;
class panel;
class popup;
class tab_container;

// TODO:
// class html_control;
// class video_control;
// class menu;
// class context_menu;
// class radial_menu;
// class render_view;

// base
class widget;
class vscroll_widget;
class widget_container;

////////////////////////////////////////////////////////////

class TCOB_API tab_stop {
public:
    i32  Index {0};
    bool Enabled {true};

    auto operator==(tab_stop const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API nav_map_entry {
public:
    string Left;
    string Up;
    string Right;
    string Down;

    auto operator==(nav_map_entry const& other) const -> bool = default;
};

using nav_map = std::unordered_map<string, nav_map_entry>;

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
    input::key_mod  SelectMod {input::key_mod::LeftShift};
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

enum class fit_mode : u8 {
    None,
    Contain,
    Fill,
    FitWidth,
    FitHeight
};

////////////////////////////////////////////////////////////

enum class orientation : u8 {
    Horizontal,
    Vertical
};

////////////////////////////////////////////////////////////

enum class position : u8 {
    None,
    Top,
    Bottom,
    Left,
    Right
};

////////////////////////////////////////////////////////////

using widget_attribute_types = std::variant<isize, f64, bool, string, orientation, fit_mode, point_i>;
using widget_attributes      = std::unordered_map<string, widget_attribute_types>;

////////////////////////////////////////////////////////////

template <typename Target>
concept SubmitTarget = requires(Target target, string const& name, widget_attributes const& properties) {
    { target[name] = properties };
};

////////////////////////////////////////////////////////////

class TCOB_API widget_flags {
public:
    bool Focus {false};
    bool Active {false};
    bool Hover {false};
    bool Checked {false};
    bool Disabled {false};

    auto operator==(widget_flags const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API widget_style_selectors {
public:
    string            Class;
    widget_flags      Flags;
    widget_attributes Attributes;

    auto operator==(widget_style_selectors const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API linear_gradient {
public:
    degree_f            Angle {0};
    gfx::color_gradient Colors;

    auto operator==(linear_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API radial_gradient {
public:
    length              InnerRadius {0.0f, length::type::Relative};
    length              OuterRadius {1.0f, length::type::Relative};
    size_f              Scale {size_f::One};
    gfx::color_gradient Colors;

    auto operator==(radial_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API box_gradient {
public:
    length              Radius {0.25f, length::type::Relative};
    length              Feather {0.50f, length::type::Relative};
    gfx::color_gradient Colors;

    auto operator==(box_gradient const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API nine_patch {
public:
    assets::asset_ptr<gfx::texture> Texture;
    string                          TextureRegion {"default"};
    rect_f                          UV;

    auto operator==(nine_patch const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

using ui_paint = std::variant<color, linear_gradient, radial_gradient, box_gradient, nine_patch>;

////////////////////////////////////////////////////////////

class TCOB_API icon {
public:
    assets::asset_ptr<gfx::texture> Texture;
    string                          TextureRegion {"default"};
    color                           Color {colors::White};

    auto operator==(icon const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class TCOB_API item {
public:
    utf8_string Text;
    icon        Icon {};
    std::any    UserData {};

    auto operator==(item const& other) const -> bool
    {
        return Text == other.Text && Icon == other.Icon; // FIXME
    }
};

////////////////////////////////////////////////////////////

static constexpr isize INVALID_INDEX {-1};

////////////////////////////////////////////////////////////

class widget_painter;
class form_base;
class layout;

namespace detail {
    class input_injector;
}

////////////////////////////////////////////////////////////

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

struct popup_event {
    popup*  Sender {nullptr};
    widget* Widget {nullptr};
};

struct drop_event {
    widget* Sender {nullptr};
    widget* Target {nullptr};
    point_i Position {point_i::Zero};
};

////////////////////////////////////////////////////////////

TCOB_API auto global_to_content(widget const& widget, point_i p) -> point_f;
TCOB_API auto global_to_parent(widget const& widget, point_i p) -> point_f;

TCOB_API void ui_paint_lerp(ui_paint& target, ui_paint const& left, ui_paint const& right, f64 step);

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API input_injector {
    public:
        void on_key_down(widget* widget, input::keyboard::event const& ev) const;
        void on_key_up(widget* widget, input::keyboard::event const& ev) const;

        void on_text_input(widget* widget, input::keyboard::text_input_event const& ev) const;

        void on_mouse_enter(widget* widget) const;
        void on_mouse_leave(widget* widget) const;
        void on_mouse_button_down(widget* widget, input::mouse::button_event const& ev) const;
        void on_mouse_button_up(widget* widget, input::mouse::button_event const& ev) const;
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
inline auto operator""_pct(long double val) -> ui::length
{
    return ui::length {static_cast<f32>(val / 100.0), ui::length::type::Relative};
}
inline auto operator""_pct(unsigned long long int val) -> ui::length
{
    return ui::length {static_cast<f32>(val) / 100.0f, ui::length::type::Relative};
}
inline auto operator""_px(unsigned long long int val) -> ui::length
{
    return ui::length {static_cast<f32>(val), ui::length::type::Absolute};
}
}

#include "UI.inl"
