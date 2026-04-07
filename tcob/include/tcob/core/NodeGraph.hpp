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

#include "tcob/core/Color.hpp"
#include "tcob/core/Signal.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

using node_value_types = std::variant<f32, i32, bool>;

struct node_param_float {
    string Name;
    f32    Value;
    f32    Min {0};
    f32    Max {1};
    f32    Step {0.01f};
};

struct node_param_int {
    string Name;
    i32    Value;
    i32    Min {-100};
    i32    Max {100};
    i32    Step {1};
};

struct node_param_bool {
    string Name;
    bool   Value;
};

using node_param_types  = std::variant<node_param_float, node_param_int, node_param_bool>;
using node_compute_func = std::function<node_value_types(std::vector<node_value_types> const&, std::vector<node_value_types> const&)>;

////////////////////////////////////////////////////////////

struct node_port {
    uid ID {0};

    string Name;
    u32    Type {0xFFFFFFFF};

    color Color {colors::White};

    node_compute_func Compute;
};

struct node_def {
    string Title;

    std::vector<node_port>        Inputs;
    std::vector<node_port>        Outputs;
    std::vector<node_param_types> Parameters;

    color HeaderColor {colors::Black};
    color Color {colors::White};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API node_graph {
public:
    struct node {
        uid      ID {0};
        node_def Def;
    };

    struct connection {
        uid ID {0};

        color Color {colors::White};

        uid OutputNodeID {0};
        uid OutputPortID {0};
        uid InputNodeID {0};
        uid InputPortID {0};
    };

    signal<> Dirty;

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

    auto evaluate(uid nodeID, uid portID, node_compute_func const& fn) const -> node_value_types;

    auto find_node(uid nodeID) const -> node const*;
    auto find_node(uid nodeID) -> node*;

    auto find_port(uid nodeID, uid portID, bool isInput) const -> node_port const*;

private:
    auto find_port(std::vector<node_port> const& ports, uid id) const -> node_port const*;

    using eval_cache = std::unordered_map<uid, std::unordered_map<uid, node_value_types>>;
    auto evaluate_port(uid nodeID, uid portID, eval_cache& cache) const -> node_value_types;

    auto gather_inputs(uid nodeID, eval_cache& cache) const -> std::vector<node_value_types>;
    auto gather_params(node const& n) const -> std::vector<node_value_types>;

    std::vector<node>       _nodes;
    std::vector<connection> _connections;
};
}
