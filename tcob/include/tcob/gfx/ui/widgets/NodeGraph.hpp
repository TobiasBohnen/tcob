// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/tcob_config.hpp"

#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {
////////////////////////////////////////////////////////////

struct node_port {
    uid ID {0};

    string Name;
    color  Color {colors::White};
    u32    Type {0xFFFFFFFF};
};

struct node_def {
    string                 Title;
    std::vector<node_port> Inputs;
    std::vector<node_port> Outputs;

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

        static void Transition(style& target, style const& from, style const& to, f64 step);
    };

    explicit node_graph(init const& wi);

    auto create_node(node_def const& def, point_f pos) -> uid;
    auto remove_node(uid node) -> bool;

    auto can_connect(uid srcNode, uid srcPort, uid dstNode, uid dstPort) const -> bool;
    auto create_connection(uid srcNode, uid srcPort, uid dstNode, uid dstPort, color color) -> std::optional<uid>;
    auto remove_connection(uid connection) -> bool;

protected:
    void on_draw(widget_painter& painter) override;

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

        uid SrcNode {0};
        uid SrcPort {0};
        uid DstNode {0};
        uid DstPort {0};
    };

    auto find_node(uid id) const -> node const*;
    auto find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*;

    std::vector<node>       _nodes;
    std::vector<connection> _connections;

    node_graph::style _style;
};

}
