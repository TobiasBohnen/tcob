// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Signal.hpp"
#include "tcob/core/easing/Tween.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API tooltip : public panel {
    friend class form_base;

public:
    signal<tooltip_event const> Popup;

    explicit tooltip(init const& wi);

    milliseconds Delay {1000};
    milliseconds FadeIn {250};

protected:
    void on_update(milliseconds deltaTime) override;

    virtual void on_tooltip(widget* top);

private:
    std::unique_ptr<linear_tween<f32>> _fadeInTween;
};
}
