// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Panel.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API modal_dialog : public panel {
public:
    explicit modal_dialog(init const& wi);

    void open();
    void close();

    auto is_open() const -> bool;

protected:
    virtual void on_open() { }
    virtual void on_close() { }

private:
    bool _open {false};
};

}
