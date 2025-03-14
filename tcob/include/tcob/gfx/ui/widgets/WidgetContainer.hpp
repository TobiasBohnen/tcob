// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API widget_container : public widget {
    friend class form_base;

public:
    explicit widget_container(init const& wi);

    void update(milliseconds deltaTime) final;

    void draw(widget_painter& painter) override;
    void prepare_redraw() override;

    auto virtual find_child_at(point_f pos) -> std::shared_ptr<widget>;
    auto virtual find_child_by_name(string const& name) -> std::shared_ptr<widget>;

    auto virtual widgets() const -> std::vector<std::shared_ptr<widget>> const& = 0;
    auto widgets_by_zorder(bool reverse) const -> std::vector<std::shared_ptr<widget>>;

    template <SubmitTarget Target>
    void submit(Target& target);

protected:
    void virtual on_draw_children(widget_painter& painter) = 0;

    void on_styles_changed() override;
    void mark_redraw() override;

    auto paint_offset() const -> point_f;

private:
    bool _redraw {true};
};

}

#include "WidgetContainer.inl"
