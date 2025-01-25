// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/Signal.hpp"

#include "tcob/core/random/Random.hpp"

namespace tcob {

namespace detail {
    ////////////////////////////////////////////////////////////

    void connection_manager::disconnect_all()
    {
        _connections.clear();
    }

    ////////////////////////////////////////////////////////////

    signal_base::~signal_base() = default;

    auto signal_base::next_id() const -> uid
    {
        return GetRandomID();
    }

}

////////////////////////////////////////////////////////////

connection::connection(detail::signal_base const* signal, uid id)
    : _signal {signal}
    , _id {id}
{
}

connection::~connection() = default;

void connection::disconnect()
{
    if (_signal && _id != INVALID_ID) {
        _signal->disconnect(_id);
    }
}

auto connection::id() const -> uid
{
    return _id;
}

////////////////////////////////////////////////////////////

scoped_connection::scoped_connection()
    : connection {nullptr, INVALID_ID}
{
}

scoped_connection::scoped_connection(connection const& other)
    : connection {other}
{
}

scoped_connection::~scoped_connection()
{
    disconnect();
}

scoped_connection::scoped_connection(scoped_connection&& other) noexcept
    : connection {other._signal, other._id}
{
    std::exchange(other._id, INVALID_ID);
    std::exchange(other._signal, nullptr);
}

auto scoped_connection::operator=(scoped_connection&& other) noexcept -> scoped_connection&
{
    disconnect();
    std::swap(_id, other._id);
    std::swap(_signal, other._signal);
    return *this;
}

}
