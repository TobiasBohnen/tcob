// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_container : public widget {
    friend class form_base;

public:
    void update(milliseconds deltaTime) final;

    void draw(widget_painter& painter) override;

    auto virtual find_child_at(point_i pos) -> std::shared_ptr<widget>;
    auto virtual find_child_by_name(string const& name) -> std::shared_ptr<widget>;

    auto virtual widgets() const -> std::span<std::shared_ptr<widget> const> = 0;
    auto widgets_by_zorder(bool reverse) const -> std::vector<std::shared_ptr<widget>>;

    template <SubmitTarget Target>
    void submit(Target& target);

protected:
    explicit widget_container(init const& wi);

    void virtual on_draw_children(widget_painter& painter) = 0;

    void on_prepare_redraw() override;

    void on_styles_changed() override;
    void set_redraw(bool val) override;
};

}

#include "WidgetContainer.inl"
