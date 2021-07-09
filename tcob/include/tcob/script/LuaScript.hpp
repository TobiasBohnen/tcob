// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <future>

#include <tcob/core/io/FileSystem.hpp>
#include <tcob/script/LuaConversions.hpp>
#include <tcob/script/LuaFunction.hpp>
#include <tcob/script/LuaState.hpp>
#include <tcob/script/LuaTable.hpp>
#include <tcob/script/LuaWrapper.hpp>

namespace tcob::lua {
enum class Library : char {
    Table,
    String,
    Math,
    Coroutine,
    IO,
    OS,
    Utf8,
    Debug,
    Package
};

class Script {
public:
    Script();
    virtual ~Script();

    Script(const Script&) = delete;
    auto operator=(const Script& other) -> Script& = delete;

    template <typename... Args>
    void open_libraries(Args&&... args)
    {
        if constexpr (sizeof...(args) == 0) {
            load_library(Library::Table, Library::String,
                Library::Math, Library::Coroutine,
                Library::IO, Library::Utf8, Library::Package);
        } else {
            load_library(args...);
        }
    }

    template <typename R = void>
    auto run_file(const std::string& file, i32 idx = 1) const -> Result<R>
    {
        return run_script<R>(FileSystem::read_as_string(file), idx, file);
    }

    template <typename R = void>
    auto run_script(const std::string& script, i32 idx = 1, const std::string& name = "Script") const -> Result<R>
    {
        const auto guard { _state.create_stack_guard() };
        auto result { call_buffer(script.data(), script.size(), name.c_str()) };
        if constexpr (std::is_void_v<R>) {
            return { result };
        } else {
            R retValue {};
            if (result == ResultState::Ok) {
                if (!_state.try_get(std::forward<int>(idx), retValue)) {
                    result = ResultState::TypeMismatch;
                }
            }

            return { retValue, result };
        }
    }

    template <typename R = void>
    auto run_file_async(const std::string& file, i32 idx = 1) const -> std::future<Result<R>>
    {
        return std::async(std::launch::async, [this, &file, idx] {
            return run_file<R>(file, idx);
        });
    }

    template <typename R = void>
    auto run_script_async(const std::string& script, i32 idx = 1, const std::string& name = "Script") const -> std::future<Result<R>>
    {
        return std::async(std::launch::async, [this, script, idx, name] {
            return run_script<R>(script, idx, name);
        });
    }

    template <typename R = void>
    auto load_binary(const std::string& file, i32 idx = 1) const -> Function<R>
    {
        const auto guard { _state.create_stack_guard() };

        Function<R> retVal {};
        if (load_binarybuffer(file)) {
            _state.try_get<Function<R>>(-1, retVal);
        }

        return retVal;
    }

    auto global_table() const -> const Table&;

    void perform_GC() const;
    void stop_GC() const;
    void restart_GC() const;

    template <typename T>
    auto create_wrapper(const std::string& name) -> Wrapper<T>&
    {
        if (_wrappers.contains(name)) {
            _wrappers[name] = nullptr;
        }

        auto wrap { std::make_shared<Wrapper<T>>(_state, _globalTable.get(), name) };
        _wrappers[name] = wrap;
        return *wrap;
    }

    void register_searcher(const std::function<Table(Script&, const std::string&)>& func);

private:
    auto do_call(i32 nargs, i32 nret) const -> ResultState;
    auto call_buffer(const byte* script, isize length, const std::string& name) const -> ResultState;
    auto load_binarybuffer(const std::string& file) const -> bool;

    template <typename... Args>
    void load_library(Library lib, Args&&... args)
    {
        load_library(lib);

        if constexpr (sizeof...(args) > 0)
            load_library(args...);
    }
    void load_library(Library lib);

    State _state;

    std::unique_ptr<Table> _globalTable;

    std::unordered_map<std::string, std::shared_ptr<detail::WrapperBase>> _wrappers;
    std::function<std::function<Table(const std::string&)>&(const std::string&)> _searcher;
    std::function<Table(const std::string&)> _loader;
};
} /* namespace tcob */