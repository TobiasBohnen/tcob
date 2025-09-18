// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "tcob/core/Interfaces.hpp"

namespace tcob {

////////////////////////////////////////////////////////////

struct event_base {
    mutable bool Handled {false};
};

////////////////////////////////////////////////////////////

namespace detail {
    class signal_base;
}

////////////////////////////////////////////////////////////

class TCOB_API connection {
public:
    connection(detail::signal_base const* signal, uid id);
    connection(connection const& other) noexcept                    = default;
    auto operator=(connection const& other) noexcept -> connection& = default;
    connection(connection&& other) noexcept                         = default;
    auto operator=(connection&& other) noexcept -> connection&      = default;
    virtual ~connection()                                           = default;

    void disconnect();

    auto id() const -> uid;

protected:
    detail::signal_base const* _signal;
    uid                        _id {INVALID_ID};
};

class [[nodiscard]] TCOB_API scoped_connection final : public connection, public non_copyable {
public:
    scoped_connection();
    scoped_connection(connection const& other);
    scoped_connection(scoped_connection&& other) noexcept;
    auto operator=(scoped_connection&& other) noexcept -> scoped_connection&;
    ~scoped_connection() override;
};

////////////////////////////////////////////////////////////

namespace detail {
    class TCOB_API signal_base : public non_copyable {
    public:
        signal_base()          = default;
        virtual ~signal_base() = default;

        virtual void disconnect(uid id) const = 0;

    protected:
        auto next_id() const -> uid;
    };
}

////////////////////////////////////////////////////////////

template <typename EvArgs = void>
class signal final : public detail::signal_base {
    static constexpr bool IsVoid {std::is_void_v<std::remove_cvref_t<EvArgs>>};

    template <typename T, bool IsVoid>
    struct slot_func_type;

    template <typename T>
    struct slot_func_type<T, false> {
        using type = std::function<void(T&)>;
    };

    template <typename T>
    struct slot_func_type<T, true> {
        using type = std::function<void()>;
    };

    using slot_func = typename slot_func_type<EvArgs, IsVoid>::type;
    using slots     = std::vector<std::pair<uid, slot_func>>;

public:
    void operator()() const
        requires(IsVoid);

    template <typename S = EvArgs>
    void operator()(S&& args) const
        requires(!IsVoid);

    template <typename Func>
    auto connect(Func func) const -> connection;
    template <auto Func, typename T>
    auto connect(T* inst) const -> connection;

    void disconnect(uid id) const override;
    void disconnect_all() const;

    template <typename Func>
    auto operator+=(Func func) const -> connection;

    auto operator-=(connection& c) const -> void;
    auto operator-=(uid c) const -> void;

    auto slot_count() const -> isize;

private:
    mutable slots _slots;
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
