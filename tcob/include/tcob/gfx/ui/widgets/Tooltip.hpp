// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API tooltip : public panel {
    friend class form;

public:
    class TCOB_API style : public panel::style {
    public:
        milliseconds Delay {0};
        milliseconds FadeIn {0};
    };

    signal<tooltip_event const> Popup;

    explicit tooltip(init const& wi);

    void force_redraw(string const& reason) override;

protected:
    void on_update(milliseconds deltaTime) override;

    void virtual on_popup(widget* top);

private:
    std::unique_ptr<linear_tween<f32>> _fadeInTween;
};
}
