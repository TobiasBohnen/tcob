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
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

class TCOB_API layout : public non_copyable {
public:
    using parent = std::variant<widget_container*, form_base*>;

    virtual ~layout() = default;

    void apply();

    void remove_widget(widget* widget);
    void clear();

    auto widgets() const -> std::vector<std::shared_ptr<widget>> const&;
    auto widgets() -> std::vector<std::shared_ptr<widget>>&;

    void bring_to_front(widget* widget);
    void send_to_back(widget* widget);

    auto is_resize_allowed() const -> bool;
    auto is_move_allowed() const -> bool;

protected:
    explicit layout(parent parent, bool resizeAllowed = false, bool moveAllowed = false);

    template <std::derived_from<widget> T>
    auto add_widget(string const& name) -> std::shared_ptr<T>;

    void virtual do_layout(size_f size) = 0;

private:
    void notify_parent(); // TODO: replace with signal
    auto create_init(string const& name) const -> widget::init;
    void ensure_zorder();

    parent                               _parent;
    std::vector<std::shared_ptr<widget>> _widgets {};

    bool _resizeAllowed;
    bool _moveAllowed;
};

////////////////////////////////////////////////////////////

// static_layout: No automatic re-layout; widgets maintain manually set bounds.
class TCOB_API static_layout : public layout {
public:
    explicit static_layout(parent parent);

    template <std::derived_from<widget> T>
    auto create_widget(rect_f const& rect, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// flex_size_layout: Dynamically adjusts each widget's size using its flex values.
class TCOB_API flex_size_layout : public layout {
public:
    explicit flex_size_layout(parent parent);

    template <std::derived_from<widget> T>
    auto create_widget(point_f pos, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// dock_layout: Positions widgets at container edges based on dock style and shrinks available space.
class TCOB_API dock_layout : public layout {
public:
    explicit dock_layout(parent parent);

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
    grid_layout(parent parent, size_i initSize);

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
    box_layout(parent parent, size_i boxSize);

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
    explicit horizontal_layout(parent parent, gfx::vertical_alignment alignment = gfx::vertical_alignment::Top);

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    gfx::vertical_alignment _alignment;
};

////////////////////////////////////////////////////////////

// vertical_layout: Evenly distributes widgets vertically down the container.
class TCOB_API vertical_layout final : public layout {
public:
    explicit vertical_layout(parent parent, gfx::horizontal_alignment alignment = gfx::horizontal_alignment::Left);

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    gfx::horizontal_alignment _alignment;
};

////////////////////////////////////////////////////////////

// flow_layout: Lays out widgets left-to-right and wraps to a new row when exceeding container width.
class TCOB_API flow_layout final : public layout {
public:
    explicit flow_layout(parent parent);

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// masonry_layout: distributes widgets across a fixed number of columns
class TCOB_API masonry_layout final : public layout {
public:
    masonry_layout(parent parent, i32 columns);

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;

private:
    i32 _columns;
};

}

#include "Layout.inl"
