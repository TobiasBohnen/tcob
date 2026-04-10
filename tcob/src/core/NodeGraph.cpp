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
#include <unordered_set>
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
    Changed();
    return node.ID;
}

auto node_graph::remove_node(uid nodeID) -> bool
{
    if (!helper::erase_first(_nodes, [nodeID](auto const& n) { return n.ID == nodeID; })) { return false; }
    std::erase_if(_connections, [nodeID](connection const& c) { return c.OutputNodeID == nodeID || c.InputNodeID == nodeID; });

    NodeRemoved(nodeID);
    Changed();
    return true;
}

auto node_graph::can_connect(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) const -> bool
{
    if (outNodeID == inNodeID) { return false; }

    auto const* outN {find_node(outNodeID)};
    auto const* inN {find_node(inNodeID)};
    if (!outN || !inN) { return false; }

    auto const* out {find_port(outN->Def.Outputs, outPortID)};
    auto const* in {find_port(inN->Def.Inputs, inPortID)};
    if (!out || !in) { return false; }

    // type check
    if (!(out->Type & in->Type)) { return false; }

    // cycle check
    std::vector<uid>        stack {inNodeID};
    std::unordered_set<uid> visited;

    while (!stack.empty()) {
        uid const current {stack.back()};
        stack.pop_back();

        if (current == outNodeID) { return false; }
        if (visited.contains(current)) { continue; }
        visited.insert(current);

        for (auto const& con : _connections) {
            if (con.OutputNodeID == current) {
                stack.push_back(con.InputNodeID);
            }
        }
    }

    return true;
}

auto node_graph::create_connection(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) -> std::optional<uid>
{
    if (std::ranges::any_of(_connections, [&](connection const& c) { return c.InputNodeID == inNodeID && c.InputPortID == inPortID; })) { return std::nullopt; }
    if (!can_connect(outNodeID, outPortID, inNodeID, inPortID)) { return std::nullopt; }

    auto& con {_connections.emplace_back(get_random_ID(), outNodeID, outPortID, inNodeID, inPortID)};
    ConnectionAdded(con.ID);
    Changed();
    return con.ID;
}

auto node_graph::remove_connection(uid connectionID) -> bool
{
    if (!helper::erase_first(_connections, [connectionID](auto const& c) { return c.ID == connectionID; })) { return false; }

    ConnectionRemoved(connectionID);
    Changed();
    return true;
}

auto node_graph::evaluate(uid nodeID, node_compute_func const& fn) const -> void
{
    cache       cache;
    auto const* n {find_node(nodeID)};
    if (!n) { return; }

    fn(gather_inputs(*n, cache), gather_params(*n));
}

auto node_graph::compute_node(uid nodeID, uid portID, cache& cache) const -> node_value_types
{
    if (auto nit {cache.find(nodeID)}; nit != cache.end()) {
        auto pit {nit->second.find(portID)};
        return pit != nit->second.end() ? pit->second : 0.0f;
    }

    auto const* n {find_node(nodeID)};
    if (!n || !n->Def.Compute) { return 0.0f; }

    auto const results {n->Def.Compute(gather_inputs(*n, cache), gather_params(*n))};
    for (auto const& [k, v] : results) {
        cache[nodeID][k] = v;
    }

    if (auto const pit {cache[nodeID].find(portID)}; pit != cache[nodeID].end()) {
        return pit->second;
    }
    return 0.0f;
}

auto node_graph::gather_inputs(node const& n, cache& cache) const -> std::vector<node_value_types>
{
    std::vector<node_value_types> inputs(n.Def.Inputs.size());
    for (auto const& c : _connections) {
        if (c.InputNodeID != n.ID) { continue; }
        for (usize i {0}; i < n.Def.Inputs.size(); ++i) {
            if (n.Def.Inputs[i].ID != c.InputPortID) { continue; }
            inputs[i] = compute_node(c.OutputNodeID, c.OutputPortID, cache);
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

auto node_graph::mutate_param(uid nodeID, usize paramIndex, std::function<bool(node_param_types&)> const& fn) -> bool
{
    auto it {std::ranges::find(_nodes, nodeID, &node::ID)};
    if (it == _nodes.end()) { return false; }
    if (paramIndex >= it->Def.Parameters.size()) { return false; }

    auto param {it->Def.Parameters[paramIndex]}; // COPY here
    if (!fn(param)) { return false; }

    std::visit(overloaded {
                   [](node_param_float& p) { p.Value = std::clamp(p.Value, p.Min, p.Max); },
                   [](node_param_int& p) { p.Value = std::clamp(p.Value, p.Min, p.Max); },
                   [](node_param_string& p) {
                       if (!p.Options.empty() && !std::ranges::contains(p.Options, p.Value)) {
                           p.Value = p.Options.front();
                       }
                   },
                   [](auto&) { }},
               param);

    it->Def.Parameters[paramIndex] = param;

    Changed();
    return true;
}

auto node_graph::find_node(uid nodeID) const -> node const*
{
    auto it {std::ranges::find(_nodes, nodeID, &node::ID)};
    return it != _nodes.end() ? &*it : nullptr;
}

auto node_graph::get_port_type(uid nodeID, uid portID, bool isInput) const -> u32
{
    return find_port(nodeID, portID, isInput)->Type;
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
