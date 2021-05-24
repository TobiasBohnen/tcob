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

namespace tcob {
enum class LuaLibrary : char {
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

class LuaScript {
public:
    LuaScript();
    virtual ~LuaScript();

    LuaScript(const LuaScript&) = delete;
    auto operator=(const LuaScript& other) -> LuaScript& = delete;

    template <typename... Args>
    void open_libraries(Args&&... args)
    {
        if constexpr (sizeof...(args) == 0) {
            load_library(LuaLibrary::Table, LuaLibrary::String,
                LuaLibrary::Math, LuaLibrary::Coroutine,
                LuaLibrary::IO, LuaLibrary::Utf8, LuaLibrary::Package);
        } else {
            load_library(args...);
        }
    }

    template <typename R = void>
    auto run_file(const std::string& file, i32 idx = 1) const -> LuaResult<R>
    {
        std::string s { FileSystem::read_as_string(file) };
        return run_script<R>(s, idx, file);
    }

    template <typename R = void>
    auto run_script(const std::string& script, i32 idx = 1, const std::string& name = "Script") const -> LuaResult<R>
    {
        _state.save_top();
        LuaResultState result { call_buffer(script.data(), script.size(), name.c_str()) };
        if (result == LuaResultState::Ok) {
            if constexpr (std::is_void_v<R>) {
                _state.restore_top();
                return { result };
            } else {
                R retValue {};
                const bool ok { _state.try_get(std::forward<int>(idx), retValue) };
                _state.restore_top();
                if (ok) {
                    return { retValue, result };
                } else {
                    return { R(), LuaResultState::TypeMismatch };
                }
            }
        } else {
            _state.restore_top();
            if constexpr (std::is_void_v<R>) {
                return { result };
            } else {
                return { R(), result };
            }
        }
    }

    template <typename R = void>
    auto run_file_async(const std::string& file, i32 idx = 1) const -> std::future<LuaResult<R>>
    {
        return std::async(std::launch::async, [this, &file, idx] {
            return run_file<R>(file, idx);
        });
    }

    template <typename R = void>
    auto run_script_async(const std::string& script, i32 idx = 1, const std::string& name = "Script") const -> std::future<LuaResult<R>>
    {
        return std::async(std::launch::async, [this, script, idx, name] {
            return run_script<R>(script, idx, name);
        });
    }

    template <typename R = void>
    auto load_binary(const std::string& file, i32 idx = 1) const -> LuaFunction<R>
    {
        _state.save_top();

        LuaFunction<R> retVal {};
        if (load_binarybuffer(file)) {
            _state.try_get<LuaFunction<R>>(-1, retVal);
        }

        _state.restore_top();
        return retVal;
    }

    auto global_table() const -> const LuaTable&;

    void perform_GC() const;
    void stop_GC() const;
    void restart_GC() const;

    template <typename T>
    auto create_wrapper(const std::string& name) -> LuaWrapper<T>&
    {
        if (_wrappers.contains(name)) {
            _wrappers[name] = nullptr;
        }

        auto wrap { std::make_shared<LuaWrapper<T>>(_state, _globalTable, name) };
        _wrappers[name] = wrap;
        return *wrap;
    }

    void register_searcher(const std::function<LuaTable(LuaScript&, const std::string&)>& func);

private:
    auto do_call(i32 nargs, i32 nret) const -> LuaResultState;
    auto call_buffer(const byte* script, isize length, const std::string& name) const -> LuaResultState;
    auto load_binarybuffer(const std::string& filee) const -> bool;

    template <typename... Args>
    void load_library(LuaLibrary lib, Args&&... args)
    {
        load_library(lib);

        if constexpr (sizeof...(args) > 0)
            load_library(args...);
    }
    void load_library(LuaLibrary lib);

    LuaState _state;

    LuaTable* _globalTable;

    std::unordered_map<std::string, std::shared_ptr<detail::LuaWrapperBase>> _wrappers;
    std::function<std::function<LuaTable(const std::string&)>&(const std::string&)> _searcher;
    std::function<LuaTable(const std::string&)> _loader;
};
} /* namespace tcob */