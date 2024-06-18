// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Rect.hpp"
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

    auto get_widgets() const -> std::vector<std::shared_ptr<widget>> const&;
    auto get_widgets() -> std::vector<std::shared_ptr<widget>>&;

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

class TCOB_API fixed_layout : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(rect_f const& rect, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

class TCOB_API flex_size_layout : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(point_f pos, string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

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

class TCOB_API hbox_layout final : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

////////////////////////////////////////////////////////////

class TCOB_API vbox_layout final : public layout {
public:
    using layout::layout;

    template <std::derived_from<widget> T>
    auto create_widget(string const& name) -> std::shared_ptr<T>;

protected:
    void do_layout(size_f size) override;
};

}

#include "Layout.inl"
