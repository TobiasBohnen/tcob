// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/DraggableWidget.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API image_box : public draggable_widget {
public:
    class TCOB_API style : public draggable_widget::style {
    public:
        gfx::alignments Alignment {.Horizontal = gfx::horizontal_alignment::Left, .Vertical = gfx::vertical_alignment::Top};
    };

    explicit image_box(init const& wi);

    prop<icon>     Image;
    prop<fit_mode> Fit;

protected:
    void on_draw(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;
    void on_animation_step(string const& val) override;

    auto drag_origin() const -> point_f override;

    auto attributes() const -> widget_attributes override;

private:
    image_box::style _style;

    rect_f _imageRectCache;
};
}
