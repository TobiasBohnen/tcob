#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob {
////////////////////////////////////////////////////////////

template <typename T>
class grid {
    using iterator       = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;

    using size_type      = size_i;
    using dimension_type = size_type::type;
    using point_type     = point<dimension_type>;

public:
    using type = T;

    grid(size_type size, T const& defaultValue = T {});

    auto operator[](point_type pos) -> T&;
    auto operator[](point_type pos) const -> T const&;
    auto operator[](usize idx) -> T&;
    auto operator[](usize idx) const -> T const&;

    void fill(T const& value);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto extent() const -> size_type;
    auto contains(point_type pos) const -> bool;

    auto size() const -> usize;
    auto data() -> T*;
    auto data() const -> T const*;

private:
    auto get_index(point_type pos) const -> dimension_type;

    size_type      _size;
    std::vector<T> _data;
};

////////////////////////////////////////////////////////////

template <typename T, usize Width, usize Height>
class static_grid {
    using array_type     = std::array<T, Width * Height>;
    using iterator       = array_type::iterator;
    using const_iterator = array_type::const_iterator;

    using size_type      = size_i;
    using dimension_type = size_type::type;
    using point_type     = point<dimension_type>;

public:
    using type = T;

    static_grid();
    explicit static_grid(T const& defaultValue);

    auto operator[](point_type pos) -> T&;
    auto operator[](point_type pos) const -> T const&;
    auto operator[](usize idx) -> T&;
    auto operator[](usize idx) const -> T const&;

    void fill(T const& value);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto extent() const -> size_type;
    auto contains(point_type pos) const -> bool;

    auto size() const -> usize;
    auto data() -> T*;
    auto data() const -> T const*;

private:
    auto get_index(point_type pos) const -> dimension_type;

    array_type _data;
};

}

#include "Grid.inl"
