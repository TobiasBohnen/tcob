// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>

#include "tcob/core/Interfaces.hpp"
#include "tcob/gfx/animation/Tween.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_tweener : public non_copyable { // TODO: improve or remove
public:
    explicit widget_tweener(widget& parent);

    void start(f32 toValue, milliseconds delay);

    void update(milliseconds deltaTime);

    auto current_value() const -> f32;
    auto target_value() const -> f32;
    void reset(f32 value);

private:
    void set_value(f32 value);

    widget&                            _parent;
    std::unique_ptr<linear_tween<f32>> _tween;
    f32                                _currentValue {0.0f};
    f32                                _targetValue {0.0f};
};

}
