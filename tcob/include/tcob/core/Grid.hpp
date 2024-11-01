#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

////////////////////////////////////////////////////////////
namespace tcob {

template <typename T>
class grid {
    using iterator       = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;

    using size_type      = size_i;
    using dimension_type = size_type::type;
    using point_type     = point<dimension_type>;

public:
    grid(size_type size, T const& defaultValue = T {});

    auto at(point_type pos) -> T&;
    auto at(point_type pos) const -> T const&;

    void fill(T const& value);

    auto begin() -> iterator;
    auto begin() const -> const_iterator;

    auto end() -> iterator;
    auto end() const -> const_iterator;

    auto get_row_count() const -> dimension_type;
    auto get_column_count() const -> dimension_type;
    auto get_extent() const -> size_type;
    auto size() const -> usize;

private:
    auto get_index(point_type pos) const -> dimension_type;

    size_type      _size;
    std::vector<T> _data;
};

}

#include "Grid.inl"
