// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <utility>
#include <vector>

#include "tcob/core/Interfaces.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API signal_base : public non_copyable {
    public:
        static constexpr i32 INVALID_ID {-1};

        signal_base() = default;
        virtual ~signal_base();

        void virtual disconnect(i32 id) const = 0;

        auto get_next_id() const -> i32;

    private:
        mutable i32 _currentId {INVALID_ID};
    };
}

////////////////////////////////////////////////////////////

class TCOB_API connection {
public:
    connection(detail::signal_base const* signal, i32 id);
    connection(connection const& other) noexcept                    = default;
    auto operator=(connection const& other) noexcept -> connection& = default;
    connection(connection&& other) noexcept                         = default;
    auto operator=(connection&& other) noexcept -> connection&      = default;
    virtual ~connection();

    void disconnect();

    auto get_id() const -> i32;

protected:
    detail::signal_base const* _signal;
    i32                        _id {detail::signal_base::INVALID_ID};
};

class [[nodiscard]] TCOB_API scoped_connection final : public connection, public non_copyable {
public:
    scoped_connection(connection const& other);
    scoped_connection(scoped_connection&& other) noexcept;
    auto operator=(scoped_connection&& other) noexcept -> scoped_connection&;
    ~scoped_connection() override;
};

////////////////////////////////////////////////////////////

template <typename EvArgs = void>
class signal final : public detail::signal_base {
    using slot_func = std::function<void(EvArgs&)>;
    using slots     = std::vector<std::pair<i32, slot_func>>;

public:
    void operator()(EvArgs& args) const;

    template <typename Func>
    auto connect(Func func) const -> connection;
    template <auto Func, typename T>
    auto connect(T* inst) const -> connection;

    void disconnect(i32 id) const override;
    void disconnect_all() const;

    auto get_slot_count() const -> isize;

private:
    mutable slots _slots;
};

template <>
class signal<void> final : public detail::signal_base {
    using slot_func = std::function<void()>;
    using slots     = std::vector<std::pair<i32, slot_func>>;

public:
    void operator()() const;

    template <typename Func>
    auto connect(Func func) const -> connection;
    template <auto Func, typename T>
    auto connect(T* inst) const -> connection;

    void disconnect(i32 id) const override;
    void disconnect_all() const;

    auto get_slot_count() const -> isize;

private:
    [[maybe_unused]] mutable slots _slots;
};

////////////////////////////////////////////////////////////

namespace detail {
    class connection_manager final : public non_copyable {
    public:
        template <typename Signal, typename Func>
        void connect(Signal const& sig, Func func);

        template <auto Func, typename Signal, typename Ptr>
        void connect(Signal const& sig, Ptr ptr);

        void disconnect_all();

    private:
        std::vector<scoped_connection> _connections;
    };
}

}

#include "Signal.inl"
