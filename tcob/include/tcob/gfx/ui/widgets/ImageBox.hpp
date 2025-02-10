// Copyright (c) 2025 Tobias Bohnen
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
    enum class fit_mode : u8 {
        None,
        Contain,
        Fill,
        FitWidth,
        FitHeight
    };

    class TCOB_API style : public widget_style {
    public:
        alignments Alignment {horizontal_alignment::Left, vertical_alignment::Top};
    };

    explicit image_box(init const& wi);

    prop<icon>     Image;
    prop<fit_mode> Fit;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;
};
}
