// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Point.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

struct tooltip_event {
    tooltip* Sender {nullptr};
    widget*  Widget {nullptr};
};

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

////////////////////////////////////////////////////////////

class TCOB_API toast : public panel {
    friend class form_base;

public:
    explicit toast(init const& wi);

    milliseconds FadeIn {250};
    milliseconds Duration {250};
    milliseconds FadeOut {250};

    auto done() const -> bool;

protected:
    void on_update(milliseconds deltaTime) override;

    virtual void on_toast();

private:
    std::unique_ptr<linear_tween<f32>> _fadeTween;
    point_i                            _slot {};
    corner                             _corner {corner::BottomRight};
    bool                               _done {false};
};
}
