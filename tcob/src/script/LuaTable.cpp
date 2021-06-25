// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/script/LuaTable.hpp>

#include <lua.hpp>

#include <tcob/script/LuaConversions.hpp>

namespace tcob {
auto LuaTable::raw_length() const -> isize
{
    const LuaState ls { lua_state() };
    const auto guard { ls.create_stack_guard() };

    push_self();
    return ls.raw_len(-1);
}

auto LuaTable::create_table(const std::string& name) const -> LuaTable
{
    const LuaState ls { lua_state() };

    ls.new_table();
    LuaTable lt { ls.lua(), -1 };
    ls.pop(1);

    set(name, lt);

    return lt;
}

void LuaTable::dump(std::stringstream& stream) const
{
    const auto guard { lua_state().create_stack_guard() };

    push_self();

    dump_it(stream, 0);
    stream << std::endl;
}

void LuaTable::dump(OutputFileStream& stream) const
{
    std::stringstream sstream;
    dump(sstream);
    stream.write(sstream.str());
}

void LuaTable::dump_it(std::stringstream& stream, i32 indent) const
{
    const LuaState ls { lua_state() };

    std::string ind1(indent, ' ');
    std::string ind2(indent + 2, ' ');

    stream << "{" << std::endl;

    std::vector<std::variant<i64, std::string>> keys;
    ls.push_nil();
    while (ls.next(-2)) {
        ls.push_value(-2);
        if (ls.is_integer(-1))
            keys.emplace_back(ls.to_integer(-1));
        else if (ls.is_string(-1))
            keys.emplace_back(ls.to_string(-1));

        ls.pop(2);
    }
    std::sort(keys.begin(), keys.end());

    for (const auto& key : keys) {
        // push key
        if (std::holds_alternative<std::string>(key))
            ls.push_string(std::get<std::string>(key).c_str());
        else if (std::holds_alternative<i64>(key))
            ls.push_integer(std::get<i64>(key));

        ls.raw_get(-2); // get value

        LuaType type { ls.get_type(-1) };
        switch (type) {
        case LuaType::Nil:
        case LuaType::LightUserdata:
        case LuaType::Function:
        case LuaType::Userdata:
        case LuaType::Thread:
            ls.pop(1);
            continue;
        default:
            if (std::holds_alternative<std::string>(key))
                stream << ind2 << std::get<std::string>(key) << " = ";
            else if (std::holds_alternative<i64>(key))
                stream << ind2;
            break;
        }

        std::string val;
        switch (type) {
        case LuaType::Boolean:
            val = ls.to_bool(-1) ? "true" : "false";
            break;
        case LuaType::Number:
            val = ls.to_string(-1);
            break;
        case LuaType::String:
            val = "\"" + std::string(ls.to_string(-1)) + "\"";
            break;
        case LuaType::Table:
            ls.push_value(-1);
            dump_it(stream, indent + 2);
            break;
        default:
            break;
        }
        stream << val << "," << std::endl;

        ls.pop(1);
    }

    ls.pop(1);
    stream << ind1 << "}";
}
}