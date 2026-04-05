// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/NodeGraph.hpp"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
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

    _portPosCache.clear();
    _drag = std::nullopt;

    return node.ID;
}

auto node_graph::remove_node(uid node) -> bool
{
    if (!helper::erase_first(_nodes, [node](auto const& n) { return n.ID == node; })) { return false; }
    std::erase_if(_connections, [node](connection const& c) { return c.OutputNodeID == node || c.InputNodeID == node; });

    _portPosCache.clear();
    _drag = std::nullopt;

    return true;
}

auto node_graph::can_connect(uid outNode, uid outPort, uid inNode, uid inPort) const -> bool
{
    if (outNode == inNode) { return false; }

    auto const* outN {find_node(outNode)};
    auto const* inN {find_node(inNode)};
    if (!outN || !inN) { return false; }

    auto const* out {find_port(outN->Def.Outputs, outPort)};
    auto const* in {find_port(inN->Def.Inputs, inPort)};
    if (!out || !in) { return false; }

    if (!(out->Type & in->Type)) { return false; }

    return !std::ranges::any_of(_connections, [&](connection const& c) {
        return c.InputNodeID == inNode && c.InputPortID == inPort;
    });
}

auto node_graph::create_connection(uid outNode, uid outPort, uid inNode, uid inPort) -> std::optional<uid>
{
    if (!can_connect(outNode, outPort, inNode, inPort)) { return std::nullopt; }
    auto const* colorNode {find_node(outNode)};
    auto const* colorPort {colorNode ? find_port(colorNode->Def.Outputs, outPort) : nullptr};
    auto&       con {_connections.emplace_back(get_random_ID(), colorPort ? colorPort->Color : colors::White, outNode, outPort, inNode, inPort)};
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
        auto const* out {find_node(con.OutputNodeID)};
        auto const* in {find_node(con.InputNodeID)};
        if (!out || !in) { continue; }

        point_f const p0 {getPortPosition(*out, con.OutputPortID, false)};
        point_f const p1 {getPortPosition(*in, con.InputPortID, true)};
        f32 const     dx {std::abs(p1.X - p0.X) * 0.5f};

        cv.set_stroke_style(con.Color);
        cv.set_stroke_width(_style.ConnectionWidth.calc(bounds.width()));
        cv.begin_path();
        cv.move_to(p0);
        cv.cubic_bezier_to({p0.X + dx, p0.Y}, {p1.X - dx, p1.Y}, p1);
        cv.stroke();
    }

    // pending connection
    if (_pendingConnection) {
        cv.set_stroke_style(_pendingConnection->PortColor);
        cv.set_stroke_width(_style.ConnectionWidth.calc(bounds.width()));
        cv.begin_path();
        cv.move_to(_pendingConnection->StartPos);
        cv.line_to(_pendingConnection->MousePos);
        cv.stroke();
    }

    // nodes
    _headerRectCache.clear();
    _portPosCache.clear();
    for (auto const& n : _nodes) {
        rect_f const nodeRect {getNodeRect(n)};
        rect_f const headerRect {nodeRect.Position, {nodeWidth, rowHeight}};
        _headerRectCache[n.ID] = headerRect;

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
            auto const&   port {n.Def.Inputs[i]};
            f32 const     y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const     rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const pos {nodeRect.left(), y};

            _portPosCache.emplace_back(port_key {.NodeID = n.ID, .PortID = port.ID, .IsInput = true}, pos);

            cv.set_fill_style(port.Color);
            cv.begin_path();
            cv.circle(pos, portRadius);
            cv.fill();

            rect_f const labelRect {{nodeRect.left() + (portRadius * 2.0f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.InputPortText, labelRect, port.Name);
        }

        // output ports
        for (usize i {0}; i < n.Def.Outputs.size(); ++i) {
            auto const&   port {n.Def.Outputs[i]};
            f32 const     y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const     rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const pos {nodeRect.right(), y};

            _portPosCache.emplace_back(port_key {.NodeID = n.ID, .PortID = port.ID, .IsInput = false}, pos);

            cv.set_fill_style(port.Color);
            cv.begin_path();
            cv.circle(pos, portRadius);
            cv.fill();

            rect_f const labelRect {{nodeRect.left() + (nodeWidth * 0.5f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.OutputPortText, labelRect, port.Name);
        }
    }
}

void node_graph::on_mouse_drag(input::mouse::motion_event const& ev)
{
    auto const mp {screen_to_local(*this, ev.Position)};

    if (_pendingConnection) {
        _pendingConnection->MousePos = mp;
        ev.Handled                   = true;
        queue_redraw();
        return;
    }

    if (!_drag || !_drag->Node) { return; }

    rect_f const  bounds {content_bounds()};
    point_f const newPos {mp - _drag->Offset};
    _drag->Node->Position = {newPos.X / bounds.width(), newPos.Y / bounds.height()};
    ev.Handled            = true;
    queue_redraw();
}

void node_graph::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    auto const   mp {screen_to_local(*this, ev.Position)};
    rect_f const bounds {content_bounds()};
    f32 const    portRadius {_style.NodeSize.Height.calc(bounds.height()) * 0.25f};

    auto const getPort {[&](uid nodeID, uid portID, bool isInput) -> node_port const* {
        auto const* n {find_node(nodeID)};
        return n ? find_port(isInput ? n->Def.Inputs : n->Def.Outputs, portID) : nullptr;
    }};

    auto const portHit {std::ranges::find_if(_portPosCache, [&](auto const& p) { return p.second.distance_to(mp) <= portRadius; })};
    if (portHit != _portPosCache.end()) {
        auto const& key {portHit->first};
        auto const& pos {portHit->second};

        if (key.IsInput) {
            auto const it {std::ranges::find_if(_connections, [&key](connection const& c) {
                return c.InputNodeID == key.NodeID && c.InputPortID == key.PortID;
            })};
            if (it != _connections.end()) {
                auto const    srcIt {std::ranges::find_if(_portPosCache, [&](auto const& p) {
                    return p.first.NodeID == it->OutputNodeID && p.first.PortID == it->OutputPortID && !p.first.IsInput;
                })};
                point_f const startPos {srcIt != _portPosCache.end() ? srcIt->second : pos};
                _pendingConnection = {.Key       = {.NodeID = it->OutputNodeID, .PortID = it->OutputPortID, .IsInput = false},
                                      .PortColor = getPort(it->OutputNodeID, it->OutputPortID, false)->Color,
                                      .StartPos  = startPos,
                                      .MousePos  = mp};
                remove_connection(it->ID);
                ev.Handled = true;
                return;
            }
        }

        _pendingConnection = {.Key = key, .PortColor = getPort(key.NodeID, key.PortID, key.IsInput)->Color, .StartPos = pos, .MousePos = mp};
        ev.Handled         = true;
        return;
    }

    for (auto const& n : _nodes) {
        auto it {_headerRectCache.find(n.ID)};
        if (it == _headerRectCache.end() || !it->second.contains(mp)) { continue; }

        _drag      = {.Node = find_node(n.ID), .Offset = mp - it->second.Position};
        ev.Handled = true;
        return;
    }
}

void node_graph::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    if (_pendingConnection) {
        auto const   mp {screen_to_local(*this, ev.Position)};
        rect_f const bounds {content_bounds()};
        f32 const    portRadius {_style.NodeSize.Height.calc(bounds.height()) * 0.25f};

        auto const it {std::ranges::find_if(_portPosCache, [&](auto const& p) {
            return p.first.IsInput != _pendingConnection->Key.IsInput
                && p.first.NodeID != _pendingConnection->Key.NodeID
                && p.second.distance_to(mp) <= portRadius;
        })};

        if (it != _portPosCache.end()) {
            auto const& [key, pos] {*it};
            uid const srcNode {_pendingConnection->Key.IsInput ? key.NodeID : _pendingConnection->Key.NodeID};
            uid const srcPort {_pendingConnection->Key.IsInput ? key.PortID : _pendingConnection->Key.PortID};
            uid const dstNode {_pendingConnection->Key.IsInput ? _pendingConnection->Key.NodeID : key.NodeID};
            uid const dstPort {_pendingConnection->Key.IsInput ? _pendingConnection->Key.PortID : key.PortID};
            create_connection(srcNode, srcPort, dstNode, dstPort);
        }

        _pendingConnection = std::nullopt;
        ev.Handled         = true;
        queue_redraw();
        return;
    }

    if (!_drag) { return; }

    _drag      = std::nullopt;
    ev.Handled = true;
}

void node_graph::on_update(milliseconds /* deltaTime */)
{
}

auto node_graph::find_node(uid id) -> node*
{
    auto it {std::ranges::find(_nodes, id, &node::ID)};
    return it != _nodes.end() ? &*it : nullptr;
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
