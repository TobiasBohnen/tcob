// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_container : public widget {
    friend class form;

public:
    explicit widget_container(init const& wi);

    void update(milliseconds deltaTime) final;
    void update_style() final;

    auto find_child_at(point_f pos) -> std::shared_ptr<widget> override;
    auto find_child_by_name(string const& name) -> std::shared_ptr<widget> override;

    auto virtual get_widgets() const -> std::vector<std::shared_ptr<widget>> const& = 0;

protected:
    void collect_widgets(std::vector<widget*>& vec) override;

    void on_styles_changed() override;

    auto get_paint_offset() const -> point_f;
};

////////////////////////////////////////////////////////////

}
