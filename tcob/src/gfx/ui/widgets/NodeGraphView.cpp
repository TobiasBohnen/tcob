// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/NodeGraphView.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/NodeGraph.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/ui/Style.hpp"
#include "tcob/gfx/ui/StyleElements.hpp"
#include "tcob/gfx/ui/UI.hpp"
#include "tcob/gfx/ui/WidgetPainter.hpp"
#include "tcob/gfx/ui/widgets/Widget.hpp"

namespace tcob::ui {

void node_graph_view::style::Transition(style& target, style const& from, style const& to, f64 step)
{
    widget_style::Transition(target, from, to, step);

    target.NodeText.lerp(from.NodeText, to.NodeText, step);
    target.NodeSize   = helper::lerp(from.NodeSize, to.NodeSize, step);
    target.NodeRadius = helper::lerp(from.NodeRadius, to.NodeRadius, step);

    target.InputPortText.lerp(from.InputPortText, to.InputPortText, step);
    target.OutputPortText.lerp(from.OutputPortText, to.OutputPortText, step);
    target.ParamText.lerp(from.ParamText, to.ParamText, step);

    target.PortHoverColor      = helper::lerp(from.PortHoverColor, to.PortHoverColor, step);
    target.PortCompatibleColor = helper::lerp(from.PortCompatibleColor, to.PortCompatibleColor, step);
    target.PortAcceptColor     = helper::lerp(from.PortAcceptColor, to.PortAcceptColor, step);

    target.ParamWidgetColor = helper::lerp(from.ParamWidgetColor, to.ParamWidgetColor, step);

    target.ConnectionWidth = helper::lerp(from.ConnectionWidth, to.ConnectionWidth, step);
}

node_graph_view::node_graph_view(init const& wi)
    : widget {wi}
{
    Graph.Changed.connect([&] { notify_dirty(); });
    Graph->Dirty.connect([&] {
        _drag              = std::nullopt;
        _hoveredPort       = std::nullopt;
        _pendingConnection = std::nullopt;
        notify_dirty();
    });
    Graph->NodeRemoved.connect([&](uid node) {
        if (_nodePos.contains(node)) {
            _nodePos.erase(node);
        }
        if (_drag && _drag->NodeID == node) {
            _drag = std::nullopt;
        }
    });

    Class("node_graph_view");
}

void node_graph_view::set_node_position(uid nodeID, point_f pos)
{
    _nodePos[nodeID] = pos;
}

void node_graph_view::on_draw(widget_painter& painter)
{
    rect_f const bounds {draw_base(_style, painter)};
    auto&        cv {painter.canvas()};

    f32 const rowHeight {_style.NodeSize.Height.calc(bounds.height())};
    f32 const nodeWidth {_style.NodeSize.Width.calc(bounds.width())};
    f32 const portRadius {rowHeight * 0.25f};
    f32 const nodeRadius {_style.NodeRadius.calc(rowHeight)};
    f32 const conWidth {_style.ConnectionWidth.calc(bounds.width())};

    auto const getNodeRect {[&](node_graph::node const& n) -> rect_f {
        usize const   rows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size()) + n.Def.Parameters.size()};
        size_f const  size {nodeWidth, rowHeight * static_cast<f32>(rows)};
        point_f const nodePos {_nodePos[n.ID]};
        return {{nodePos.X * bounds.width(), nodePos.Y * bounds.height()}, size};
    }};

    auto const getPortPosition {[&](node_graph::node const& n, uid portID, bool isInput) -> point_f {
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
    for (auto const& con : Graph->connections()) {
        auto const* out {Graph->find_node(con.OutputNodeID)};
        auto const* in {Graph->find_node(con.InputNodeID)};
        if (!out || !in) { continue; }

        point_f const p0 {getPortPosition(*out, con.OutputPortID, false)};
        point_f const p1 {getPortPosition(*in, con.InputPortID, true)};
        f32 const     dx {std::abs(p1.X - p0.X) * 0.5f};

        cv.set_stroke_style(con.Color);
        cv.set_stroke_width(conWidth);
        cv.begin_path();
        cv.move_to(p0);
        cv.cubic_bezier_to({p0.X + dx, p0.Y}, {p1.X - dx, p1.Y}, p1);
        cv.stroke();
    }

    // pending connection
    if (_pendingConnection) {
        cv.set_stroke_style(_pendingConnection->PortColor);
        cv.set_stroke_width(conWidth);
        cv.begin_path();
        cv.move_to(_pendingConnection->StartPos);
        cv.line_to(_pendingConnection->MousePos);
        cv.stroke();
    }

    // nodes
    _headerRectCache.clear();
    _portPosCache.clear();
    _paramRectCache.clear();

    auto const drawChevrons {[&](rect_f const& controlRect) {
        f32 const cx {controlRect.right() - (rowHeight * 0.5f)};
        f32 const cy {controlRect.top() + (rowHeight * 0.5f)};
        f32 const sz {rowHeight * 0.2f};

        cv.set_stroke_style(_style.ParamWidgetColor);
        cv.set_stroke_width(conWidth);

        cv.begin_path();
        cv.move_to({cx - sz, cy - (sz * 0.5f)});
        cv.line_to({cx, cy - (sz * 1.5f)});
        cv.line_to({cx + sz, cy - (sz * 0.5f)});
        cv.stroke();

        cv.begin_path();
        cv.move_to({cx - sz, cy + (sz * 0.5f)});
        cv.line_to({cx, cy + (sz * 1.5f)});
        cv.line_to({cx + sz, cy + (sz * 0.5f)});
        cv.stroke();
    }};

    auto const drawNode {[&](auto const& n) {
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

        // ports
        auto const drawPort {[&](node_port_key const& key, node_port const& port, point_f const& pos) {
            cv.set_fill_style(port.Color);
            cv.begin_path();
            cv.circle(pos, portRadius);
            cv.fill();

            std::optional<color> ringColor;
            if (_pendingConnection && key != _pendingConnection->Key) {
                auto const compatIt {std::ranges::find_if(_pendingConnection->CompatibilityCache, [&](auto const& p) { return p.first == key; })};
                if (compatIt != _pendingConnection->CompatibilityCache.end() && compatIt->second) {
                    ringColor = _hoveredPort && key == _hoveredPort->first
                        ? _style.PortAcceptColor
                        : _style.PortCompatibleColor;
                }
            } else if (_hoveredPort && key == _hoveredPort->first) {
                ringColor = _style.PortHoverColor;
            }

            if (ringColor) {
                cv.set_stroke_style(*ringColor);
                cv.set_stroke_width(conWidth);
                cv.stroke();
            }
        }};
        // input ports
        for (usize i {0}; i < n.Def.Inputs.size(); ++i) {
            auto const&         port {n.Def.Inputs[i]};
            f32 const           y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const           rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const       pos {nodeRect.left(), y};
            node_port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = true};

            _portPosCache.emplace_back(key, pos);
            drawPort(key, port, pos);

            rect_f const labelRect {{nodeRect.left() + (portRadius * 2.0f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.InputPortText, labelRect, port.Name);
        }

        // output ports
        for (usize i {0}; i < n.Def.Outputs.size(); ++i) {
            auto const&         port {n.Def.Outputs[i]};
            f32 const           y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const           rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const       pos {nodeRect.right(), y};
            node_port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = false};

            _portPosCache.emplace_back(key, pos);
            drawPort(key, port, pos);

            rect_f const labelRect {{nodeRect.left() + (nodeWidth * 0.5f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.OutputPortText, labelRect, port.Name);
        }

        // parameter rows
        usize const portRows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size())};
        for (usize i {0}; i < n.Def.Parameters.size(); ++i) {
            auto const&  entry {n.Def.Parameters[i]};
            rect_f const rowRect {{nodeRect.left(), nodeRect.top() + (rowHeight * static_cast<f32>(portRows + i))},
                                  {nodeWidth, rowHeight}};

            _paramRectCache.emplace_back(std::pair {n.ID, i}, rowRect);

            rect_f const labelRect {rowRect.Position, {nodeWidth, rowHeight}};
            rect_f const controlRect {{rowRect.right() - rowHeight, rowRect.top()},
                                      {rowHeight, rowHeight}};

            std::visit(
                overloaded {
                    [&](node_param_float const& val) {
                        painter.draw_text(_style.ParamText, labelRect, std::format("{}: {:.3f}", val.Name, val.Value));
                        drawChevrons(controlRect);
                    },
                    [&](node_param_int const& val) {
                        painter.draw_text(_style.ParamText, labelRect, std::format("{}: {}", val.Name, val.Value));
                        drawChevrons(controlRect);
                    },
                    [&](node_param_bool const& val) {
                        painter.draw_text(_style.ParamText, labelRect, std::format("{}", val.Name));
                        f32 const     size {rowHeight * 0.4f};
                        point_f const center {controlRect.center()};
                        rect_f const  box {{center.X, center.Y - (size * 0.5f)}, {size, size}};
                        cv.set_stroke_style(_style.ParamWidgetColor);
                        cv.set_stroke_width(conWidth);
                        cv.begin_path();
                        cv.rect(box);
                        if (val.Value) {
                            cv.set_fill_style(_style.ParamWidgetColor);
                            cv.fill();
                        }
                        cv.stroke();
                    },
                    [&](auto const&) { }},
                entry);
        }
    }};

    auto nodes {Graph->nodes()};
    for (auto const& n : nodes) {
        if (_drag && n.ID == _drag->NodeID) { continue; }
        drawNode(n);
    }
    if (_drag) { drawNode(*Graph->find_node(_drag->NodeID)); }
}

void node_graph_view::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const mp {screen_to_local(*this, ev.Position)};
    f32 const  portRadius {get_port_radius()};

    auto const portHit {std::ranges::find_if(_portPosCache, [&](auto const& p) { return p.second.distance_to(mp) <= portRadius; })};
    auto const newHover {portHit != _portPosCache.end() ? std::optional {*portHit} : std::nullopt};

    if (newHover != _hoveredPort) {
        _hoveredPort = newHover;
        queue_redraw();
    }

    ev.Handled = true;
}

void node_graph_view::on_mouse_drag(input::mouse::motion_event const& ev)
{
    auto const mp {screen_to_local(*this, ev.Position)};

    if (_pendingConnection) {
        _pendingConnection->MousePos = mp;
        ev.Handled                   = true;
        queue_redraw();
        return;
    }

    if (_drag) {
        rect_f const  bounds {content_bounds()};
        point_f const newPos {mp - _drag->Offset};
        _nodePos[_drag->NodeID] = {newPos.X / bounds.width(), newPos.Y / bounds.height()};
        queue_redraw();
        ev.Handled = true;
        return;
    }
}

void node_graph_view::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }
    auto const mp {screen_to_local(*this, ev.Position)};
    ev.Handled = try_drag_node(mp) || try_start_connection(mp) || try_param_hit(mp);
}

void node_graph_view::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }
    if (_pendingConnection) {
        finish_connection(screen_to_local(*this, ev.Position));
        ev.Handled = true;
    } else if (_drag) {
        _drag      = std::nullopt;
        ev.Handled = true;
    }
}

void node_graph_view::on_update(milliseconds /* deltaTime */)
{
}

auto node_graph_view::try_drag_node(point_f mp) -> bool
{
    auto const nodes {Graph->nodes()};
    auto const it {std::ranges::find_if(nodes, [&](node_graph::node const& n) {
        auto const hit {_headerRectCache.find(n.ID)};
        return hit != _headerRectCache.end() && hit->second.contains(mp);
    })};
    if (it == nodes.end()) { return false; }

    _drag = {.NodeID = it->ID, .Offset = mp - _headerRectCache.at(it->ID).Position};
    return true;
}

auto node_graph_view::try_start_connection(point_f mp) -> bool
{
    if (!_hoveredPort) { return false; }

    auto const checkCompatibility {[&]() {
        for (auto const& [key, pos] : _portPosCache) {
            if (key.IsInput == _pendingConnection->Key.IsInput) { continue; }
            if (key.NodeID == _pendingConnection->Key.NodeID) { continue; }
            uid const srcNode {_pendingConnection->Key.IsInput ? key.NodeID : _pendingConnection->Key.NodeID};
            uid const srcPort {_pendingConnection->Key.IsInput ? key.PortID : _pendingConnection->Key.PortID};
            uid const dstNode {_pendingConnection->Key.IsInput ? _pendingConnection->Key.NodeID : key.NodeID};
            uid const dstPort {_pendingConnection->Key.IsInput ? _pendingConnection->Key.PortID : key.PortID};
            _pendingConnection->CompatibilityCache.emplace_back(key, Graph->can_connect(srcNode, srcPort, dstNode, dstPort));
        }
    }};

    auto const& key {_hoveredPort->first};
    auto const& pos {_hoveredPort->second};

    if (key.IsInput) {
        auto const connections {Graph->connections()};
        auto const it {std::ranges::find_if(connections, [&key](auto const& c) {
            return c.InputNodeID == key.NodeID && c.InputPortID == key.PortID;
        })};
        if (it != connections.end()) {
            auto const    srcIt {std::ranges::find_if(_portPosCache, [&](auto const& p) {
                return p.first.NodeID == it->OutputNodeID && p.first.PortID == it->OutputPortID && !p.first.IsInput;
            })};
            point_f const startPos {srcIt != _portPosCache.end() ? srcIt->second : pos};
            _pendingConnection = {.Key       = {.NodeID = it->OutputNodeID, .PortID = it->OutputPortID, .IsInput = false},
                                  .PortColor = Graph->find_port(it->OutputNodeID, it->OutputPortID, false)->Color,
                                  .StartPos  = startPos,
                                  .MousePos  = mp};
            Graph.mutate([&](auto& graph) { graph.remove_connection(it->ID); });
            checkCompatibility();
            return true;
        }
    }

    _pendingConnection = {.Key = key, .PortColor = Graph->find_port(key.NodeID, key.PortID, key.IsInput)->Color, .StartPos = pos, .MousePos = mp};
    checkCompatibility();
    return true;
}

void node_graph_view::finish_connection(point_f mp)
{
    auto const it {std::ranges::find_if(_portPosCache, [&](auto const& p) {
        return p.first.IsInput != _pendingConnection->Key.IsInput
            && p.first.NodeID != _pendingConnection->Key.NodeID
            && p.second.distance_to(mp) <= get_port_radius();
    })};

    if (it != _portPosCache.end()) {
        auto const& [key, pos] {*it};
        uid const srcNode {_pendingConnection->Key.IsInput ? key.NodeID : _pendingConnection->Key.NodeID};
        uid const srcPort {_pendingConnection->Key.IsInput ? key.PortID : _pendingConnection->Key.PortID};
        uid const dstNode {_pendingConnection->Key.IsInput ? _pendingConnection->Key.NodeID : key.NodeID};
        uid const dstPort {_pendingConnection->Key.IsInput ? _pendingConnection->Key.PortID : key.PortID};
        Graph.mutate([&](auto& graph) { graph.create_connection(srcNode, srcPort, dstNode, dstPort); });
    }

    _pendingConnection = std::nullopt;
    queue_redraw();
}

auto node_graph_view::try_param_hit(point_f mp) -> bool
{
    rect_f const bounds {content_bounds()};
    f32 const    rowHeight {_style.NodeSize.Height.calc(bounds.height())};

    for (auto const& [keyPair, rowRect] : _paramRectCache) {
        if (!rowRect.contains(mp)) { continue; }

        auto const* cn {Graph->find_node(keyPair.first)};
        if (!cn || keyPair.second >= cn->Def.Parameters.size()) { continue; }
        rect_f const chevronRect {{rowRect.right() - rowHeight, rowRect.top()}, {rowHeight, rowHeight}};

        Graph.mutate([&](auto& graph) {
            auto* n {graph.find_node(keyPair.first)};
            auto& entry {n->Def.Parameters[keyPair.second]};
            if (chevronRect.contains(mp)) {
                bool const isUp {mp.Y < chevronRect.top() + (chevronRect.height() * 0.5f)};
                bool const handled {std::visit(
                    overloaded {
                        [&](auto& val) {
                            val.Value = std::clamp(val.Value + (isUp ? val.Step : -val.Step), val.Min, val.Max);
                            return true;
                        },
                        [&](node_param_bool&) { return false; }},
                    entry)};
                if (handled) {
                    queue_redraw();
                    return;
                }
            }

            if (std::holds_alternative<node_param_bool>(entry)) {
                std::get<node_param_bool>(entry).Value = !std::get<node_param_bool>(entry).Value;
                queue_redraw();
            }
        });

        return true;
    }
    return false;
}

auto node_graph_view::get_port_radius() const -> f32 { return _style.NodeSize.Height.calc(content_bounds().height()) * 0.25f; }

void node_graph_view::notify_dirty()
{
    queue_redraw();
    GraphDirty();
}

}
