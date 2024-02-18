#include "tests.hpp"

TEST_CASE("GFX.UI.Bounds")
{
    using namespace tcob::literals;

    form form0 {"", nullptr, {60, 80, 800, 700}};
    auto panel0 {form0.create_container<panel>(dock_style::Fill, "Panel0")};
    auto panel0Layout {panel0->get_layout<fixed_layout>()};
    panel0->set_scroll_offset({10, 20});
    panel0->Flex = {100_pct, 100_pct};
    auto button0 {panel0Layout->create_widget<button>({10, 20, 200, 100}, "button0")};

    auto cPanel0 {panel0Layout->create_widget<panel>({230, 280, 250, 500}, "cPanel0")};
    cPanel0->set_scroll_offset({5, 10});
    auto cPanel0Layout0 {cPanel0->create_layout<fixed_layout>()};
    auto cButton0 {cPanel0Layout0->create_widget<button>({5, 50, 200, 100}, "button0")};

    style_collection& styles {*form0.Styles};

    auto buttonStyle {styles.create<button>("button", {})};
    buttonStyle->Border.Size = 2_px;
    buttonStyle->Margin      = {4_px};
    buttonStyle->Padding     = {8_px};

    auto panelStyle {styles.create<panel>("panel", {})};
    panelStyle->Border.Size = 10_px;
    panelStyle->Margin      = {20_px};
    panelStyle->Padding     = {40_px};

    form0.fixed_update(milliseconds {0});

    REQUIRE(panel0->get_global_position() == point_f {60, 80});
    REQUIRE(button0->get_global_position() == point_f {125, 145});
    REQUIRE(cPanel0->get_global_position() == point_f {345, 405});
    REQUIRE(cButton0->get_global_position() == point_f {410, 510});

    REQUIRE(panel0->get_content_bounds() == rect_f {65, 65, 670, 570});
    REQUIRE(button0->get_content_bounds() == rect_f {23, 33, 174, 74});
    REQUIRE(cPanel0->get_content_bounds() == rect_f {295, 345, 120, 370});
    REQUIRE(cButton0->get_content_bounds() == rect_f {18, 63, 174, 74});

    REQUIRE(panel0->get_global_content_bounds() == rect_f {125, 145, 670, 570});
    REQUIRE(button0->get_global_content_bounds() == rect_f {138, 158, 174, 74});
    REQUIRE(cPanel0->get_global_content_bounds() == rect_f {410, 470, 120, 370});
    REQUIRE(cButton0->get_global_content_bounds() == rect_f {423, 523, 174, 74});

    REQUIRE(panel0->get_hit_test_bounds() == rect_f {80, 100, 760, 660});
    REQUIRE(button0->get_hit_test_bounds() == rect_f {129, 149, 192, 92});
    REQUIRE(cPanel0->get_hit_test_bounds() == rect_f {365, 425, 210, 290});
    REQUIRE(cButton0->get_hit_test_bounds() == rect_f {414, 514, 116, 92});
}
