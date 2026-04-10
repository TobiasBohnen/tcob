// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/widgets/NodeGraphView.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Common.hpp"
#include "tcob/core/NodeGraph.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/input/Input.hpp"
#include "tcob/gfx/Gfx.hpp"
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

    target.ConnectionWidth = helper::lerp(from.ConnectionWidth, to.ConnectionWidth, step);

    target.NodeColor       = helper::lerp(from.NodeColor, to.NodeColor, step);
    target.NodeHeaderColor = helper::lerp(from.NodeHeaderColor, to.NodeHeaderColor, step);

    if (from.PortColors.size() == to.PortColors.size()) {
        for (auto const& [k, v] : from.PortColors) {
            if (to.PortColors.contains(k)) {
                target.PortColors[k] = helper::lerp(v, to.PortColors.at(k), step);
            }
        }
    }

    target.PortHoverColor      = helper::lerp(from.PortHoverColor, to.PortHoverColor, step);
    target.PortCompatibleColor = helper::lerp(from.PortCompatibleColor, to.PortCompatibleColor, step);
    target.PortAcceptColor     = helper::lerp(from.PortAcceptColor, to.PortAcceptColor, step);

    target.ParamColor       = helper::lerp(from.ParamColor, to.ParamColor, step);
    target.ParamWidgetColor = helper::lerp(from.ParamWidgetColor, to.ParamWidgetColor, step);

    target.MinimapSize            = helper::lerp(from.MinimapSize, to.MinimapSize, step);
    target.MinimapAlpha           = helper::lerp(from.MinimapAlpha, to.MinimapAlpha, step);
    target.MinimapBackgroundColor = helper::lerp(from.MinimapBackgroundColor, to.MinimapBackgroundColor, step);
    target.MinimapViewportColor   = helper::lerp(from.MinimapViewportColor, to.MinimapViewportColor, step);
}

node_graph_view::node_graph_view(init const& wi)
    : widget {wi}
{
    MinimapAlignment.Changed.connect([&] { queue_redraw(); });

    Class("node_graph_view");
}

auto node_graph_view::create_node(node_def const& def, point_f pos) -> uid
{
    uid const id {_graph.create_node(def)};
    _nodePos[id] = pos;
    _nodeOrder.push_back(id);
    GraphChanged({this});
    return id;
}
auto node_graph_view::remove_node(uid nodeID) -> bool
{
    if (!_graph.remove_node(nodeID)) { return false; }
    _nodePos.erase(nodeID);
    std::erase(_nodeOrder, nodeID);
    if (_drag && _drag->NodeID == nodeID) { _drag = std::nullopt; }
    _pendingConnection = std::nullopt;
    GraphChanged({this});
    return true;
}

auto node_graph_view::create_connection(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) -> std::optional<uid>
{
    auto const id {_graph.create_connection(outNodeID, outPortID, inNodeID, inPortID)};
    if (id) { GraphChanged({this}); }
    return id;
}

void node_graph_view::evaluate(uid nodeID, node_compute_func const& fn) const
{
    _graph.evaluate(nodeID, fn);
}

constexpr f32 PORT_SCALE {0.25f};
constexpr f32 PORT_HIT_SCALE {1.7f};
constexpr f32 BORDER_SCALE {0.5f};

void node_graph_view::on_draw(widget_painter& painter)
{
    rect_f const bounds {draw_base(_style, painter)};
    auto&        canvas {painter.canvas()};

    f32 const rowHeight {_style.NodeSize.Height.calc(bounds.height()) * _zoom};
    f32 const nodeWidth {_style.NodeSize.Width.calc(bounds.width()) * _zoom};
    f32 const portRadius {rowHeight * PORT_SCALE};
    f32 const nodeRadius {_style.NodeRadius.calc(rowHeight)};
    f32 const conWidth {_style.ConnectionWidth.calc(bounds.width()) * _zoom};
    f32 const borderWidth {conWidth * BORDER_SCALE};

    auto const getNodeRect {[&](node_graph::node const& n) -> rect_f {
        usize const   rows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size()) + n.Def.Parameters.size()};
        size_f const  size {nodeWidth, rowHeight * static_cast<f32>(rows)};
        point_f const nodePos {_nodePos.at(n.ID)};
        return {{(nodePos.X * bounds.width() * _zoom) + _pan.X, (nodePos.Y * bounds.height() * _zoom) + _pan.Y}, size};
    }};

    auto const getPortPosition {[&](node_graph::node const& n, uid portID, bool isInput) -> point_f {
        rect_f const nodeRect {getNodeRect(n)};
        auto const&  ports {isInput ? n.Def.Inputs : n.Def.Outputs};
        f32 const    x {isInput ? nodeRect.left() : nodeRect.right()};
        for (usize i {0}; i < ports.size(); ++i) {
            if (ports[i].ID == portID) {
                return {x, nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight / 2.0f)};
            }
        }
        return nodeRect.center();
    }};

    scoped_scissor const guard {painter, this};

    // connections
    for (auto const& con : _graph.connections()) {
        auto const* out {_graph.find_node(con.OutputNodeID)};
        auto const* in {_graph.find_node(con.InputNodeID)};
        if (!out || !in) { continue; }

        point_f const p0 {getPortPosition(*out, con.OutputPortID, false)};
        point_f const p1 {getPortPosition(*in, con.InputPortID, true)};
        f32 const     dx {std::abs(p1.X - p0.X) / 2.0f};

        canvas.set_stroke_style(get_port_color(_graph.get_port_type(con.OutputNodeID, con.OutputPortID, false)));
        canvas.set_stroke_width(conWidth);
        canvas.begin_path();
        canvas.move_to(p0);
        canvas.cubic_bezier_to({p0.X + dx, p0.Y}, {p1.X - dx, p1.Y}, p1);
        canvas.stroke();
    }

    // pending connection
    if (_pendingConnection) {
        canvas.set_stroke_style(_pendingConnection->PortColor);
        canvas.set_stroke_width(conWidth);
        canvas.begin_path();
        canvas.move_to(_pendingConnection->StartPos);
        canvas.line_to(_pendingConnection->MousePos);
        canvas.stroke();
    }

    // nodes
    _headerRectCache.clear();
    _portPosCache.clear();
    _paramRectCache.clear();

    auto const drawChevrons {[&](rect_f const& controlRect) {
        f32 const cx {controlRect.right() - (rowHeight / 2.0f)};
        f32 const cy {controlRect.top() + (rowHeight / 2.0f)};
        f32 const sz {rowHeight * 0.2f};

        canvas.set_stroke_style(_style.ParamWidgetColor);
        canvas.set_stroke_width(conWidth);

        canvas.begin_path();
        canvas.move_to({cx - sz, cy - (sz / 2.0f)});
        canvas.line_to({cx, cy - (sz * 1.5f)});
        canvas.line_to({cx + sz, cy - (sz / 2.0f)});
        canvas.stroke();

        canvas.begin_path();
        canvas.move_to({cx - sz, cy + (sz / 2.0f)});
        canvas.line_to({cx, cy + (sz * 1.5f)});
        canvas.line_to({cx + sz, cy + (sz / 2.0f)});
        canvas.stroke();
    }};

    auto const drawNode {[&](auto const& n) {
        rect_f const nodeRect {getNodeRect(n)};
        rect_f const headerRect {nodeRect.Position, {nodeWidth, rowHeight}};
        _headerRectCache[n.ID] = headerRect;

        // body
        canvas.set_fill_style(_style.NodeColor);
        canvas.begin_path();
        canvas.rounded_rect(nodeRect, nodeRadius);
        canvas.fill();
        canvas.set_stroke_style(colors::Black);
        canvas.set_stroke_width(borderWidth);
        canvas.stroke();

        // header
        canvas.set_fill_style(_style.NodeHeaderColor);
        canvas.begin_path();
        canvas.rounded_rect_varying(headerRect, nodeRadius, nodeRadius, 0.0f, 0.0f);
        canvas.fill();

        // header title
        painter.draw_text(_style.NodeText, headerRect, n.Def.Title);

        // ports
        auto const drawPort {[&](port_key const& key, node_port const& port, point_f const& pos) {
            canvas.set_fill_style(get_port_color(port.Type));
            canvas.begin_path();
            canvas.circle(pos, portRadius);
            canvas.fill();

            std::optional<color> ringColor;
            if (_pendingConnection && key != _pendingConnection->Key) {
                if (_pendingConnection->CompatibilityCache.contains(key)) {
                    ringColor = _hoveredPort && key == _hoveredPort->NodePort
                        ? _style.PortAcceptColor
                        : _style.PortCompatibleColor;
                }
            } else if (_hoveredPort && key == _hoveredPort->NodePort) {
                ringColor = _style.PortHoverColor;
            }

            if (ringColor) {
                canvas.set_stroke_style(*ringColor);
                canvas.set_stroke_width(conWidth);
                canvas.stroke();
            }
        }};
        // input ports
        for (usize i {0}; i < n.Def.Inputs.size(); ++i) {
            auto const&    port {n.Def.Inputs[i]};
            f32 const      y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight / 2.0f)};
            f32 const      rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const  pos {nodeRect.left(), y};
            port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = true};

            _portPosCache.emplace_back(key, pos);
            drawPort(key, port, pos);

            rect_f const labelRect {{nodeRect.left() + (portRadius * 2.0f), rowTop},
                                    {(nodeWidth / 2.0f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.InputPortText, labelRect, port.Name);
        }

        // output ports
        for (usize i {0}; i < n.Def.Outputs.size(); ++i) {
            auto const&    port {n.Def.Outputs[i]};
            f32 const      y {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1)) + (rowHeight / 2.0f)};
            f32 const      rowTop {nodeRect.top() + (rowHeight * static_cast<f32>(i + 1))};
            point_f const  pos {nodeRect.right(), y};
            port_key const key {.NodeID = n.ID, .PortID = port.ID, .IsInput = false};

            _portPosCache.emplace_back(key, pos);
            drawPort(key, port, pos);

            rect_f const labelRect {{nodeRect.left() + (nodeWidth / 2.0f), rowTop},
                                    {(nodeWidth / 2.0f) - (portRadius * 2.0f), rowHeight}};
            painter.draw_text(_style.OutputPortText, labelRect, port.Name);
        }

        // parameter rows
        usize const portRows {1 + std::max(n.Def.Inputs.size(), n.Def.Outputs.size())};
        for (usize i {0}; i < n.Def.Parameters.size(); ++i) {
            auto const&  entry {n.Def.Parameters[i]};
            rect_f const rowRect {{nodeRect.left(), nodeRect.top() + (rowHeight * static_cast<f32>(portRows + i))},
                                  {nodeWidth, rowHeight}};

            _paramRectCache.emplace_back(n.ID, i, rowRect);

            rect_f const labelRect {rowRect.Position, {nodeWidth, rowHeight}};
            rect_f const controlRect {{rowRect.right() - rowHeight, rowRect.top()},
                                      {rowHeight, rowHeight}};

            canvas.set_fill_style(_style.ParamColor);
            canvas.begin_path();
            if (n.Def.Parameters.size() == 1) {
                canvas.rounded_rect(rowRect, nodeRadius);
            } else if (i == 0) {
                canvas.rounded_rect_varying(rowRect, nodeRadius, nodeRadius, 0, 0);
            } else if (i < n.Def.Parameters.size() - 1) {
                canvas.rect(rowRect);
            } else {
                canvas.rounded_rect_varying(rowRect, 0, 0, nodeRadius, nodeRadius);
            }
            canvas.fill();
            canvas.set_stroke_style(colors::Black);
            canvas.set_stroke_width(borderWidth);
            canvas.stroke();

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
                        rect_f const  box {{center.X, center.Y - (size / 2.0f)}, {size, size}};
                        canvas.set_stroke_style(_style.ParamWidgetColor);
                        canvas.set_stroke_width(conWidth);
                        canvas.begin_path();
                        canvas.rect(box);
                        if (val.Value) {
                            canvas.set_fill_style(_style.ParamWidgetColor);
                            canvas.fill();
                        }
                        canvas.stroke();
                    },
                    [&](node_param_string const& val) {
                        painter.draw_text(_style.ParamText, labelRect, std::format("{}: {}", val.Name, val.Value));
                        if (!val.Options.empty()) { drawChevrons(controlRect); }
                    },
                    [&](auto const&) { }},
                entry);
        }
    }};

    for (uid id : _nodeOrder) {
        if (auto const* n {_graph.find_node(id)}) { drawNode(*n); }
    }

    draw_minimap(canvas, bounds);
}

void node_graph_view::draw_minimap(gfx::canvas& canvas, rect_f const& bounds)
{
    if (_style.MinimapAlpha == 0) { return; }
    canvas.set_global_alpha(_style.MinimapAlpha);

    f32 const mmWidth {_style.MinimapSize.Width.calc(bounds.width())};
    f32 const mmHeight {_style.MinimapSize.Height.calc(bounds.height())};
    if (mmWidth == 0 || mmHeight == 0) { return; }

    f32 const mmX {MinimapAlignment->Horizontal == gfx::horizontal_alignment::Right
                       ? bounds.right() - mmWidth
                       : MinimapAlignment->Horizontal == gfx::horizontal_alignment::Centered
                       ? bounds.left() + ((bounds.width() - mmWidth) / 2.0f)
                       : bounds.left()};
    f32 const mmY {MinimapAlignment->Vertical == gfx::vertical_alignment::Bottom
                       ? bounds.bottom() - mmHeight
                       : MinimapAlignment->Vertical == gfx::vertical_alignment::Middle
                       ? bounds.top() + ((bounds.height() - mmHeight) / 2.0f)
                       : bounds.top()};

    rect_f const mmRect {{mmX, mmY}, {mmWidth, mmHeight}};

    // background
    canvas.set_fill_style(_style.MinimapBackgroundColor);
    canvas.begin_path();
    canvas.rect(mmRect);
    canvas.fill();

    // compute world bounds of all nodes
    if (_nodeOrder.empty()) { return; }

    f32 worldMinX {std::numeric_limits<f32>::max()};
    f32 worldMinY {std::numeric_limits<f32>::max()};
    f32 worldMaxX {std::numeric_limits<f32>::lowest()};
    f32 worldMaxY {std::numeric_limits<f32>::lowest()};

    for (uid id : _nodeOrder) {
        auto const* n {_graph.find_node(id)};
        if (!n) { continue; }
        point_f const pos {_nodePos[id]};
        usize const   rows {1 + std::max(n->Def.Inputs.size(), n->Def.Outputs.size()) + n->Def.Parameters.size()};
        f32 const     w {_style.NodeSize.Width.calc(bounds.width())};
        f32 const     h {_style.NodeSize.Height.calc(bounds.height()) * static_cast<f32>(rows)};
        f32 const     nx {pos.X * bounds.width()};
        f32 const     ny {pos.Y * bounds.height()};
        worldMinX = std::min(worldMinX, nx);
        worldMinY = std::min(worldMinY, ny);
        worldMaxX = std::max(worldMaxX, nx + w);
        worldMaxY = std::max(worldMaxY, ny + h);
    }

    f32 const worldW {worldMaxX - worldMinX};
    f32 const worldH {worldMaxY - worldMinY};
    if (worldW <= 0.0f || worldH <= 0.0f) { return; }

    // map world -> minimap
    auto const toMM {[&](f32 wx, f32 wy) -> point_f {
        return {
            mmRect.left() + (((wx - worldMinX) / worldW) * mmWidth),
            mmRect.top() + (((wy - worldMinY) / worldH) * mmHeight)};
    }};
    auto const toMMRect {[&](f32 wx, f32 wy, f32 ww, f32 wh) -> rect_f {
        point_f const tl {toMM(wx, wy)};
        point_f const br {toMM(wx + ww, wy + wh)};
        return {tl, size_f {std::max(2.0f, br.X - tl.X), std::max(2.0f, br.Y - tl.Y)}};
    }};

    // connections
    for (auto const& con : _graph.connections()) {
        auto const* out {_graph.find_node(con.OutputNodeID)};
        auto const* in {_graph.find_node(con.InputNodeID)};
        if (!out || !in) { continue; }

        point_f const op {_nodePos[out->ID]};
        point_f const ip {_nodePos[in->ID]};
        point_f const p0 {toMM(op.X * bounds.width(), op.Y * bounds.height())};
        point_f const p1 {toMM(ip.X * bounds.width(), ip.Y * bounds.height())};

        canvas.set_stroke_style(get_port_color(_graph.get_port_type(con.OutputNodeID, con.OutputPortID, false)));
        canvas.set_stroke_width(1.0f);
        canvas.begin_path();
        canvas.move_to(p0);
        canvas.line_to(p1);
        canvas.stroke();
    }

    // nodes
    for (uid id : _nodeOrder) {
        auto const* n {_graph.find_node(id)};
        if (!n) { continue; }
        point_f const pos {_nodePos[id]};
        usize const   rows {1 + std::max(n->Def.Inputs.size(), n->Def.Outputs.size()) + n->Def.Parameters.size()};
        f32 const     w {_style.NodeSize.Width.calc(bounds.width())};
        f32 const     h {_style.NodeSize.Height.calc(bounds.height()) * static_cast<f32>(rows)};
        rect_f const  nr {toMMRect(pos.X * bounds.width(), pos.Y * bounds.height(), w, h)};

        canvas.set_fill_style(_style.NodeHeaderColor);
        canvas.begin_path();
        canvas.rect(nr);
        canvas.fill();
    }

    // viewport rect
    f32 const vpWorldX0 {(0.0f - _pan.X) / (bounds.width() * _zoom)};
    f32 const vpWorldY0 {(0.0f - _pan.Y) / (bounds.height() * _zoom)};
    f32 const vpWorldX1 {(bounds.width() - _pan.X) / (bounds.width() * _zoom)};
    f32 const vpWorldY1 {(bounds.height() - _pan.Y) / (bounds.height() * _zoom)};

    f32 const    vpW {(vpWorldX1 - vpWorldX0) * bounds.width()};
    f32 const    vpH {(vpWorldY1 - vpWorldY0) * bounds.height()};
    rect_f const vpRect {toMMRect(vpWorldX0 * bounds.width(), vpWorldY0 * bounds.height(), vpW, vpH)};

    canvas.set_stroke_style(_style.MinimapViewportColor);
    canvas.set_stroke_width(1.0f);
    canvas.begin_path();
    canvas.rect(vpRect);
    canvas.stroke();
}

void node_graph_view::on_mouse_hover(input::mouse::motion_event const& ev)
{
    auto const mp {screen_to_local(*this, ev.Position)};
    f32 const  portRadius {_style.NodeSize.Height.calc(content_bounds().height()) * PORT_SCALE * _zoom * PORT_HIT_SCALE};

    auto const portHit {std::ranges::find_if(_portPosCache, [&](auto const& p) { return p.Pos.distance_to(mp) <= portRadius; })};
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
        _nodePos[_drag->NodeID] = {(newPos.X - _pan.X) / (bounds.width() * _zoom), (newPos.Y - _pan.Y) / (bounds.height() * _zoom)};
        queue_redraw();
        ev.Handled = true;
        return;
    }

    if (_panning) {
        _pan.X += static_cast<f32>(ev.RelativeMotion.X);
        _pan.Y += static_cast<f32>(ev.RelativeMotion.Y);
        queue_redraw();
        ev.Handled = true;
        return;
    }
}

void node_graph_view::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == controls().PrimaryMouseButton) {
        auto const mp {screen_to_local(*this, ev.Position)};
        ev.Handled = try_drag_node(mp) || try_start_connection(mp) || try_param_hit(mp);
        if (!ev.Handled) {
            _panning   = true;
            ev.Handled = true;
        }
    } else if (ev.Button == controls().SecondaryMouseButton) {
        ev.Handled = try_remove_connections();
    }
}

void node_graph_view::on_mouse_button_up(input::mouse::button_event const& ev)
{
    if (ev.Button != controls().PrimaryMouseButton) { return; }

    _panning = false;

    if (_pendingConnection) {
        finish_connection();
        ev.Handled = true;
    } else if (_drag) {
        _drag      = std::nullopt;
        ev.Handled = true;
    }
}
void node_graph_view::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    f32 const     factor {ev.Scroll.Y > 0 ? 1.1f : 1.0f / 1.1f};
    point_f const mp {screen_to_local(*this, ev.Position)};

    _pan.X = mp.X + ((_pan.X - mp.X) * factor);
    _pan.Y = mp.Y + ((_pan.Y - mp.Y) * factor);
    _zoom *= factor;
    _zoom = std::clamp(_zoom, 0.25f, 4.0f);

    queue_redraw();
    ev.Handled = true;
}

void node_graph_view::on_update(milliseconds /* deltaTime */)
{
}

auto node_graph_view::try_drag_node(point_f mp) -> bool
{
    auto const nodes {_graph.nodes()};
    auto const it {std::ranges::find_if(nodes, [&](node_graph::node const& n) {
        auto const hit {_headerRectCache.find(n.ID)};
        return hit != _headerRectCache.end() && hit->second.contains(mp);
    })};
    if (it == nodes.end()) { return false; }

    _drag = {.NodeID = it->ID, .Offset = (mp - _headerRectCache.at(it->ID).Position)};
    std::erase(_nodeOrder, it->ID);
    _nodeOrder.push_back(it->ID);
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
            if (_graph.can_connect(srcNode, srcPort, dstNode, dstPort)) {
                _pendingConnection->CompatibilityCache.insert(key);
            }
        }
    }};

    auto const& key {_hoveredPort->NodePort};
    auto const& pos {_hoveredPort->Pos};

    if (key.IsInput) {
        auto const connections {_graph.connections()};
        auto const it {std::ranges::find_if(connections, [&key](auto const& c) {
            return c.InputNodeID == key.NodeID && c.InputPortID == key.PortID;
        })};
        if (it != connections.end()) {
            auto const    srcIt {std::ranges::find_if(_portPosCache, [&](auto const& p) {
                return p.NodePort.NodeID == it->OutputNodeID && p.NodePort.PortID == it->OutputPortID && !p.NodePort.IsInput;
            })};
            point_f const startPos {srcIt != _portPosCache.end() ? srcIt->Pos : pos};
            _pendingConnection = {.Key       = {.NodeID = it->OutputNodeID, .PortID = it->OutputPortID, .IsInput = false},
                                  .PortColor = get_port_color(_graph.get_port_type(it->OutputNodeID, it->OutputPortID, false)),
                                  .StartPos  = startPos,
                                  .MousePos  = mp};
            remove_connection(it->ID);
            checkCompatibility();
            return true;
        }
    }

    _pendingConnection = {.Key       = key,
                          .PortColor = get_port_color(_graph.get_port_type(key.NodeID, key.PortID, key.IsInput)),
                          .StartPos  = pos,
                          .MousePos  = mp};
    checkCompatibility();
    return true;
}

auto node_graph_view::try_remove_connections() -> bool
{
    if (!_hoveredPort) { return false; }

    auto const& key {_hoveredPort->NodePort};
    auto const  ids {_graph.connections()
                     | std::views::filter([&](auto const& c) {
                          return key.IsInput
                              ? c.InputNodeID == key.NodeID && c.InputPortID == key.PortID
                              : c.OutputNodeID == key.NodeID && c.OutputPortID == key.PortID;
                       })
                     | std::views::transform(&node_graph::connection::ID)
                     | std::ranges::to<std::vector>()};

    if (ids.empty()) { return false; }

    for (uid id : ids) { _graph.remove_connection(id); }
    GraphChanged({this});
    return true;
}

void node_graph_view::finish_connection()
{
    if (_hoveredPort) {
        auto const& [key, pos] {*_hoveredPort};
        uid const srcNode {_pendingConnection->Key.IsInput ? key.NodeID : _pendingConnection->Key.NodeID};
        uid const srcPort {_pendingConnection->Key.IsInput ? key.PortID : _pendingConnection->Key.PortID};
        uid const dstNode {_pendingConnection->Key.IsInput ? _pendingConnection->Key.NodeID : key.NodeID};
        uid const dstPort {_pendingConnection->Key.IsInput ? _pendingConnection->Key.PortID : key.PortID};

        if (key.IsInput) { // remove exisiting connection form input node
            auto const connections {_graph.connections()};
            auto const existing {std::ranges::find_if(connections, [&](auto const& c) {
                return c.InputNodeID == key.NodeID && c.InputPortID == key.PortID;
            })};
            if (existing != connections.end() && _graph.can_connect(srcNode, srcPort, dstNode, dstPort)) {
                remove_connection(existing->ID);
            }
        }

        create_connection(srcNode, srcPort, dstNode, dstPort);
    }

    _pendingConnection = std::nullopt;
    queue_redraw();
}

auto node_graph_view::remove_connection(uid connectionID) -> bool
{
    if (_graph.remove_connection(connectionID)) {
        GraphChanged({this});
        return true;
    }

    return false;
}

auto node_graph_view::try_param_hit(point_f mp) -> bool
{
    for (auto const& [nodeID, idx, rowRect] : _paramRectCache) {
        if (!rowRect.contains(mp)) { continue; }

        auto const* cn {_graph.find_node(nodeID)};
        if (!cn || idx >= cn->Def.Parameters.size()) { continue; }

        f32 const rowHeight {rowRect.height()};

        if (_graph.mutate_param(nodeID, idx, [&](auto& entry) {
                return std::visit(
                    overloaded {
                        [&](auto& val) -> bool {
                            rect_f const chevronRect {{rowRect.right() - rowHeight, rowRect.top()}, {rowHeight, rowHeight}};
                            if (chevronRect.contains(mp)) {
                                bool const isUp {mp.Y < chevronRect.top() + (rowHeight / 2.0f)};
                                isUp ? val.Value = val.Value + val.Step : val.Value = val.Value - val.Step;
                                return true;
                            }
                            return false;
                        },
                        [](node_param_bool& val) -> bool {
                            val.Value = !val.Value;
                            return true;
                        },
                        [&](node_param_string& val) -> bool {
                            if (val.Options.empty()) { return false; }
                            rect_f const chevronRect {{rowRect.right() - rowHeight, rowRect.top()}, {rowHeight, rowHeight}};
                            if (!chevronRect.contains(mp)) { return false; }
                            bool const isUp {mp.Y < chevronRect.top() + (rowHeight / 2.0f)};
                            auto       it {std::ranges::find(val.Options, val.Value)};
                            if (isUp) {
                                val.Value = (it == val.Options.end() || std::next(it) == val.Options.end())
                                    ? val.Options.front()
                                    : *std::next(it);
                            } else {
                                val.Value = (it == val.Options.end() || it == val.Options.begin())
                                    ? val.Options.back()
                                    : *std::prev(it);
                            }
                            return true;
                        }},
                    entry);
            })) {

            _drag        = std::nullopt;
            _hoveredPort = std::nullopt;
            queue_redraw();
            GraphChanged({this});

            return true;
        }
    }
    return false;
}

auto node_graph_view::get_port_color(u32 type) const -> color { return _style.PortColors.contains(type) ? _style.PortColors.at(type) : colors::Black; }

}
