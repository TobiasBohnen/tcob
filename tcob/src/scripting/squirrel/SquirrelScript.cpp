// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/squirrel/SquirrelScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <squirrel.h>

    #include <cstdarg>
    #include <cstdio>
    #include <iostream>
    #include <memory>
    #include <optional>
    #include <utility>

    #include "tcob/core/Logger.hpp"
    #include "tcob/scripting/Scripting.hpp"
    #include "tcob/scripting/squirrel/Squirrel.hpp"
    #include "tcob/scripting/squirrel/SquirrelTypes.hpp"

namespace tcob::scripting::squirrel {

extern "C" {
void static print(HSQUIRRELVM, char const* c, ...)
{
    #if defined(__llvm__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wformat-nonliteral"
    #endif
    va_list vl {};
    va_start(vl, c);
    string buf {};
    buf.resize(256);
    auto newSize {vsnprintf(buf.data(), buf.size(), c, vl)};
    if (newSize != -1) {
        buf.resize(newSize);
    }
    va_end(vl);
    #if defined(__llvm__)
        #pragma clang diagnostic pop
    #endif
    std::cout << buf << "\n";
}

void static comp_error(HSQUIRRELVM, char const* desc, char const* source, SQInteger line, SQInteger column)
{
    logger::Error("Squirrel: \"{}\" @([{}] line: {} col: {})",
                  desc, source, line, column);
}

auto static error(HSQUIRRELVM v) -> SQInteger
{
    SQChar const* sErr {nullptr};
    if (sq_gettop(v) >= 1) {
        if (SQ_SUCCEEDED(sq_getstring(v, 2, &sErr))) {
            SQStackInfos si {};
            if (SQ_SUCCEEDED(sq_stackinfos(v, 1, &si)) || SQ_SUCCEEDED(sq_stackinfos(v, 0, &si))) {
                logger::Error("Squirrel: \"{}\" @([{}] func: {} line: {})",
                              sErr ? sErr : "", si.source ? si.source : "?", si.funcname ? si.funcname : "?", si.line);
            } else {
                logger::Error("Squirrel: \"{}\"", sErr);
            }
        } else {
            logger::Error("Squirrel: unknown error");
        }
    }

    return 0;
}
}

script::script()
    : _view {vm_view::NewVM()}
{
    _view.set_print_func(&print, &print);
    _view.set_compiler_errorhandler(&comp_error);
    _view.new_closure(&error, 0);
    _view.set_errorhandler();

    _view.push_roottable();
    _rootTable.acquire(_view, -1);
    _view.pop(1);
}

script::~script()
{
    _rootTable.release();
    clear_wrappers();
    _view.close();
}

auto script::root_table() -> table&
{
    return _rootTable;
}

auto script::get_view() const -> vm_view
{
    return _view;
}

auto script::create_array() const -> array
{
    return array::Create(_view);
}

auto script::create_table() const -> table
{
    return table::Create(_view);
}

auto script::create_class() const -> clazz
{
    return clazz::Create(_view);
}

void script::enable_debug_info() const
{
    _view.enable_debug_info(true);
}

void static hook(HSQUIRRELVM v, SQInteger type, SQChar const* sourcename, SQInteger line, SQChar const* funcname)
{
    vm_view view {v};

    script::HookFunc* hook {reinterpret_cast<script::HookFunc*>(view.get_foreign_ptr())};
    (*hook)(static_cast<debug_event>(type), sourcename ? sourcename : "", line, funcname ? funcname : "");
}

void script::set_hook(HookFunc&& func)
{
    _hookFunc = std::move(func);
    _view.set_foreign_ptr(&_hookFunc);
    _view.set_native_debughook(&hook);
}

void script::remove_hook()
{
    _view.set_native_debughook(nullptr);
}

auto script::call_buffer(string_view script, string const& name, bool retValue) const -> std::optional<error_code>
{
    if (!_view.compile_buffer(script, name)) {
        _view.push_roottable();
        return _view.call(1, retValue, true);
    }

    return error_code::Error;
}

void script::load_library(library lib)
{
    _rootTable.push_self();
    switch (lib) {
    case library::IO:     _view.register_iolib(); break;
    case library::Blob:   _view.register_bloblib(); break;
    case library::Math:   _view.register_mathlib(); break;
    case library::System: _view.register_systemlib(); break;
    case library::String: _view.register_stringlib(); break;
    }
    _view.poptop();
}
}

////////////////////////////////////////////////////////////

auto tcob::literals::operator""_squirrel(char const* str, usize) -> std::unique_ptr<tcob::scripting::squirrel::script>
{
    auto retValue {std::make_unique<tcob::scripting::squirrel::script>()};
    (void)retValue->run(string {str});
    return retValue;
}

#endif
