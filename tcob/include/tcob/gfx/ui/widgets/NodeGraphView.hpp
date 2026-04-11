// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/NodeGraph.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Property.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Signal.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
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

        color NodeColor {colors::White};
        color NodeHeaderColor {colors::Black};

        std::unordered_map<u32, color> PortColors;
        color                          PortHoverColor {colors::White};
        color                          PortCompatibleColor {colors::Blue};
        color                          PortAcceptColor {colors::Green};

        color ParamColor {colors::White};
        color ParamWidgetColor {colors::Black};

        dimensions MinimapSize {.Width = {0, length::type::Relative}, .Height = {0, length::type::Relative}};
        f32        MinimapAlpha {0.5f};
        color      MinimapBackgroundColor {colors::Silver};
        color      MinimapViewportColor {colors::Black};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit node_graph_view(init const& wi);

    signal<widget_event const> GraphChanged;

    prop<gfx::alignment> MinimapAlignment;

    void create_node(node const& def, point_f pos);
    auto remove_node(uid nodeID) -> bool;

    auto create_connection(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) -> std::optional<uid>;

    void evaluate(uid nodeID, node_compute_func const& fn) const;

protected:
    void on_draw(widget_painter& painter) override;
    void draw_minimap(gfx::canvas& canvas, rect_f const& bounds);

    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    struct port_key {
        uid  NodeID;
        uid  PortID;
        bool IsInput;

        auto operator==(port_key const& other) const -> bool = default;
    };
    struct port_key_hash {
        auto operator()(port_key const& k) const -> usize
        {
            return helper::hash_combine(0, k.NodeID, k.PortID, k.IsInput);
        }
    };

    auto try_drag_node(point_f mp) -> bool;
    auto try_param_hit(point_f mp) -> bool;
    auto try_start_connection(point_f mp) -> bool;
    auto try_remove_connections() -> bool;

    void finish_connection();

    auto remove_connection(uid connectionID) -> bool;
    auto get_port_color(u32 type) const -> color;

    std::unordered_map<uid, point_f> _nodePos;
    std::vector<uid>                 _nodeOrder;

    std::unordered_map<uid, rect_f> _headerRectCache;

    struct port_pos {
        port_key NodePort {};
        point_f  Pos {};

        auto operator==(port_pos const& other) const -> bool = default;
    };
    std::vector<port_pos> _portPosCache;

    struct param_rect {
        uid    NodeID {};
        usize  Index {};
        rect_f Rect {};
    };
    std::vector<param_rect> _paramRectCache;

    struct drag_state {
        uid     NodeID {};
        point_f Offset {};
    };
    std::optional<drag_state> _drag;

    struct pending_connection {
        port_key                                    Key {};
        color                                       PortColor {};
        point_f                                     StartPos {};
        point_f                                     MousePos {};
        std::unordered_set<port_key, port_key_hash> CompatibilityCache {};
    };
    std::optional<pending_connection> _pendingConnection;

    std::optional<port_pos> _hoveredPort;

    bool    _panning {false};
    point_f _pan {point_f::Zero};
    f32     _zoom {1.0f};

    node_graph_view::style _style;
    node_graph             _graph;
};

}
