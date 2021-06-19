// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <vector>

#include <tcob/thirdparty/sigslot/signal.hpp>

namespace tcob::detail {
class ConnectionManager final {
public:
    template <typename Signal, typename Func, typename Ptr>
    void connect(Signal& sig, Func func, Ptr ptr)
    {
        _connections.push_back(std::move(sig.connect(func, ptr)));
    }

    void disconnect_all()
    {
        _connections.clear();
    }

private:
    std::vector<sigslot::scoped_connection> _connections;
};
}