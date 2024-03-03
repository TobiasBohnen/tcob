// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/scripting/squirrel/SquirrelScript.hpp"

#if defined(TCOB_ENABLE_ADDON_SCRIPTING_SQUIRREL)

    #include <cstdarg>
    #include <iostream>

    #include <squirrel.h>

    #include "tcob/core/Logger.hpp"

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
                if (si.source && si.funcname) {
                    logger::Error("Squirrel: \"{}\" @([{}] func: {} line: {})",
                                  sErr, si.source, si.funcname, si.line);
                }
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
    : _vm {vm_view::NewVM()}
{
    _vm.set_print_func(&print, &print);
    _vm.set_compiler_errorhandler(&comp_error);
    _vm.new_closure(&error, 0);
    _vm.set_errorhandler();

    _vm.push_roottable();
    _rootTable.acquire(_vm, -1);
    _vm.pop(1);
}

script::~script()
{
    _rootTable.release();
    clear_wrappers();
    _vm.close();
}

auto script::get_root_table() -> table&
{
    return _rootTable;
}

auto script::get_view() const -> vm_view
{
    return _vm;
}

auto script::create_array() const -> array
{
    return array {get_view()};
}

auto script::create_table() const -> table
{
    return table {get_view()};
}

auto script::create_class() const -> clazz
{
    return clazz {get_view()};
}

auto script::call_buffer(string_view script, string const& name, bool retValue) const -> error_code
{
    if (_vm.compile_buffer(script, name) == error_code::Ok) {
        _vm.push_roottable();
        if (_vm.call(1, retValue, true) == error_code::Ok) {
            return error_code::Ok;
        }
    }

    return error_code::Error;
}

void script::load_library(library lib)
{
    _rootTable.push_self();
    switch (lib) {
    case library::IO:
        _vm.register_iolib();
        break;
    case library::Blob:
        _vm.register_bloblib();
        break;
    case library::Math:
        _vm.register_mathlib();
        break;
    case library::System:
        _vm.register_systemlib();
        break;
    case library::String:
        _vm.register_stringlib();
        break;
    }
    _vm.poptop();
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
