#include "tests.hpp"

using namespace tcob::scripting;
using namespace tcob::scripting::lua;

class LuaDSLTests : public lua::script {
public:
    LuaDSLTests()
        : global(get_global_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(LuaDSLTests, "Script.LuaDSL.Form")
{
    std::string script = R"-(
        local function rect(x,y,w,h)
            return {x=x, y=y, width=w, height=h}
        end
        local function size(w,h)
            return {width=w, height=h}
        end

        form1 = ui.form {
            ui.panel 'panel1' {
                bounds = rect(0, 0, 200, 200),
                layout = ui.grid_layout {
                    ui.button    'button1'   { label = 'click me',     bounds = rect(0, 0, 1, 1) },
                    ui.button    'button2'   { label = 'click me too', bounds = rect(0, 1, 3, 1) },
                    ui.check_box 'checkbox1' { checked = true,         bounds = rect(2, 2, 1, 1) }
                }
            },
            ui.panel 'panel2' {
                bounds = rect(0, 300, 100, 200),
                layout = ui.grid_layout {
                    ui.button 'button3' { label = 'click me', bounds = rect(0, 0, 1, 2) },
                    ui.panel  'panel3'  {
                        bounds = rect(0, 0, 200, 200),
                        layout = ui.hbox_layout {
                            ui.button    'button4'   { label = 'click me' },
                            ui.button    'button5'   { label = 'click me too' },
                            ui.check_box 'checkbox2' { checked = true }
                        }
                    }
                }
            },
            ui.tab_container 'tabs1' {
                bounds = rect(250, 150, 100, 200),
                tabs   = {
                    ui.panel  'panel4' {
                        layout = ui.grid_layout {
                            ui.check_box 'checkbox3' { checked = true, bounds = rect(2, 2, 1, 1) }
                        }
                    },
                    ui.panel  'panel5' {
                        layout = ui.grid_layout {
                            ui.check_box 'checkbox4' { checked = true, bounds = rect(2, 2, 1, 1) }
                        }
                    }
                }
            }
        }
    )-";

    std::vector<native_closure_shared_ptr> funcs;

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    auto* l_form {make_func([&](table& content) {
        io::ofstream stream {"forms.txt"};
        content.dump(stream);
        return content;
    })};

    global["ui"]         = table {};
    global["ui"]["form"] = l_form;

    // layouts
    auto make_layout_func {[&](string_view type) {
        return make_func([&, type](table& t) {
            table lt {get_view()};
            lt["type"]    = type;
            lt["widgets"] = t;
            return lt;
        });
    }};

    std::array const layouts {"fixed_layout", "flex_size_layout", "grid_layout", "hbox_layout", "vbox_layout"};
    for (auto const& l : layouts) {
        global["ui"][l] = make_layout_func(l);
    }

    // widgets
    auto make_widget_func {[&](string_view type, string_view name) {
        return make_func([&, type, name](table& t) {
            t["type"] = type;
            t["name"] = name;
            return t;
        });
    }};

    std::array const widgets {"panel", "tab_container", "accordion", "tooltip", "button", "check_box",
                              "cycle_button", "image_box", "label", "list_box", "progress_bar", "radio_button",
                              "slider", "spinner", "text_box", "toggle"};
    for (auto const& w : widgets) {
        global["ui"][w] = make_func([&](string_view name) { return make_widget_func(w, name); });
    }

    auto res = run(script);
    REQUIRE(res);

    table formContent = global["form1"];
    table panel1      = formContent[1];
    REQUIRE(panel1["type"].as<string>() == "panel");
    REQUIRE(panel1["name"].as<string>() == "panel1");
    REQUIRE(panel1["bounds"].as<rect_f>() == rect_f {0, 0, 200, 200});
    table layout1 = panel1["layout"];
    REQUIRE(layout1["type"].as<string>() == "grid_layout");
    REQUIRE(layout1["widgets"][1]["type"].as<string>() == "button");
    REQUIRE(layout1["widgets"][1]["name"].as<string>() == "button1");
    REQUIRE(layout1["widgets"][1]["label"].as<string>() == "click me");
    REQUIRE(layout1["widgets"][1]["bounds"].as<rect_f>() == rect_f {0, 0, 1, 1});
    REQUIRE(layout1["widgets"][2]["name"].as<string>() == "button2");
    REQUIRE(layout1["widgets"][3]["name"].as<string>() == "checkbox1");
    REQUIRE(formContent[2]["type"].as<string>() == "panel");
    REQUIRE(formContent[2]["name"].as<string>() == "panel2");
}

TEST_CASE_FIXTURE(LuaDSLTests, "Script.LuaDSL.Style")
{
    std::string script = R"-(
        styles = ui.styles {
            ui.style.button 'button' {
                background = 'chartreuse',
                border = { type = 'solid' },
            },
            ui.style.button 'button:hover' {
                background = 'darkkhaki'
            }
        }
    )-";

    std::vector<native_closure_shared_ptr> funcs;

    auto make_func {[&](auto&& func) {
        auto ptr {make_shared_closure(std::function {func})};
        funcs.push_back(ptr);
        return ptr.get();
    }};

    auto* l_styles {make_func([&](table& content) {
        io::ofstream stream {"styles.lua"};
        content.dump(stream);
        return content;
    })};

    global["ui"]           = table {};
    global["ui"]["styles"] = l_styles;

    // styles
    auto make_style_func {[&](string_view type, string_view className) {
        return make_func([&, type, className](table& t) {
            t["type"]  = type;
            t["class"] = className;
            return t;
        });
    }};

    std::array const styles {"panel", "tab_container", "accordion", "tooltip", "button", "check_box",
                             "cycle_button", "image_box", "label", "list_box", "progress_bar", "radio_button",
                             "slider", "spinner", "text_box", "toggle",
                             "thumb", "nav_arrows", "item"};
    for (auto const& w : styles) {
        global["ui"]["style"][w] = make_func([&](string_view name) { return make_style_func(w, name); });
    }

    auto res = run(script);
    REQUIRE(res);

    table stylesCntent = global["styles"];
    table buttonStyle  = stylesCntent[1];
    REQUIRE(buttonStyle["class"].as<string>() == "button");
    REQUIRE(buttonStyle["type"].as<string>() == "button");
    REQUIRE(buttonStyle["background"].as<string>() == "chartreuse");
    REQUIRE(buttonStyle["border"]["type"].as<string>() == "solid");
    table buttonHoverStyle = stylesCntent[2];
    REQUIRE(buttonHoverStyle["class"].as<string>() == "button:hover");
    REQUIRE(buttonHoverStyle["type"].as<string>() == "button");
    REQUIRE(buttonHoverStyle["background"].as<string>() == "darkkhaki");
    // TODO: REQUIRE(buttonHoverStyle["border"]["type"].as<string>() == "solid");
}
