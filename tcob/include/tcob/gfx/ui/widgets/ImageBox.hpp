// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API image_box : public widget {
public:
    enum class fit_mode {
        None,
        Contain,
        Fill,
        FitWidth,
        FitHeight
    };

    class TCOB_API style : public background_style {
    public:
        fit_mode   Fit {fit_mode::Fill};
        alignments Alignment {horizontal_alignment::Left, vertical_alignment::Top};
    };

    explicit image_box(init const& wi);

    prop<image_def> Image;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;
};
}
