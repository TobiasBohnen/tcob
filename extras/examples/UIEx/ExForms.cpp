#include "ExForms.hpp"

using namespace tcob::literals;

////////////////////////////////////////////////////////////

auto create_form0(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form0", wnd)};

    auto tooltip0 {retValue->create_tooltip<tooltip>("tooltip")};
    auto tooltipLayout {tooltip0->get_layout<fixed_layout>()};
    tooltip0->Bounds = {0, 0, 170, 50};
    auto tooltipLabel0 {tooltipLayout->create_widget<label>({5, 5, 160, 40}, "TTLabel0")};
    tooltip0->Popup.connect([tooltipLabel0](auto const& event) { tooltipLabel0->Label = event.Widget->get_name(); });

    auto panel0 {retValue->create_container<panel>(dock_style::Fill, "Panel0")};
    panel0->Flex = {100_pct, 100_pct};
    auto panel0Layout {panel0->get_layout<fixed_layout>()};
    (*panel0->TabStop).Enabled = false;
    panel0->ScrollEnabled      = true;

    auto label0 {panel0Layout->create_widget<label>({0, 520, 450, 40}, "Label0")};
    auto button0 {panel0Layout->create_widget<button>({0, 0, 200, 100}, "Button0")};
    button0->Label = "Button0";
    label0->For    = button0;

    auto spinner0 {panel0Layout->create_widget<spinner>({0, 150, 120, 100}, "Spinner0")};
    spinner0->Min   = 0;
    spinner0->Step  = 25;
    spinner0->Max   = 100;
    spinner0->Value = 0;

    auto cycleButton0 {panel0Layout->create_widget<cycle_button>({0, 260, 120, 100}, "CycleButton0")};
    cycleButton0->add_item("abc");
    cycleButton0->add_item("def");
    cycleButton0->add_item("ghi");

    auto progressBar0 {panel0Layout->create_widget<progress_bar>({130, 150, 75, 200}, "ProgressBar0")};
    progressBar0->Min   = 0;
    progressBar0->Max   = 100;
    progressBar0->Value = 50;

    auto sliderPanel {panel0Layout->create_widget<panel>({490, 390, 350, 350}, "SliderPanel")};
    auto sliderPanelLayout {sliderPanel->get_layout<fixed_layout>()};

    auto slider0 {sliderPanelLayout->create_widget<slider>({0, 0, 250, 100}, "Slider0")};
    slider0->Min               = 0;
    slider0->Max               = 255;
    slider0->Value             = 50;
    slider0->Step              = 5;
    slider0->IncrementalChange = false;

    auto sliderLabel0 {sliderPanelLayout->create_widget<label>({50, 150, 50, 50}, "Label2")};
    sliderLabel0->For                = slider0;
    (*sliderLabel0->TabStop).Enabled = false;
    slider0->Value.Changed.connect([sliderLabel0, slider0](auto val) {
        sliderLabel0->Label = std::to_string(val);
        slider0->Alpha      = val / 255.f;
    });
    auto slider1 {sliderPanelLayout->create_widget<slider>({250, 0, 75, 250}, "Slider1")};
    slider1->Min               = 0;
    slider1->Max               = 100;
    slider1->Value             = 100;
    slider1->Step              = 5;
    slider1->IncrementalChange = false;
    slider1->Value.Changed.connect([sliderLabel0](auto val) {
        sliderLabel0->Label = std::to_string(val);
    });

    auto textBox0 {panel0Layout->create_widget<text_box>({890, 350, 125, 50}, "TextBox0")};
    textBox0->MaxLength = 9;
    textBox0->Submit.connect([label0, textBox0](auto const&) { label0->Label = "submitted: " + textBox0->Text(); });

    auto toggle0 {panel0Layout->create_widget<toggle>({890, 500, 150, 75}, "Toggle0")};
    toggle0->Enabled = true;

    auto gridPanel {panel0Layout->create_widget<panel>({230, 20, 250, 250}, "SPanel0")};
    (*gridPanel->TabStop).Enabled = false;
    auto gridLayout {gridPanel->create_layout<grid_layout>()};
    auto createGridWidget {
        [&](rect_i loc, std::string const& name) {
            auto b {gridLayout->create_widget<button>(loc, "gridB" + name)};
            b->Label   = name;
            b->Tooltip = tooltip0;
        }};
    createGridWidget({0, 0, 2, 1}, "0");
    createGridWidget({2, 0, 2, 1}, "1");
    createGridWidget({0, 1, 4, 1}, "2");
    createGridWidget({0, 2, 1, 1}, "3");
    createGridWidget({1, 2, 1, 1}, "4");
    createGridWidget({2, 2, 1, 1}, "5");
    createGridWidget({3, 2, 1, 1}, "6");

    auto boxVPanel0 {panel0Layout->create_widget<panel>({490, 20, 250, 250}, "SPanel1")};
    (*boxVPanel0->TabStop).Enabled = false;
    auto boxVLayout0 {boxVPanel0->create_layout<vbox_layout>()};
    for (i32 i = 0; i < 4; i++) {
        auto bb {boxVLayout0->create_widget<button>("Button" + std::to_string(i))};
        bb->Label = std::to_string(i);
    }

    auto boxHPanel1 {panel0Layout->create_widget<panel>({230, 280, 250, 100}, "SPanel2")};
    (*boxHPanel1->TabStop).Enabled = false;
    auto boxHLayout1 {boxHPanel1->create_layout<hbox_layout>()};
    for (i32 i = 0; i < 4; i++) {
        auto chk {boxHLayout1->create_widget<checkbox>("Check" + std::to_string(i))};
        chk->Checked.Changed.connect([label0, chk](bool value) { label0->Label = (value ? "checked: " : "unchecked: ") + chk->get_name(); });
    }

    auto boxHPanel2 {panel0Layout->create_widget<panel>({490, 280, 250, 100}, "SPanel3")};
    (*boxHPanel2->TabStop).Enabled = false;
    auto boxHLayout2 {boxHPanel2->create_layout<hbox_layout>()};
    for (i32 i = 0; i < 4; i++) {
        auto rb {boxHLayout2->create_widget<radio_button>("Radio" + std::to_string(i))};
        rb->Checked.Changed.connect([label0, rb](bool value) { label0->Label = (value ? "checked: " : "unchecked: ") + rb->get_name(); });
    }

    auto flexPanel0 {panel0Layout->create_widget<panel>({230, 390, 250, 160}, "SPanel4")};
    (*flexPanel0->TabStop).Enabled = false;
    flexPanel0->FocusGained.connect([](widget_event ev) { (*ev.Sender->Bounds).Width = 510; });
    flexPanel0->FocusLost.connect([](widget_event ev) { (*ev.Sender->Bounds).Width = 250; });
    auto flexLayout0 {flexPanel0->create_layout<flex_size_layout>()};
    for (i32 i = 0; i < 4; i++) {
        auto fb {flexLayout0->create_widget<button>({0.f, i * 32.5f}, "FButton" + std::to_string(i))};
        fb->Flex = {100_pct, 20_pct};
    }

    auto dropDownList0 {panel0Layout->create_widget<drop_down_list>({1200, 10, 150, 50}, "DropDownList0")};
    for (i32 i {0}; i < 4; ++i) {
        dropDownList0->add_item("item " + std::to_string(i));
    }
    dropDownList0->SelectedItemIndex.Changed.connect([label0, dropDownList0](i32 value) { label0->Label = "selected: " + std::to_string(value); });
    dropDownList0->HoveredItemIndex.Changed.connect([label0, dropDownList0](i32 value) { label0->Label = "hovered: " + std::to_string(value); });
    dropDownList0->ZOrder = 1;

    auto listbox0 {panel0Layout->create_widget<list_box>({1200, 80, 150, 300}, "Listbox0")};
    for (i32 i {0}; i < 4; ++i) {
        listbox0->add_item("item " + std::to_string(i));
    }
    listbox0->SelectedItemIndex.Changed.connect([label0, listbox0](i32 value) { label0->Label = "selected: " + std::to_string(value); });
    listbox0->HoveredItemIndex.Changed.connect([label0, listbox0](i32 value) { label0->Label = "hovered: " + std::to_string(value); });

    auto  imgBox0 {panel0Layout->create_widget<image_box>({750, 20, 150, 200}, "ImageBox0")};
    auto* resGrp {locate_service<assets::library>().get_group("ui")};
    imgBox0->Image = resGrp->get<texture>("blue_boxCross");

    auto gridView0 {panel0Layout->create_widget<grid_view>({1080, 400, 450, 400}, "GridView0")};
    gridView0->set_columns({"Last", "First", "Age", "City"});
    gridView0->add_row({"Smith", "John", "28", "New York"});
    gridView0->add_row({"Johnson", "Emily", "35", "Los Angeles"});
    gridView0->add_row({"Brown", "Michael", "22", "Chicago"});
    gridView0->add_row({"Davis", "Sarah", "40", "Houston"});
    gridView0->add_row({"Patel", "Raj", "32", "San Francisco"});

    button0->Click.connect([=]() {
        progressBar0->Value = progressBar0->Value == 100
            ? 0
            : progressBar0->Value + 10;
        listbox0->add_item(std::to_string(progressBar0->Value));
        dropDownList0->add_item(std::to_string(progressBar0->Value));
        gridView0->add_row({"XXX", "XX", std::to_string(progressBar0->Value * 10), "XXXXX"});
    });

    locate_service<input::system>().CurrentInputMode.Changed.connect([retValue, wnd](input::mode mode) {
        if (mode == input::mode::Controller) {
            retValue->find_widget_by_name("gridB0")->focus();
            wnd->Cursor = nullptr;
            wnd->hide_system_cursor(true);
        } else {
            auto* resGrp {locate_service<assets::library>().get_group("ui")};
            auto  defaultCursor {resGrp->get<cursor>("default")};
            wnd->Cursor               = defaultCursor;
            defaultCursor->ActiveMode = "default";
        }
    });

    // nav map
    auto& navMap {*retValue->NavMap};
    navMap["gridB0"] = {.Left = "Slider0", .Right = "gridB1", .Down = "gridB2"};
    navMap["gridB1"] = {.Left = "gridB0", .Down = "gridB2"};
    navMap["gridB2"] = {.Up = "gridB0", .Down = "gridB3"};
    navMap["gridB3"] = {.Up = "gridB2", .Right = "gridB4"};
    navMap["gridB4"] = {.Left = "gridB3", .Up = "gridB2", .Right = "gridB5"};
    navMap["gridB5"] = {.Left = "gridB4", .Up = "gridB2", .Right = "gridB6"};
    navMap["gridB6"] = {.Left = "gridB5", .Up = "gridB2", .Down = "Check0"};
    navMap["Check0"] = {.Up = "gridB6", .Right = "Check1"};
    navMap["Check1"] = {.Left = "Check0", .Right = "Check2"};
    navMap["Check2"] = {.Left = "Check1", .Right = "Check3"};
    navMap["Check3"] = {.Left = "Check2"};

    return retValue;
}

////////////////////////////////////////////////////////////

auto create_form1(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form1", wnd)};

    auto panel0 {retValue->create_container<panel>(dock_style::Fill, "Panel0")};
    panel0->Flex          = {100_pct, 100_pct};
    panel0->ScrollEnabled = true;
    auto panel0Layout {panel0->get_layout<fixed_layout>()};

    auto panel1 {panel0Layout->create_widget<panel>({50, 30, 400, 400}, "Panel1")};
    panel1->ScrollEnabled = true;
    auto panel1Layout {panel1->get_layout<fixed_layout>()};

    auto panel2 {panel1Layout->create_widget<panel>({50, 30, 400, 400}, "Panel2")};
    panel2->ScrollEnabled = true;
    auto panel2Layout {panel2->get_layout<fixed_layout>()};

    auto button0 {panel2Layout->create_widget<button>({-10, -10, 30, 30}, "Button0")};
    auto button1 {panel2Layout->create_widget<button>({350, 350, 80, 50}, "Button1")};

    // styles
    return retValue;
}

auto create_form_displays(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form2", wnd)};

    auto panel0 {retValue->create_container<panel>(dock_style::Fill, "Panel0")};
    panel0->Flex = {100_pct, 100_pct};
    auto panel0Layout {panel0->get_layout<fixed_layout>()};
    (*panel0->TabStop).Enabled = false;
    {
        size_i const termSize {80, 24};
        auto         terminal0 {panel0Layout->create_widget<terminal>({5, 5, termSize.Width * 14.f, termSize.Height * 28.f}, "Term1")};
        terminal0->Size = termSize;
        terminal0->clear();

        terminal0->add_str("\033[106;34mC\033[105;33mo\033[104;32ml\033[103;31mo\033[102;31mr\033[0m Te\033[31;40mst ");
        terminal0->color_set(color::FromString("#008f11"), colors::White);
        terminal0->add_str("\nThis is line 2");
        terminal0->add_str({79, 23}, "x");
        terminal0->move({20, 10});
        terminal0->add_str("st\033[4CTe");
        terminal0->move({40, 10});
        terminal0->add_str("st\033[4CTe");

        terminal0->curs_set(true);
        terminal0->mouse_set(true);
        terminal0->echo(true);

        auto button0 {panel0Layout->create_widget<button>({1150, 450, 200, 100}, "Button0")};
        button0->Label = "Button0";
        auto button1 {panel0Layout->create_widget<button>({1150, 650, 200, 100}, "Button1")};
        button1->Label = "Button1";

        button0->Click.connect([terminal0](auto const&) {
            terminal0->rectangle({terminal0->get_xy(), {10, 5}});
        });
        button1->Click.connect([terminal0](auto const&) {
            {
                io::ifstream str {"term.dump"};
                auto         _ = terminal0->restore(str);
            }
            terminal0->flash();
        });
    }
    {
        auto canvas {panel0Layout->create_widget<canvas_widget>({5, 700, 200, 200}, "Canvas1")};
        canvas->set_fill_style(colors::Blue);
        canvas->begin_path();
        canvas->rect({0, 0, 100, 100});
        canvas->fill();

        canvas->set_fill_style(colors::Red);
        canvas->begin_path();
        canvas->rect({50, 50, 100, 100});
        canvas->fill();

        canvas->set_fill_style(colors::Green);
        canvas->begin_path();
        canvas->rect({150, 150, 100, 100});
        canvas->fill();
    }

    {
        auto dotMatrix {panel0Layout->create_widget<dot_matrix_display>({1150, 60, 240, 120}, "DM1")};
        dotMatrix->Size = {40, 20};
        rng              rand;
        std::vector<u16> dots;
        dots.reserve(dotMatrix->Size->Width * dotMatrix->Size->Height);
        for (i32 i {0}; i < dots.capacity(); ++i) {
            dots.push_back(rand(0, 2));
        }
        dotMatrix->Dots = dots;
    }

    {
        auto lcdDisplay0 {panel0Layout->create_widget<seven_segment_display>({1150, 200, 350, 70}, "LCD0")};
        lcdDisplay0->Text = "0123456789 -=\"',";
        auto lcdDisplay1 {panel0Layout->create_widget<seven_segment_display>({1150, 270, 350, 70}, "LCD1")};
        lcdDisplay1->Text = "ABCDEFGHIJLOPSUZ";
    }

    // styles
    return retValue;
}

auto create_form_colorpicker(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form2", wnd)};

    auto panel0 {retValue->create_container<panel>(dock_style::Fill, "Panel0")};
    panel0->Flex = {25_pct, 25_pct};
    auto panel0Layout {panel0->get_layout<fixed_layout>()};
    auto colorPicker00 {panel0Layout->create_widget<color_picker>({5, 5, 300, 200}, "CP1")};
    colorPicker00->SelectedColor.Changed.connect([wnd](auto val) {
        wnd->ClearColor = val;
    });

    // styles
    return retValue;
}

auto create_form_tabcontainer(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form3", wnd)};

    auto createTabs = [](std::shared_ptr<tab_container> const& tabContainer0) {
        auto boxVPanel0 {tabContainer0->create_tab<panel>("SPanel0")};
        (*boxVPanel0->TabStop).Enabled = false;
        auto boxVLayout0 {boxVPanel0->create_layout<vbox_layout>()};
        for (i32 i = 0; i < 4; i++) {
            auto bb {boxVLayout0->create_widget<button>("Button" + std::to_string(i))};
            bb->Label = std::to_string(i);
        }

        auto gridPanel {tabContainer0->create_tab<panel>("SPanel1")};
        (*gridPanel->TabStop).Enabled = false;
        auto gridLayout {gridPanel->create_layout<grid_layout>()};

        auto createGridWidget {
            [&](rect_i loc, std::string const& name) {
                auto b {gridLayout->create_widget<button>(loc, "gridB" + name)};
                b->Label = name;
            }};
        createGridWidget({0, 0, 2, 1}, "0");
        createGridWidget({2, 0, 2, 1}, "1");
        createGridWidget({0, 1, 4, 1}, "2");
        createGridWidget({0, 2, 1, 1}, "3");
        createGridWidget({1, 2, 1, 1}, "4");
        createGridWidget({2, 2, 1, 1}, "5");
        createGridWidget({3, 2, 1, 1}, "6");

        auto boxHPanel1 {tabContainer0->create_tab<panel>("SPanel2")};
        (*boxHPanel1->TabStop).Enabled = false;
        auto boxHLayout1 {boxHPanel1->create_layout<hbox_layout>()};
        for (i32 i = 0; i < 4; i++) {
            boxHLayout1->create_widget<checkbox>("Check" + std::to_string(i));
        }
    };

    {
        auto tabContainer0 {retValue->create_container<tab_container>(dock_style::Left, "TabContainer0")};
        tabContainer0->Flex = {50_pct, 100_pct};
        createTabs(tabContainer0);
    }

    {
        auto tabContainer0 {retValue->create_container<tab_container>(dock_style::Left, "TabContainer0")};
        tabContainer0->Flex = {50_pct, 100_pct};

        auto tabContainer1 {tabContainer0->create_tab<tab_container>("TabContainer1")};
        createTabs(tabContainer1);

        auto tabContainer2 {tabContainer0->create_tab<tab_container>("TabContainer2")};
        createTabs(tabContainer2);

        auto tabContainer3 {tabContainer0->create_tab<tab_container>("TabContainer3")};
        createTabs(tabContainer3);
    }

    return retValue;
}

auto create_form_accordion(window* wnd) -> std::shared_ptr<form>
{
    auto retValue {std::make_shared<form>("form4", wnd)};

    auto createSections = [](std::shared_ptr<accordion> const& accordion0) {
        auto boxVPanel0 {accordion0->create_section<panel>("SPanel0")};
        (*boxVPanel0->TabStop).Enabled = false;
        auto boxVLayout0 {boxVPanel0->create_layout<vbox_layout>()};
        for (i32 i = 0; i < 4; i++) {
            auto bb {boxVLayout0->create_widget<button>("Button" + std::to_string(i))};
            bb->Label = std::to_string(i);
        }

        auto gridPanel {accordion0->create_section<panel>("SPanel1")};
        (*gridPanel->TabStop).Enabled = false;
        auto gridLayout {gridPanel->create_layout<grid_layout>()};

        auto createGridWidget {
            [&](rect_i loc, std::string const& name) {
                auto b {gridLayout->create_widget<button>(loc, "gridB" + name)};
                b->Label = name;
            }};
        createGridWidget({0, 0, 2, 1}, "0");
        createGridWidget({2, 0, 2, 1}, "1");
        createGridWidget({0, 1, 4, 1}, "2");
        createGridWidget({0, 2, 1, 1}, "3");
        createGridWidget({1, 2, 1, 1}, "4");
        createGridWidget({2, 2, 1, 1}, "5");
        createGridWidget({3, 2, 1, 1}, "6");

        auto boxHPanel1 {accordion0->create_section<panel>("SPanel2")};
        (*boxHPanel1->TabStop).Enabled = false;
        auto boxHLayout1 {boxHPanel1->create_layout<hbox_layout>()};
        for (i32 i = 0; i < 4; i++) {
            boxHLayout1->create_widget<checkbox>("Check" + std::to_string(i));
        }
    };

    {
        auto accordion0 {retValue->create_container<accordion>(dock_style::Left, "Accordion0")};
        accordion0->Flex = {30_pct, 100_pct};
        createSections(accordion0);
    }

    {
        auto panel0 {retValue->create_container<panel>(dock_style::Left, "Panel0")};
        panel0->Flex          = {30_pct, 100_pct};
        panel0->ScrollEnabled = true;
        auto panel0Layout {panel0->get_layout<fixed_layout>()};

        auto accordion0 {panel0Layout->create_widget<accordion>({0, 0, 500, 500}, "Accordion0")};
        createSections(accordion0);
    }

    {
        auto accordion0 {retValue->create_container<accordion>(dock_style::Left, "Accordion0")};
        accordion0->Flex = {30_pct, 100_pct};

        auto accordion1 {accordion0->create_section<accordion>("Accordion1")};
        createSections(accordion1);

        auto accordion2 {accordion0->create_section<accordion>("Accordion2")};
        createSections(accordion2);

        auto accordion3 {accordion0->create_section<accordion>("Accordion3")};
        createSections(accordion3);
    }

    return retValue;
}
