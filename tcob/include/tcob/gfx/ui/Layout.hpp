// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/ui/Length.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

namespace detail {
    template <typename Derived>
    class default_creator {
    public:
        template <DerivedFrom<widget> T>
        auto create_widget(string const& name, auto&&... args) -> T&;
    };
}

////////////////////////////////////////////////////////////

class TCOB_API layout : public non_copyable {
    template <typename>
    friend class detail::default_creator;

public:
    using parent = std::variant<widget_container*, form_base*>;

    virtual ~layout() = default;

    signal<> Changed;

    void apply(size_f size);

    void remove(widget const& target);
    void clear();

    auto widgets() const -> std::span<std::unique_ptr<widget> const>;

    void bring_to_front(widget& target);
    void send_to_back(widget& target);

    virtual auto allows_move() const -> bool;
    virtual auto allows_resize() const -> bool;

protected:
    explicit layout(parent parent);

    template <DerivedFrom<widget> T>
    auto add_widget(string const& name, auto&&... args) -> T&;

    virtual void do_layout(size_f size) = 0;

private:
    auto create_init(string const& name) const -> widget::init;
    void normalize_zorder();

    parent                               _parent;
    std::vector<std::unique_ptr<widget>> _widgets {};
};

////////////////////////////////////////////////////////////

// manual_layout: No automatic re-layout; widgets maintain manually set bounds.
class TCOB_API manual_layout : public layout {
public:
    explicit manual_layout(parent parent);

    template <DerivedFrom<widget> T>
    auto create_widget(rect_f const& rect, string const& name, auto&&... args) -> T&;

    auto allows_move() const -> bool override;
    auto allows_resize() const -> bool override;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// auto_size_layout: Dynamically adjusts each widget's size using its relative size values.
class TCOB_API auto_size_layout : public layout {
public:
    explicit auto_size_layout(parent parent);

    template <DerivedFrom<widget> T>
    auto create_widget(point_f pos, string const& name, auto&&... args) -> T&;

    auto allows_move() const -> bool override;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// dock_layout: Positions widgets at container edges based on dock style and shrinks available space.
class TCOB_API dock_layout : public layout {
public:
    explicit dock_layout(parent parent);

    template <DerivedFrom<widget> T>
    auto create_widget(dock_style dock, string const& name, auto&&... args) -> T&;

protected:
    void do_layout(size_f size) override;

private:
    std::unordered_map<widget*, dock_style> _widgetDock;
};

////////////////////////////////////////////////////////////

// grid_layout: Divides the container into a grid and scales each widget's bounds within its grid cell.
class TCOB_API grid_layout final : public layout {
public:
    grid_layout(parent parent, size_i initSize, bool autoGrow = false);

    template <DerivedFrom<widget> T>
    auto create_widget(rect_i const& bounds, string const& name, auto&&... args) -> T&;

protected:
    void do_layout(size_f size) override;

private:
    size_i                              _grid;
    bool                                _autoGrow;
    std::unordered_map<widget*, rect_i> _widgetBounds;
};

////////////////////////////////////////////////////////////
using weights_t = std::vector<std::vector<f32>>;

// horizontal_layout: Arranges widgets into rows with a variable number of weighted columns per row.
class TCOB_API horizontal_layout : public layout, public detail::default_creator<horizontal_layout> {
public:
    horizontal_layout(parent parent, weights_t columnWeights, gfx::vertical_alignment alignment = gfx::vertical_alignment::Top);
    horizontal_layout(parent parent, std::span<i32 const> columns, gfx::vertical_alignment alignment = gfx::vertical_alignment::Top);
    horizontal_layout(parent parent, gfx::vertical_alignment alignment = gfx::vertical_alignment::Top);

protected:
    void do_layout(size_f size) override;

private:
    weights_t               _weights;
    gfx::vertical_alignment _alignment;
    bool                    _autoWeights {false};
};

////////////////////////////////////////////////////////////

// vertical_layout: Arranges widgets into columns with a variable number of rows per column.
class TCOB_API vertical_layout : public layout, public detail::default_creator<vertical_layout> {
public:
    vertical_layout(parent parent, weights_t rowHeights, gfx::horizontal_alignment alignment = gfx::horizontal_alignment::Left);
    vertical_layout(parent parent, std::span<i32 const> rows, gfx::horizontal_alignment alignment = gfx::horizontal_alignment::Left);
    vertical_layout(parent parent, gfx::horizontal_alignment alignment = gfx::horizontal_alignment::Left);

protected:
    void do_layout(size_f size) override;

private:
    weights_t                 _weights;
    gfx::horizontal_alignment _alignment;
    bool                      _autoWeights {false};
};

////////////////////////////////////////////////////////////

// tile_layout: Arranges widgets in a fixed grid defined by box dimensions, positioning each widget in its cell.
class TCOB_API tile_layout final : public layout, public detail::default_creator<tile_layout> {
public:
    tile_layout(parent parent, size_i boxSize);

protected:
    void do_layout(size_f size) override;

private:
    size_i _box {size_i::Zero};
};

////////////////////////////////////////////////////////////

// flow_layout: Lays out widgets left-to-right and wraps to a new row when exceeding container width.
class TCOB_API flow_layout final : public layout, public detail::default_creator<flow_layout> {
public:
    explicit flow_layout(parent parent);

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

// masonry_layout: distributes widgets across a fixed number of columns
class TCOB_API masonry_layout final : public layout, public detail::default_creator<masonry_layout> {
public:
    masonry_layout(parent parent, i32 tracks, direction dir = direction::Down);

protected:
    void do_layout(size_f size) override;

private:
    i32       _tracks;
    direction _dir;
};

////////////////////////////////////////////////////////////

// tree_layout: Hierarchical tree structure layout.
class TCOB_API tree_layout final : public layout {
public:
    tree_layout(parent parent);

    template <DerivedFrom<widget> T>
    auto create_widget(i32 level, string const& name, auto&&... args) -> T&;

protected:
    void do_layout(size_f size) override;

private:
    std::unordered_map<widget*, i32> _levels;
    i32                              _maxLevel {0};
};

////////////////////////////////////////////////////////////

// stack_layout: Only active widget is visible.
class TCOB_API stack_layout final : public layout, public detail::default_creator<stack_layout> {
public:
    explicit stack_layout(parent parent);

    void activate_widget(widget* widget);

protected:
    void do_layout(size_f size) override;

private:
    widget* _active {nullptr};
};

////////////////////////////////////////////////////////////

// circle_layout: Positions widgets in a circle around the container center.
class TCOB_API circle_layout final : public layout, public detail::default_creator<circle_layout> {
public:
    explicit circle_layout(parent parent, length radius = {0.75f, length::type::Relative});

protected:
    void do_layout(size_f size) override;

private:
    length _radius;
};

////////////////////////////////////////////////////////////

// magnetic_snap_layout: No automatic re-layout; widgets snap to nearby edges.
class TCOB_API magnetic_snap_layout : public layout {
public:
    explicit magnetic_snap_layout(parent parent, f32 distance, bool snapEdges = true, bool snapSiblings = true);

    template <DerivedFrom<widget> T>
    auto create_widget(rect_f const& rect, string const& name, auto&&... args) -> T&;

    auto allows_move() const -> bool override;
    // FIXME: auto allows_resize() const -> bool override;

protected:
    void do_layout(size_f size) override;

private:
    f32  _distance {50};
    bool _snapEdges {true};
    bool _snapSiblings {true};
};

}

#include "Layout.inl"
