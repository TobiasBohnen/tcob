// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/Interfaces.hpp"
#include "tcob/core/Signal.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <Arithmetic T>
class TCOB_API node_param_numeric {
public:
    string Name;
    T      Value {};

    T Min {0};
    T Max {1};
    T Step {1};
};

using node_param_float = node_param_numeric<f32>;
using node_param_int   = node_param_numeric<i32>;

class TCOB_API node_param_bool {
public:
    string Name;
    bool   Value;
};

class TCOB_API node_param_string {
public:
    string Name;
    string Value;

    std::vector<string> Options;
};

using node_param_types = std::variant<node_param_float, node_param_int, node_param_bool, node_param_string>;
using node_value_types = std::variant<f32, i32, bool, string>;

using node_compute_result = std::unordered_map<uid, node_value_types>;
using node_compute_func   = std::function<node_compute_result(std::vector<node_value_types> const&, std::vector<node_value_types> const&)>;

////////////////////////////////////////////////////////////

struct node_port {
    uid ID {0};

    string Name;
    u32    Type {0xFFFFFFFF};
};

struct node_def {
    string Title;

    std::vector<node_port>        Inputs;
    std::vector<node_port>        Outputs;
    std::vector<node_param_types> Parameters;

    node_compute_func Compute;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API node_graph final : public non_copyable {
public:
    struct node {
        uid      ID {0};
        node_def Def;
    };

    struct connection {
        uid ID {0};

        uid OutputNodeID {0};
        uid OutputPortID {0};
        uid InputNodeID {0};
        uid InputPortID {0};
    };

    signal<> Changed;

    signal<uid> NodeAdded;
    signal<uid> NodeRemoved;

    signal<uid> ConnectionAdded;
    signal<uid> ConnectionRemoved;

    auto nodes() const -> std::span<node const>;

    auto connections() const -> std::span<connection const>;

    auto create_node(node_def const& def) -> uid;
    auto remove_node(uid nodeID) -> bool;

    auto can_connect(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) const -> bool;
    auto create_connection(uid outNodeID, uid outPortID, uid inNodeID, uid inPortID) -> std::optional<uid>;
    auto remove_connection(uid connectionID) -> bool;

    void evaluate(uid nodeID, node_compute_func const& fn) const;

    auto mutate_param(uid nodeID, usize paramIndex, std::function<bool(node_param_types&)> const& fn) -> bool;

    auto find_node(uid nodeID) const -> node const*;
    auto find_node(uid nodeID) -> node*;

    auto find_port(uid nodeID, uid portID, bool isInput) const -> node_port const*;

private:
    auto find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*;

    using cache = std::unordered_map<uid, std::unordered_map<uid, node_value_types>>;
    auto compute_node(uid nodeID, uid portID, cache& cache) const -> node_value_types;

    auto gather_inputs(node const& n, cache& cache) const -> std::vector<node_value_types>;
    auto gather_params(node const& n) const -> std::vector<node_value_types>;

    std::vector<node>       _nodes;
    std::vector<connection> _connections;
};
}
