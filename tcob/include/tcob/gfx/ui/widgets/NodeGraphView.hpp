// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/NodeGraph.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

class TCOB_API node_graph_view : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element NodeText;
        dimensions   NodeSize {};
        length       NodeRadius {10, length::type::Absolute};

        text_element InputPortText;
        text_element OutputPortText;
        text_element ParamText;

        length ConnectionWidth {3, length::type::Absolute};

        color PortHoverColor {colors::White};
        color PortCompatibleColor {colors::Blue};
        color PortAcceptColor {colors::Green};

        color ParamWidgetColor {colors::Black};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit node_graph_view(init const& wi);

    signal<> GraphChanged;

    auto graph() -> node_graph&;

    void set_node_position(uid nodeID, point_f pos);

protected:
    void on_draw(widget_painter& painter) override;

    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    struct node_port_key {
        uid  NodeID;
        uid  PortID;
        bool IsInput;

        auto operator==(node_port_key const& other) const -> bool = default;
    };

    struct pending_connection {
        node_port_key                               Key;
        color                                       PortColor;
        point_f                                     StartPos;
        point_f                                     MousePos;
        std::vector<std::pair<node_port_key, bool>> CompatibilityCache {};
    };

    struct drag_state {
        uid     NodeID;
        point_f Offset;
    };

    auto try_drag_node(point_f mp) -> bool;
    auto try_param_hit(point_f mp) -> bool;
    auto try_start_connection(point_f mp) -> bool;
    auto try_remove_connections() -> bool;

    void finish_connection(point_f mp);

    auto get_port_radius() const -> f32;

    void notify_dirty();

    std::unordered_map<uid, point_f> _nodePos;

    std::unordered_map<uid, rect_f>                       _headerRectCache;
    std::vector<std::pair<node_port_key, point_f>>        _portPosCache;
    std::vector<std::pair<std::pair<uid, usize>, rect_f>> _paramRectCache;

    std::optional<drag_state>                        _drag;
    std::optional<std::pair<node_port_key, point_f>> _hoveredPort;

    std::optional<pending_connection> _pendingConnection;

    node_graph_view::style _style;

    node_graph _graph;
};

}
