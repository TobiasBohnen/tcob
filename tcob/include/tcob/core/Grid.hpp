#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <initializer_list>
#include <span>
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

    grid() = default;
    explicit grid(size_type size, T const& defaultValue = T {});

    auto operator[](this auto&& self, isize idx) -> decltype(auto);
    auto operator[](this auto&& self, isize x, isize y) -> decltype(auto);
    auto operator[](this auto&& self, point_type pos) -> decltype(auto);

    auto row(this auto&& self, isize row) -> decltype(auto);

    void fill(T const& value);

    void assign(point_type pos, std::initializer_list<T> values);
    void assign(point_type pos, std::span<T const> values);

    void append(std::initializer_list<T> values);
    void append(std::span<T const> values);

    void erase(isize row);
    void clear();

    auto begin(this auto&& self) -> decltype(auto);
    auto end(this auto&& self) -> decltype(auto);

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto size() const -> size_type;

    auto count() const -> isize;
    void resize(size_type newSize);

    auto data(this auto&& self) -> decltype(auto);

    auto operator==(grid const& other) const -> bool = default;

private:
    auto get_index(isize x, isize y) const -> isize;

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
    explicit static_grid(std::initializer_list<std::initializer_list<T>> rows);

    auto operator[](this auto&& self, isize idx) -> decltype(auto);
    auto operator[](this auto&& self, isize x, isize y) -> decltype(auto);
    auto operator[](this auto&& self, point_type pos) -> decltype(auto);

    void fill(T const& value);

    auto begin(this auto&& self) -> decltype(auto);

    auto end(this auto&& self) -> decltype(auto);

    auto height() const -> dimension_type;
    auto width() const -> dimension_type;
    auto size() const -> size_type;

    auto count() const -> usize;
    auto data(this auto&& self) -> decltype(auto);

    auto operator==(static_grid const& other) const -> bool = default;

private:
    auto get_index(isize x, isize y) const -> isize;

    array_type _data;
};

}

#include "Grid.inl"
