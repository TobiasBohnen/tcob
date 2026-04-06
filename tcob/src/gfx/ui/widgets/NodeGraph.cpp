// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/NodeGraph.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <variant>
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
    target.ParamText.lerp(from.ParamText, to.ParamText, step);

    target.PortHoverColor      = helper::lerp(from.PortHoverColor, to.PortHoverColor, step);
    target.PortCompatibleColor = helper::lerp(from.PortCompatibleColor, to.PortCompatibleColor, step);
    target.PortAcceptColor     = helper::lerp(from.PortAcceptColor, to.PortAcceptColor, step);

    target.ParamWidgetColor = helper::lerp(from.ParamWidgetColor, to.ParamWidgetColor, step);

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

    Changed({this});
    return node.ID;
}

auto node_graph::remove_node(uid node) -> bool
{
    if (!helper::erase_first(_nodes, [node](auto const& n) { return n.ID == node; })) { return false; }
    std::erase_if(_connections, [node](connection const& c) { return c.OutputNodeID == node || c.InputNodeID == node; });

    _portPosCache.clear();
    _drag = std::nullopt;

    Changed({this});
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

    if (std::ranges::any_of(_connections, [&](connection const& c) { return c.InputNodeID == inNode && c.InputPortID == inPort; })) { return false; }

    std::function<bool(uid)> const hasPath {[&](uid from) -> bool {
        return from == outNode || std::ranges::any_of(_connections, [&](connection const& c) {
                   return c.OutputNodeID == from && hasPath(c.InputNodeID);
               });
    }};

    return !hasPath(inNode);
}

auto node_graph::create_connection(uid outNode, uid outPort, uid inNode, uid inPort) -> std::optional<uid>
{
    if (!can_connect(outNode, outPort, inNode, inPort)) { return std::nullopt; }
    auto const* colorNode {find_node(outNode)};
    auto const* colorPort {colorNode ? find_port(colorNode->Def.Outputs, outPort) : nullptr};
    auto&       con {_connections.emplace_back(get_random_ID(), colorPort ? colorPort->Color : colors::White, outNode, outPort, inNode, inPort)};
    Changed({this});
    return con.ID;
}

auto node_graph::remove_connection(uid connection) -> bool
{
    auto const retValue {helper::erase_first(_connections, [connection](auto const& c) { return c.ID == connection; })};
    Changed({this});
    return retValue;
}

auto node_graph::evaluate(uid nodeID, uid portID, node_compute_func const& fn) const -> std::vector<node_value_types>
{
    eval_cache cache;

    auto const* n {find_node(nodeID)};
    if (!n) { return {}; }

    node_value_types inputVal {0.0f};
    for (auto const& c : _connections) {
        if (c.InputNodeID != nodeID || c.InputPortID != portID) { continue; }
        inputVal = evaluate_port(c.OutputNodeID, c.OutputPortID, cache);
        break;
    }

    std::vector<node_value_types> params;
    for (auto const& p : n->Def.Parameters) {
        std::visit([&](auto const& val) { params.emplace_back(val.Value); }, p);
    }

    return fn({inputVal}, params);
}

auto node_graph::evaluate_port(uid nodeID, uid portID, eval_cache& cache) const -> node_value_types
{
    if (auto nit {cache.find(nodeID)}; nit != cache.end()) {
        if (auto pit {nit->second.find(portID)}; pit != nit->second.end()) {
            return pit->second;
        }
    }

    auto const* n {find_node(nodeID)};
    if (!n) { return 0.0f; }
    auto const* port {find_port(n->Def.Outputs, portID)};
    if (!port || !port->Compute) { return 0.0f; }

    std::vector<node_value_types> inputs(n->Def.Inputs.size());
    for (auto const& c : _connections) {
        if (c.InputNodeID != nodeID) { continue; }
        for (usize i {0}; i < n->Def.Inputs.size(); ++i) {
            if (n->Def.Inputs[i].ID != c.InputPortID) { continue; }
            inputs[i] = evaluate_port(c.OutputNodeID, c.OutputPortID, cache);
        }
    }

    std::vector<node_value_types> params;
    params.reserve(n->Def.Parameters.size());
    for (auto const& p : n->Def.Parameters) {
        std::visit([&](auto const& val) { params.emplace_back(val.Value); }, p);
    }

    auto const result {port->Compute(inputs, params)};
    auto const value {result.empty() ? node_value_types {0.0f} : result[0]};
    return cache[nodeID][portID] = value;
}

void node_graph::on_draw(widget_painter& painter)
{
    rect_f const bounds {draw_base(_style, painter)};
    auto&        cv {painter.canvas()};

    f32 const rowHeight {_style.NodeSize.Height.calc(bounds.height())};
    f32 const nodeWidth {_style.NodeSize.Width.calc(bounds.width())};
    f32 const portRadius {rowHeight * 0.25f};
    f32 const nodeRadius {_style.NodeRadius.calc(rowHeight)};
    f32 const conWidth {_style.ConnectionWidth.calc(bounds.width())};

    auto const getNodeRect {[&](node const& n) -> rect_f {
        usize const  rows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size()) + n.Def.Parameters.size()};
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

    auto const drawNode {[&](node const& n) {
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
        auto const drawPort {[&](port_key const& key, node_port const& port, point_f const& pos) {
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
            auto const&    port {n.Def.Inputs[i]};
            f32 const      y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const      rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const  pos {nodeRect.left(), y};
            port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = true};

            _portPosCache.emplace_back(key, pos);
            drawPort(key, port, pos);

            rect_f const labelRect {{nodeRect.left() + (portRadius * 2.0f), rowTop},
                                    {(nodeWidth * 0.5f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.InputPortText, labelRect, port.Name);
        }

        // output ports
        for (usize i {0}; i < n.Def.Outputs.size(); ++i) {
            auto const&    port {n.Def.Outputs[i]};
            f32 const      y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight * 0.5f)};
            f32 const      rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const  pos {nodeRect.right(), y};
            port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = false};

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

    for (auto const& n : _nodes) {
        if (_drag && &n == _drag->Node) { continue; }
        drawNode(n);
    }
    if (_drag) { drawNode(*_drag->Node); }
}

void node_graph::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const   mp {screen_to_local(*this, ev.Position)};
    rect_f const bounds {content_bounds()};
    f32 const    portRadius {_style.NodeSize.Height.calc(bounds.height()) * 0.25f};

    auto const portHit {std::ranges::find_if(_portPosCache, [&](auto const& p) { return p.second.distance_to(mp) <= portRadius; })};
    auto const newHover {portHit != _portPosCache.end() ? std::optional {*portHit} : std::nullopt};

    if (newHover != _hoveredPort) {
        _hoveredPort = newHover;
        queue_redraw();
    }

    ev.Handled = true;
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

    auto const mp {screen_to_local(*this, ev.Position)};

    // header hit -> node drag
    for (auto const& n : _nodes) {
        auto it {_headerRectCache.find(n.ID)};
        if (it == _headerRectCache.end() || !it->second.contains(mp)) { continue; }

        _drag      = {.Node = find_node(n.ID), .Offset = mp - it->second.Position};
        ev.Handled = true;
        return;
    }

    // port hit
    if (_hoveredPort) {
        auto const getPort {[&](uid nodeID, uid portID, bool isInput) -> node_port const* {
            auto const* n {find_node(nodeID)};
            return n ? find_port(isInput ? n->Def.Inputs : n->Def.Outputs, portID) : nullptr;
        }};

        auto const checkCompatibility {[&]() {
            for (auto const& [key, pos] : _portPosCache) {
                if (key.IsInput == _pendingConnection->Key.IsInput) { continue; }
                if (key.NodeID == _pendingConnection->Key.NodeID) { continue; }
                uid const srcNode {_pendingConnection->Key.IsInput ? key.NodeID : _pendingConnection->Key.NodeID};
                uid const srcPort {_pendingConnection->Key.IsInput ? key.PortID : _pendingConnection->Key.PortID};
                uid const dstNode {_pendingConnection->Key.IsInput ? _pendingConnection->Key.NodeID : key.NodeID};
                uid const dstPort {_pendingConnection->Key.IsInput ? _pendingConnection->Key.PortID : key.PortID};
                _pendingConnection->CompatibilityCache.emplace_back(key, can_connect(srcNode, srcPort, dstNode, dstPort));
            }
        }};

        auto const& key {_hoveredPort->first};
        auto const& pos {_hoveredPort->second};

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
                checkCompatibility();
                ev.Handled = true;
                return;
            }
        }

        _pendingConnection = {.Key = key, .PortColor = getPort(key.NodeID, key.PortID, key.IsInput)->Color, .StartPos = pos, .MousePos = mp};
        checkCompatibility();
        ev.Handled = true;
        return;
    }

    // parameter row hit
    rect_f const bounds {content_bounds()};
    f32 const    rowHeight {_style.NodeSize.Height.calc(bounds.height())};
    f32 const    nodeWidth {_style.NodeSize.Width.calc(bounds.width())};

    for (auto const& [keyPair, rowRect] : _paramRectCache) {
        if (!rowRect.contains(mp)) { continue; }
        auto* n {find_node(keyPair.first)};
        if (!n || keyPair.second >= n->Def.Parameters.size()) { continue; }
        auto& entry {n->Def.Parameters[keyPair.second]};

        rect_f const controlRect {{rowRect.left() + (nodeWidth * 0.5f), rowRect.top()},
                                  {nodeWidth * 0.5f, rowHeight}};
        rect_f const chevronRect {{controlRect.right() - rowHeight, controlRect.top()},
                                  {rowHeight, rowHeight}};

        if (chevronRect.contains(mp)) {
            bool const isUp {mp.Y < chevronRect.top() + (chevronRect.height() * 0.5f)};
            bool       handled {false};
            std::visit(overloaded {
                           [&](auto& val) {
                               val.Value = std::clamp(val.Value + (isUp ? val.Step : -val.Step), val.Min, val.Max);
                               handled   = true;
                           },
                           [&](node_param_bool&) { }},
                       entry);

            if (handled) {
                ev.Handled = true;
                queue_redraw();
                Changed({this});
                return;
            }
        }

        // bool toggle — click anywhere in the row
        if (std::holds_alternative<node_param_bool>(entry)) {
            std::get<node_param_bool>(entry).Value = !std::get<node_param_bool>(entry).Value;
            ev.Handled                             = true;
            queue_redraw();
            Changed({this});
            return;
        }

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
