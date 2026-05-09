// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "tcob/core/Point.hpp"
#include "tcob/core/Serialization.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/core/input/Input_Codes.hpp"
#include "tcob/gfx/Gfx.hpp"

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
class node_graph_view;
class progress_bar;
class radio_button;
class range_slider;
class slider;
class spinner;
class terminal;
class text_box;
class toggle;
class tree_view;

// displays
class dot_matrix_display;
class seven_segment_display;

// container
class accordion;
class glass;
class panel;
class modal_dialog;
class tab_container;
class toast;
class tooltip;

// TODO:
// class html_control;
// class menu;
// class context_menu;
// class radial_menu;
// class render_view;

// base
class widget;
class vscroll_widget;
class widget_container;

// utility
class widget_painter;
class form_base;
class layout;

// charting
namespace charts {
    template <typename T>
    class chart;
    template <typename T>
    class grid_chart;

    class line_chart;
    class bar_chart;
    class marimekko_chart;
    class pie_chart;
    class scatter_chart;
    class radar_chart;

    class legend;
}

////////////////////////////////////////////////////////////

class TCOB_API tab_stop {
public:
    i32  Index {0};
    bool Enabled {true};

    auto operator==(tab_stop const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&tab_stop::Index> {"index"},
            member<&tab_stop::Enabled> {"enabled"}};
    }
};

////////////////////////////////////////////////////////////

class TCOB_API nav_map_entry {
public:
    string Left;
    string Up;
    string Right;
    string Down;

    auto operator==(nav_map_entry const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&nav_map_entry::Left> {"left"},
            member<&nav_map_entry::Up> {"up"},
            member<&nav_map_entry::Right> {"right"},
            member<&nav_map_entry::Down> {"down"}};
    }
};

using nav_map = std::unordered_map<string, nav_map_entry>;

////////////////////////////////////////////////////////////

struct control_map {
    input::mouse::button PrimaryMouseButton {input::mouse::button::Left};
    input::mouse::button SecondaryMouseButton {input::mouse::button::Right};

    input::key_code ActivateKey {input::key_code::SPACE};

    std::unordered_set<input::key_code> SubmitKeys {input::key_code::RETURN, input::key_code::KP_ENTER};
    std::unordered_set<input::key_code> NavLeftKeys {input::key_code::LEFT};
    std::unordered_set<input::key_code> NavRightKeys {input::key_code::RIGHT};
    std::unordered_set<input::key_code> NavUpKeys {input::key_code::UP};
    std::unordered_set<input::key_code> NavDownKeys {input::key_code::DOWN};

    input::key_code ForwardDeleteKey {input::key_code::DEL};
    input::key_code BackwardDeleteKey {input::key_code::BACKSPACE};

    input::key_mod SelectMod {input::key_mod::LeftShift};

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
    PixelPerfect,
    Fill,
    FitWidth,
    FitHeight
};

////////////////////////////////////////////////////////////

enum class grid_select_mode : u8 {
    None,
    Cell,
    Row,
    Column
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

enum class corner : u8 {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

////////////////////////////////////////////////////////////

enum class cursor_mode : u8 {
    Default,

    W_Resize,
    E_Resize,
    N_Resize,
    S_Resize,
    SE_Resize,
    NW_Resize,
    NE_Resize,
    SW_Resize,

    Move,
    Grab,
    Grabbing,
    Text,
    Cell,

    User1,
    User2,
    User3,
    User4,
    User5,
};

////////////////////////////////////////////////////////////

using widget_attribute_types = std::variant<isize, f64, bool, string, point_i, orientation, fit_mode, grid_select_mode, gfx::alignment>;
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

static constexpr isize INVALID_INDEX {-1};

////////////////////////////////////////////////////////////

TCOB_API auto screen_to_content(widget const& widget, point_i p) -> point_f;
TCOB_API auto screen_to_local(widget const& widget, point_i p) -> point_f;
TCOB_API auto local_to_screen(widget const& widget, point_f p) -> point_f;

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
        static auto Check(widget* widget) -> bool;
    };

}

}
