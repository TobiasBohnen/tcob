// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::gfx::ui {

////////////////////////////////////////////////////////////

class TCOB_API layout : public non_copyable {
public:
    using parent = std::variant<widget_container*, form*>;

    explicit layout(parent parent);
    virtual ~layout() = default;

    void update();

    void mark_dirty();

    void remove_widget(widget* widget);
    void clear();

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const&;
    auto widgets() -> std::vector<std::shared_ptr<widget>>&;

protected:
    template <std::derived_from<widget> T>
    auto add_widget(string const& name) -> std::shared_ptr<T>;

    void virtual do_layout(size_f size) = 0;

private:
    auto create_init(string const& name) const -> widget::init;

    parent                               _parent;
    std::vector<std::shared_ptr<widget>> _widgets {};
    bool                                 _isDirty {false};
};

////////////////////////////////////////////////////////////

// fixed_layout: No automatic re-layout; widgets maintain manually set bounds.
class TCOB_API fixed_layout : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(rect_f const& rect, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// flex_size_layout: Dynamically adjusts each widget's size using its flex values.
class TCOB_API flex_size_layout : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(point_f pos, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// dock_layout: Positions widgets at container edges based on dock style and shrinks available space.
class TCOB_API dock_layout : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(dock_style dock, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    std::unordered_map<widget*, dock_style> _widgetDock;
};

////////////////////////////////////////////////////////////

// grid_layout: Divides the container into a grid and scales each widget's bounds within its grid cell.
class TCOB_API grid_layout final : public layout {
public:
    using layout::layout;
    explicit grid_layout(parent parent, size_i initSize);

    template <std::derived_from<widget> T>
    auto create_widget(rect_i const& bounds, string const& name, bool growGrid = false) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    size_i                              _grid {size_i::Zero};
    std::unordered_map<widget*, rect_i> _widgetBounds;
};

////////////////////////////////////////////////////////////

// box_layout: Arranges widgets in a fixed grid defined by box dimensions, positioning each widget in its cell.
class TCOB_API box_layout final : public layout {
public:
    using layout::layout;
    explicit box_layout(parent parent, size_i boxSize);

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    size_i _box {size_i::Zero};
};

////////////////////////////////////////////////////////////

// horizontal_layout: Evenly distributes widgets horizontally across the container.
class TCOB_API horizontal_layout final : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// vertical_layout: Evenly distributes widgets vertically down the container.
class TCOB_API vertical_layout final : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// flow_layout: Lays out widgets left-to-right and wraps to a new row when exceeding container width.
class TCOB_API flow_layout final : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

}

#include "Layout.inl"
