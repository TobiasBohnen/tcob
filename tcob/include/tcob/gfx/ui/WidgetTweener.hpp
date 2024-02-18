// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_tweener { // TODO: improve or remove
public:
    explicit widget_tweener(widget& parent);

    void start(f32 toValue, milliseconds delay);

    void update(milliseconds deltaTime);

    auto get_value() const -> f32;
    void reset(f32 val);

private:
    widget&                                      _parent;
    std::unique_ptr<tweening::linear_tween<f32>> _tween;
    f32                                          _val {0.0f};
};

}
