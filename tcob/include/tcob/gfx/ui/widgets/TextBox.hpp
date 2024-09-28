// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API text_box : public widget {
public:
    class TCOB_API style : public background_style {
    public:
        element::text  Text;
        element::caret Caret;
    };

    explicit text_box(init const& wi);

    signal<text_event>       BeforeTextInserted;
    signal<text_event const> Submit;

    prop<utf8_string> Text;
    prop<usize>       MaxLength;

protected:
    void on_paint(widget_painter& painter) override;

    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;

    void on_text_input(input::keyboard::text_input_event const& ev) override;
    void on_text_editing(input::keyboard::text_editing_event const& ev) override;

    void on_focus_gained() override;
    void on_focus_lost() override;

    auto get_attributes() const -> widget_attributes override;

    void insert_text(utf8_string const& newText);

    void on_styles_changed() override;

private:
    std::unique_ptr<tweening::square_wave_tween<bool>> _caretTween;
    usize                                              _caretPos {0};
    bool                                               _caretVisible {false};

    text_formatter::result _formatResult;
    usize                  _textLength {0};
    bool                   _textDirty {false};
};
}
