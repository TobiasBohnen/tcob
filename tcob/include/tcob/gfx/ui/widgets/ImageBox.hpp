// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>

#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/animation/Animation.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/component/Icon.hpp"
#include "tcob/gfx/ui/component/WidgetTweener.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API image_box : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        gfx::alignments Alignment {.Horizontal = gfx::horizontal_alignment::Left, .Vertical = gfx::vertical_alignment::Top};

        f32 DragAlpha {0.5f};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit image_box(init const& wi);

    signal<drop_event const> Dropped;

    prop<icon>     Image;
    prop<fit_mode> Fit;

    bool Draggable {false};

    void start_animation(gfx::frame_animation const& ani, playback_mode mode);
    void stop_animation();

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;

    auto attributes() const -> widget_attributes override;

private:
    image_box::style _style;

    point_f                _dragStart {point_f::Zero};
    std::optional<point_f> _dragPosition {std::nullopt};

    animation_tweener _animationTween;
};
}
