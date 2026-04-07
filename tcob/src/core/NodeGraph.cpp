// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/NodeGraph.hpp"

#include <algorithm>
#include <functional>
#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Common.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob {

auto node_graph::nodes() const -> std::span<node const>
{
    return _nodes;
}
auto node_graph::connections() const -> std::span<connection const>
{
    return _connections;
}

auto node_graph::create_node(node_def const& def) -> uid
{
    auto& node {_nodes.emplace_back(get_random_ID(), def)};

    NodeAdded(node.ID);
    Dirty();
    return node.ID;
}

auto node_graph::remove_node(uid node) -> bool
{
    if (!helper::erase_first(_nodes, [node](auto const& n) { return n.ID == node; })) { return false; }
    std::erase_if(_connections, [node](connection const& c) { return c.OutputNodeID == node || c.InputNodeID == node; });

    NodeRemoved(node);
    Dirty();
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
    ConnectionAdded(con.ID);
    Dirty();
    return con.ID;
}

auto node_graph::remove_connection(uid connection) -> bool
{
    auto const retValue {helper::erase_first(_connections, [connection](auto const& c) { return c.ID == connection; })};
    ConnectionRemoved(connection);
    Dirty();
    return retValue;
}

auto node_graph::evaluate(uid nodeID, uid portID, node_compute_func const& fn) const -> node_value_types
{
    eval_cache  cache;
    auto const* n {find_node(nodeID)};
    if (!n) { return 0.0f; }

    uid outputNodeID {};
    uid outputPortID {};
    for (auto const& c : _connections) {
        if (c.InputNodeID != nodeID || c.InputPortID != portID) { continue; }
        outputNodeID = c.OutputNodeID;
        outputPortID = c.OutputPortID;
        break;
    }

    return fn({evaluate_port(outputNodeID, outputPortID, cache)}, gather_params(*n));
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

    return cache[nodeID][portID] = port->Compute(gather_inputs(nodeID, cache), gather_params(*n));
}

auto node_graph::gather_inputs(uid nodeID, eval_cache& cache) const -> std::vector<node_value_types>
{
    auto const* n {find_node(nodeID)};
    if (!n) { return {}; }
    std::vector<node_value_types> inputs(n->Def.Inputs.size());
    for (auto const& c : _connections) {
        if (c.InputNodeID != nodeID) { continue; }
        for (usize i {0}; i < n->Def.Inputs.size(); ++i) {
            if (n->Def.Inputs[i].ID != c.InputPortID) { continue; }
            inputs[i] = evaluate_port(c.OutputNodeID, c.OutputPortID, cache);
        }
    }
    return inputs;
}

auto node_graph::gather_params(node const& n) const -> std::vector<node_value_types>
{
    std::vector<node_value_types> params;
    params.reserve(n.Def.Parameters.size());
    for (auto const& p : n.Def.Parameters) {
        std::visit([&](auto const& val) { params.emplace_back(val.Value); }, p);
    }
    return params;
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

auto node_graph::find_port(uid nodeID, uid portID, bool isInput) const -> node_port const*
{
    auto const* n {find_node(nodeID)};
    return n ? find_port(isInput ? n->Def.Inputs : n->Def.Outputs, portID) : nullptr;
}

auto node_graph::find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*
{
    auto it {std::ranges::find(ports, id, &node_port::ID)};
    return it != ports.end() ? &*it : nullptr;
}

}
