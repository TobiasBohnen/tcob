// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"
#include <optional>

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API draggable_widget : public widget { // TODO: convert to component
public:
    class TCOB_API style : public widget_style {
    public:
        f32 DragAlpha {0.5f};

        void static Transition(style& target, style const& left, style const& right, f64 step);
    };

    signal<drop_event const> Dropped;

    bool Draggable {false};

protected:
    explicit draggable_widget(init const& wi);

    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    auto virtual drag_origin() const -> point_f = 0;    // relative to parent
    auto drag_offset() const -> std::optional<point_f>; // relative to parent

private:
    bool    _isDragging {false};
    point_f _dragOffset {point_f::Zero};
};
}
