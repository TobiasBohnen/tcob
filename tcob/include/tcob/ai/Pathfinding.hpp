// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <compare>
#include <limits>
#include <set>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai::pathfinding {
////////////////////////////////////////////////////////////

template <typename T>
concept PathGrid =
    requires(T& t, point_i p) {
        { t.get_cost(p, p) } -> std::same_as<u64>;
    };

////////////////////////////////////////////////////////////

static constexpr u64 IMPASSABLE_COST {std::numeric_limits<u64>::max()};

enum class heuristic : u8 {
    Euclidean,
    Manhattan,
    Chebyshev
};

namespace detail {
    TCOB_API auto distance(heuristic h, point_i a, point_i b) -> u64;

    class TCOB_API neighbor_list {
        using data = std::array<point_i, 8>;

    public:
        data Data;
        i32  Count {0};

        auto begin() const -> data::const_iterator;
        auto end() const -> data::const_iterator;
    };

    TCOB_API auto neighbors(bool allowDiagonal, size_i gridSize, point_i pos) -> neighbor_list;

    TCOB_API auto reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) -> std::vector<point_i>;
}

////////////////////////////////////////////////////////////

class TCOB_API astar final {
public:
    explicit astar(bool allowDiagonal = false, heuristic heuristic = heuristic::Manhattan);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    class TCOB_API node {
    public:
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    bool      _allowDiagonal;
    heuristic _heuristic;
};

////////////////////////////////////////////////////////////

class TCOB_API bidir_astar final {
public:
    explicit bidir_astar(bool allowDiagonal = false, heuristic heuristic = heuristic::Manhattan);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    class TCOB_API node {
    public:
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    auto reconstruct_path(grid<point_i> const& cameFrom, point_i current, point_i sentinel) -> std::vector<point_i>;

    bool      _allowDiagonal;
    heuristic _heuristic;
};

////////////////////////////////////////////////////////////

class TCOB_API astar_minturns final {
public:
    explicit astar_minturns(bool allowDiagonal = false, heuristic heuristic = heuristic::Manhattan);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    class TCOB_API state {
    public:
        point_i Pos;
        point_i Dir;
        auto    operator==(state const& other) const -> bool = default;
    };

    class TCOB_API node {
    public:
        state State;
        u64   G {};
        u64   F {};
        auto  operator<=>(node const& other) const -> std::strong_ordering;
    };

    bool      _allowDiagonal;
    heuristic _heuristic;
};

////////////////////////////////////////////////////////////

class TCOB_API thetastar final {
public:
    explicit thetastar(bool allowDiagonal = true);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    class TCOB_API node {
    public:
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    auto has_line_of_sight(PathGrid auto&& testGrid, size_i gridExtent, point_i a, point_i b) const -> bool;

    bool                   _allowDiagonal;
    static heuristic const _heuristic {heuristic::Euclidean};
};

////////////////////////////////////////////////////////////

class TCOB_API lpastar final {
public:
    explicit lpastar(bool allowDiagonal = true, heuristic heuristic = heuristic::Manhattan);

    void initialize(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish);

    void update(PathGrid auto&& testGrid, point_i changedTile);

    auto path() const -> std::vector<point_i> const&;

private:
    class TCOB_API key {
    public:
        u64  K1 {};
        u64  K2 {};
        auto operator<=>(key const& other) const -> std::strong_ordering = default;
    };

    class TCOB_API node {
    public:
        point_i Pos;
        key     Key;
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    auto calculate_key(point_i p) const -> key;
    void update_vertex(PathGrid auto&& testGrid, point_i p);
    void compute_shortest_path(PathGrid auto&& testGrid);
    void rebuild_path();

    size_i    _gridExtent {};
    point_i   _start {-1, -1};
    point_i   _finish {-1, -1};
    bool      _allowDiagonal {false};
    heuristic _heuristic;

    grid<u64>     _g {};
    grid<u64>     _rhs {};
    grid<point_i> _parent {};
    grid<key>     _nodeKey {};
    grid<bool>    _inOpen {};

    std::set<node>       _open;
    std::vector<point_i> _path;
};

////////////////////////////////////////////////////////////

class TCOB_API dstar_lite final {
public:
    explicit dstar_lite(bool allowDiagonal = false, heuristic heuristic = heuristic::Manhattan);

    void initialize(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish);

    void update(PathGrid auto&& testGrid, point_i changedTile);

    auto path() const -> std::vector<point_i> const&;

    void move(PathGrid auto&& testGrid);

    auto position() const -> point_i;

private:
    class TCOB_API key {
    public:
        u64  K1 {};
        u64  K2 {};
        auto operator<=>(key const& other) const -> std::strong_ordering = default;
    };

    class TCOB_API node {
    public:
        point_i Pos;
        key     Key;
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    auto calculate_key(point_i p) const -> key;
    void update_vertex(PathGrid auto&& testGrid, point_i p);
    void compute_shortest_path(PathGrid auto&& testGrid);
    void rebuild_path();

    size_i    _gridExtent {};
    point_i   _start {-1, -1};
    point_i   _finish {-1, -1};
    point_i   _current {-1, -1};
    point_i   _last {-1, -1};
    bool      _allowDiagonal {false};
    heuristic _heuristic;
    u64       _km {0};

    grid<u64>     _g {};
    grid<u64>     _rhs {};
    grid<point_i> _parent {};
    grid<key>     _nodeKey {};
    grid<bool>    _inOpen {};

    std::set<node>       _open;
    std::vector<point_i> _path;
};

////////////////////////////////////////////////////////////

class TCOB_API flow_field final {
public:
    static constexpr point_i INVALID_DIR {-2, -2};

    explicit flow_field(bool allowDiagonal = false);

    void build(PathGrid auto&& testGrid, size_i gridExtent, point_i finish);

    auto direction(point_i from) const -> point_i;

    auto path(point_i from) const -> std::vector<point_i>;

private:
    class TCOB_API node {
    public:
        u64     Cost {};
        point_i Pos;
        auto    operator<=>(node const& other) const -> std::strong_ordering;
    };

    bool    _allowDiagonal;
    size_i  _gridExtent {};
    point_i _finish {-1, -1};

    grid<u64>     _cost {};
    grid<point_i> _flow {};
};

}

#include "Pathfinding.inl"
