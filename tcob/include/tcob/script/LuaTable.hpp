// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <cassert>
#include <sstream>
#include <vector>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/script/LuaRef.hpp>

namespace tcob::lua {
class Table : public Ref {
    template <typename... Keys>
    struct Proxy final {
        Proxy(const Table* scr, std::tuple<Keys...> keys)
            : _table { scr }
            , _keys { keys }
        {
        }

        template <typename T>
        operator T() const
        {
            return std::apply(
                [this](auto&&... args) {
                    return _table->get<T>(args...).Value;
                },
                _keys);
        }

        template <typename T>
        void operator=(T&& other)
        {
            std::apply(
                [this, &other](auto&&... args) {
                    return _table->set(args..., std::forward<T>(other));
                },
                _keys);
        }

        template <typename Key>
        auto operator[](Key key) const -> Proxy<Keys..., Key>
        {
            return Proxy<Keys..., Key>(_table, std::tuple_cat(_keys, std::make_tuple(key)));
        }

        const Table* _table;
        std::tuple<Keys...> _keys;
    };

public:
    Table();
    Table(const State& ls, i32 idx);

    template <typename Key>
    auto operator[](Key key) const -> Proxy<Key>
    {
        return Proxy<Key> { this, std::make_tuple(key) };
    }

    template <typename... Keys>
    void set(Keys&&... keys) const
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };
        set(ls, keys...);
    }

    template <typename T, typename... Keys>
    auto get(Keys&&... keys) const -> Result<T>
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };
        const Result<T> retValue { get<T>(ls, keys...) };
        return retValue;
    }

    template <typename T, typename... Keys>
    auto is(Keys&&... keys) const -> bool
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };
        const bool retValue { is<T>(ls, keys...) };
        return retValue;
    }

    template <typename... Keys>
    auto has(Keys&&... keys) const -> bool
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };
        const bool retValue { has(ls, keys...) };
        return retValue;
    }

    auto raw_length() const -> isize;

    auto create_table(const std::string& name) const -> Table;

    void dump(OutputFileStream& stream) const;
    void dump(std::stringstream& stream) const;

    template <typename T>
    auto keys() const -> std::vector<T>
    {
        const auto& ls { state() };
        const auto guard { ls.create_stack_guard() };

        std::vector<T> retValue {};
        push_self();
        ls.push(nullptr);
        while (ls.next(-2)) {
            ls.push_value(-2);

            T var {};
            if (Converter<T>::IsType(ls, -1) && ls.try_get(-1, var)) {
                retValue.push_back(var);
            }

            ls.pop(2);
        }

        return retValue;
    }

private:
    void dump_it(std::stringstream& stream, i32 indent) const;

    template <typename Key, typename... Keys>
    void set(const State& state, Key&& key, Keys&&... keys) const
    {
        push_self();
        state.push(key);

        if constexpr (sizeof...(Keys) > 1) {
            state.get_table(-2);
            Table lt;

            if (!state.is_table(-1)) { // set new nested table
                state.new_table();
                lt.ref(state, -1);
                set(state, key, lt);
            } else {
                lt.ref(state, -1);
            }

            lt.set(state, keys...);
        } else {
            state.push(keys...);
            state.set_table(-3);
        }
    }

    template <typename T, typename Key, typename... Keys>
    auto get(const State& state, Key&& key, Keys&&... keys) const -> Result<T>
    {
        push_self();
        state.push(key);
        state.get_table(-2);

        if constexpr (sizeof...(Keys) > 0) {
            if (!state.is_table(-1)) {
                return { T(), ResultState::NonTableIndex };
            }
            Table lt { state, -1 };
            return lt.get<T>(state, keys...);
        } else {
            T retValue {};
            ResultState result;

            if (state.is_nil(-1)) {
                result = ResultState::Undefined;
            } else {
                result = state.try_get(-1, retValue) ? ResultState::Ok : ResultState::TypeMismatch;
            }
            return { retValue, result };
        }
    }

    template <typename T, typename Key, typename... Keys>
    auto is(const State& state, Key&& key, Keys&&... keys) const -> bool
    {
        push_self();
        state.push(key);
        state.get_table(-2);

        if constexpr (sizeof...(Keys) > 0) {
            if (!state.is_table(-1)) {
                return false;
            }
            Table lt { state, -1 };
            return lt.is<T>(state, keys...);
        } else {
            return !state.is_nil(-1) && Converter<T>::IsType(state, -1);
        }
    }

    template <typename Key, typename... Keys>
    auto has(const State& state, Key&& key, Keys&&... keys) const -> bool
    {
        push_self();
        state.push(key);
        state.get_table(-2);

        if constexpr (sizeof...(Keys) > 0) {
            if (!state.is_table(-1)) {
                return false;
            }
            Table lt { state, -1 };
            return lt.has(state, keys...);
        } else {
            return !state.is_nil(-1);
        }
    }
};
}