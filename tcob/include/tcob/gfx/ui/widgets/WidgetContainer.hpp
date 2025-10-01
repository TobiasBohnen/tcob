// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>

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

    virtual auto find_child_at(point_i pos) -> widget*;
    virtual auto find_child_by_name(string const& name) -> widget*;

    virtual auto widgets() const -> std::span<std::unique_ptr<widget> const> = 0;

    template <SubmitTarget Target>
    void submit(Target& target);

    virtual auto scroll_offset() const -> point_f;

protected:
    explicit widget_container(init const& wi);

    virtual void on_draw_children(widget_painter& painter) = 0;

    void on_prepare_redraw() override;

    void on_styles_changed() override;
    void set_redraw(bool val) override;
};

}

#include "WidgetContainer.inl"
