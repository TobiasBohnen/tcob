// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
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

struct node_port {
    uid ID {0};

    string Name;
    u32    Type {0xFFFFFFFF};

    color Color {colors::White};
};

using node_value_types = std::variant<float, int, bool>;

struct node_def {
    string                                                                             Title;
    std::vector<node_port>                                                             Inputs;
    std::vector<node_port>                                                             Outputs;
    std::function<std::vector<node_value_types>(std::vector<node_value_types> const&)> Compute;

    color HeaderColor {colors::Black};
    color Color {colors::White};
};

class TCOB_API node_graph : public widget {
public:
    class TCOB_API style : public widget_style {
    public:
        text_element NodeText;
        dimensions   NodeSize {};
        length       NodeRadius {10, length::type::Absolute};

        text_element InputPortText;
        text_element OutputPortText;

        length ConnectionWidth {3, length::type::Absolute};

        color PortHoverColor {colors::White};
        color PortCompatibleColor {colors::Blue};
        color PortAcceptColor {colors::Green};

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit node_graph(init const& wi);

    signal<widget_event const> Changed;

    auto create_node(node_def const& def, point_f pos) -> uid;
    auto remove_node(uid node) -> bool;

    auto can_connect(uid outNode, uid outPort, uid inNode, uid inPort) const -> bool;
    auto create_connection(uid outNode, uid outPort, uid inNode, uid inPort) -> std::optional<uid>;
    auto remove_connection(uid connection) -> bool;

    auto evaluate(uid nodeID) const -> std::vector<node_value_types>;

protected:
    auto evaluate(uid nodeID, std::unordered_map<uid, std::vector<node_value_types>>& cache) const -> std::vector<node_value_types>;

    void on_draw(widget_painter& painter) override;

    void on_mouse_hover(input::mouse::motion_event const& ev) override;
    void on_mouse_drag(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;

    void on_update(milliseconds deltaTime) override;

private:
    struct node {
        uid      ID {0};
        node_def Def;
        point_f  Position;
    };

    struct connection {
        uid ID {0};

        color Color {colors::White};

        uid OutputNodeID {0};
        uid OutputPortID {0};
        uid InputNodeID {0};
        uid InputPortID {0};
    };

    struct port_key {
        uid  NodeID;
        uid  PortID;
        bool IsInput;

        auto operator==(port_key const& other) const -> bool = default;
    };

    struct pending_connection {
        port_key                               Key;
        color                                  PortColor;
        point_f                                StartPos;
        point_f                                MousePos;
        std::vector<std::pair<port_key, bool>> CompatibilityCache {};
    };

    struct drag_state {
        node*   Node;
        point_f Offset;
    };

    auto find_node(uid id) -> node*;
    auto find_node(uid id) const -> node const*;

    auto find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*;

    std::unordered_map<uid, rect_f>           _headerRectCache;
    std::vector<std::pair<port_key, point_f>> _portPosCache;

    std::optional<drag_state>                   _drag;
    std::optional<std::pair<port_key, point_f>> _hoveredPort;

    std::vector<node>                 _nodes;
    std::vector<connection>           _connections;
    std::optional<pending_connection> _pendingConnection;

    node_graph::style _style;
};

}
