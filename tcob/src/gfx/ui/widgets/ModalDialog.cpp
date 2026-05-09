// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/ModalDialog.hpp"

#include "tcob/gfx/ui/Form.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

modal_dialog::modal_dialog(init const& wi)
    : panel {wi}
{
    Class("modal_dialog");
}

void modal_dialog::open()
{
    form().push_modal(*this); // always push to top
    if (!_open) {
        _open = true;
        on_open();
    }
    queue_redraw();
}

void modal_dialog::close()
{
    if (_open) {
        form().pop_modal(*this);
        _open = false;
        on_close();
        queue_redraw();
    }
}

auto modal_dialog::is_open() const -> bool
{
    return _open;
}

}
