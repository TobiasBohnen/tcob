// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/NodeGraph.hpp"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/random/Random.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void node_graph::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.NodeText.lerp(from.NodeText, to.NodeText, step);
    target.NodeSize   = helper::lerp(from.NodeSize, to.NodeSize, step);
    target.NodeRadius = helper::lerp(from.NodeRadius, to.NodeRadius, step);

    target.InputPortText.lerp(from.InputPortText, to.InputPortText, step);
    target.OutputPortText.lerp(from.OutputPortText, to.OutputPortText, step);

    target.ConnectionWidth = helper::lerp(from.ConnectionWidth, to.ConnectionWidth, step);
}

node_graph::node_graph(init const& wi)
    : widget {wi}
{
    Class("node_graph");
}

auto node_graph::create_node(node_def const& def, point_f pos) -> uid
{
    auto& node {_nodes.emplace_back(get_random_ID(), def, pos)};
    return node.ID;
}

auto node_graph::remove_node(uid node) -> bool
{
    if (!helper::erase_first(_nodes, [node](auto const& n) { return n.ID == node; })) { return false; }
    std::erase_if(_connections, [node](connection const& c) {
        return c.SrcNode == node || c.DstNode == node;
    });
    return true;
}

auto node_graph::can_connect(uid srcNode, uid srcPort, uid dstNode, uid dstPort) const -> bool
{
    auto const* src {find_node(srcNode)};
    auto const* dst {find_node(dstNode)};
    if (!src || !dst) { return false; }
    if (srcNode == dstNode) { return false; }

    auto const* out {find_port(src->Def.Outputs, srcPort)};
    auto const* in {find_port(dst->Def.Inputs, dstPort)};
    if (!out || !in) { return false; }

    return out->Type & in->Type;
}

auto node_graph::create_connection(uid srcNode, uid srcPort, uid dstNode, uid dstPort, color color) -> std::optional<uid>
{
    if (!can_connect(srcNode, srcPort, dstNode, dstPort)) { return std::nullopt; }
    auto& con {_connections.emplace_back(get_random_ID(), color, srcNode, srcPort, dstNode, dstPort)};
    return con.ID;
}

auto node_graph::remove_connection(uid connection) -> bool
{
    return helper::erase_first(_connections, [connection](auto const& c) { return c.ID == connection; });
}

void node_graph::on_draw(widget_painter& painter)
{
    rect_f const bounds {draw_base(_style, painter)};
    auto&        cv {painter.canvas()};

    f32 const rowHeight {_style.NodeSize.Height.calc(bounds.height())};
    f32 const nodeWidth {_style.NodeSize.Width.calc(bounds.width())};
    f32 const portRadius {rowHeight * 0.25f};
    f32 const nodeRadius {_style.NodeRadius.calc(rowHeight)};

    auto const getNodeRect {[&](node const& n) -> rect_f {
        usize const  rows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size())};
        size_f const size {nodeWidth, rowHeight * static_cast<f32>(rows)};
        return {{n.Position.X * bounds.width(), n.Position.Y * bounds.height()}, size};
    }};

    auto const getPortPosition {[&](node const& n, uid portID, bool isInput) -> point_f {
        rect_f const nodeRect {getNodeRect(n)};
        auto const&  ports {isInput ? n.Def.Inputs : n.Def.Outputs};
        f32 const    x {isInput ? nodeRect.left() : nodeRect.right()};
        for (usize i {0}; i < ports.size(); ++i) {
            if (ports[i].ID == portID) {
                return {x, nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            }
        }
        return nodeRect.center();
    }};

    scoped_scissor const guard {painter, this};

    // connections
    for (auto const& con : _connections) {
        auto const* src {find_node(con.SrcNode)};
        auto const* dst {find_node(con.DstNode)};
        if (!src || !dst) { continue; }

        point_f const p0 {getPortPosition(*src, con.SrcPort, false)};
        point_f const p1 {getPortPosition(*dst, con.DstPort, true)};
        f32 const     dx {std::abs(p1.X - p0.X) * 0.5f};

        cv.set_stroke_style(con.Color);
        cv.set_stroke_width(_style.ConnectionWidth.calc(bounds.width()));
        cv.begin_path();
        cv.move_to(p0);
        cv.cubic_bezier_to({p0.X + dx, p0.Y}, {p1.X - dx, p1.Y}, p1);
        cv.stroke();
    }

    // nodes
    for (auto const& n : _nodes) {
        rect_f const nodeRect {getNodeRect(n)};
        rect_f const headerRect {nodeRect.Position, {nodeWidth, rowHeight}};
        rect_f const bodyRect {{nodeRect.left(), nodeRect.top() + rowHeight},
                               {nodeWidth, nodeRect.height() - rowHeight}};

        // body
        cv.set_fill_style(n.Def.Color);
        cv.begin_path();
        cv.rounded_rect(nodeRect, nodeRadius);
        cv.fill();

        // header
        cv.set_fill_style(n.Def.HeaderColor);
        cv.begin_path();
        cv.rounded_rect_varying(headerRect, nodeRadius, nodeRadius, 0.0f, 0.0f);
        cv.fill();

        // header title
        painter.draw_text(_style.NodeText, headerRect, n.Def.Title);

        // input ports
        for (usize i {0}; i < n.Def.Inputs.size(); ++i) {
            auto const& port {n.Def.Inputs[i]};
            f32 const   y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const   rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};

            cv.set_fill_style(port.Color);
            cv.begin_path();
            cv.circle({nodeRect.left(), y}, portRadius);
            cv.fill();

            rect_f const labelRect {{nodeRect.left() + (portRadius * 2.0f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.InputPortText, labelRect, port.Name);
        }

        // output ports
        for (usize i {0}; i < n.Def.Outputs.size(); ++i) {
            auto const& port {n.Def.Outputs[i]};
            f32 const   y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const   rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};

            cv.set_fill_style(port.Color);
            cv.begin_path();
            cv.circle({nodeRect.right(), y}, portRadius);
            cv.fill();

            rect_f const labelRect {{nodeRect.left() + (nodeWidth * 0.5f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.OutputPortText, labelRect, port.Name);
        }
    }
}

void node_graph::on_update(milliseconds /* deltaTime */)
{
}

auto node_graph::find_node(uid id) const -> node const*
{
    auto it {std::ranges::find(_nodes, id, &node::ID)};
    return it != _nodes.end() ? &*it : nullptr;
}

auto node_graph::find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*
{
    auto it {std::ranges::find(ports, id, &node_port::ID)};
    return it != ports.end() ? &*it : nullptr;
}

}
