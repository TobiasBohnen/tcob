#include "tests.hpp"

TEST_CASE("GFX.UI.Styles")
{
    flags fl_hover {.Focus = false, .Active = false, .Hover = true, .Checked = false, .Disabled = false};
    flags fl_focus {.Focus = true, .Active = false, .Hover = false, .Checked = false, .Disabled = false};
    flags fl_focus_hover {.Focus = true, .Active = false, .Hover = true, .Checked = false, .Disabled = false};
    flags fl_active {.Focus = false, .Active = true, .Hover = false, .Checked = false, .Disabled = false};
    flags fl_active_hover {.Focus = false, .Active = true, .Hover = true, .Checked = false, .Disabled = false};

    std::string name = "button";

    SUBCASE("test1")
    {
        style_collection s;
        // button
        auto             button  = s.create<background_style>(name, {});
        button->Background       = colors::Black;
        // button:hover
        auto button_hover        = s.create<background_style>(name, fl_hover);
        button_hover->Background = colors::White;

        // button:focus > button
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_focus, {}))->Background) == std::get<color>(button->Background));
        // button:focus:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_focus_hover, {}))->Background) == std::get<color>(button_hover->Background));
        // button:active:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_active_hover, {}))->Background) == std::get<color>(button_hover->Background));
    }
    SUBCASE("test2")
    {
        style_collection s;
        // button
        auto             button        = s.create<background_style>(name, {});
        button->Background             = colors::Black;
        // button:focus:hover
        auto button_focus_hover        = s.create<background_style>(name, fl_focus_hover);
        button_focus_hover->Background = colors::Red;

        // button:focus > button
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_focus, {}))->Background) == std::get<color>(button->Background));
        // button:hover > button
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_hover, {}))->Background) == std::get<color>(button->Background));
        // button:focus:hover >  button:focus:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_focus_hover, {}))->Background) == std::get<color>(button_focus_hover->Background));
        // button:active:hover > button
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_active_hover, {}))->Background) == std::get<color>(button->Background));
    }
    SUBCASE("test3")
    {
        style_collection s;
        // button
        auto             button   = s.create<background_style>(name, {});
        button->Background        = colors::Black;
        // button:active
        auto button_active        = s.create<background_style>(name, fl_active);
        button_active->Background = colors::White;
        // button:focus
        auto button_focus         = s.create<background_style>(name, fl_focus);
        button_focus->Background  = colors::Red;
        // button:hover
        auto button_hover         = s.create<background_style>(name, fl_hover);
        button_hover->Background  = colors::Yellow;

        // button:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_hover, {}))->Background) == std::get<color>(button_hover->Background));
        // button:focus:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_focus_hover, {}))->Background) == std::get<color>(button_hover->Background));
        // button:active:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<background_style*>(s.get(name, fl_active_hover, {}))->Background) == std::get<color>(button_hover->Background));
    }

    SUBCASE("attributes 1")
    {
        style_collection s;
        auto             style {s.create<button>(name, {})};
        style->Background = color {1, 1, 1, 1};

        auto hoverStyle {s.create<button>(name, fl_hover)};
        *hoverStyle       = *style;
        style->Background = color {3, 3, 3, 3};

        auto incHoverStyle {s.create<button>(name, fl_hover, {{"inc_button", true}})};
        *incHoverStyle            = *hoverStyle;
        incHoverStyle->Background = color {5, 5, 5, 5};
        auto incActiveStyle {s.create<button>(name, fl_active, {{"inc_button", true}})};
        *incActiveStyle            = *hoverStyle;
        incActiveStyle->Background = color {6, 6, 6, 6};

        auto decHoverStyle {s.create<button>(name, fl_hover, {{"dec_button", true}})};
        *decHoverStyle = *hoverStyle;
        auto decActiveStyle {s.create<button>(name, fl_active, {{"dec_button", true}})};
        *decActiveStyle = *hoverStyle;

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_hover, {{"inc_button", true}}))->Background)
                == std::get<color>(incHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_active, {{"inc_button", true}}))->Background)
                == std::get<color>(incActiveStyle->Background));

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_hover, {{"dec_button", true}}))->Background)
                == std::get<color>(decHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_active, {{"dec_button", true}}))->Background)
                == std::get<color>(decActiveStyle->Background));
    }

    SUBCASE("attributes 2")
    {
        style_collection s;
        auto             style {s.create<button>(name, {})};
        style->Background = color {1, 1, 1, 1};

        auto hoverStyle {s.create<button>(name, fl_hover)};
        *hoverStyle       = *style;
        style->Background = color {3, 3, 3, 3};

        auto incHoverStyle {s.create<button>(name, fl_hover, {{"value", 80}, {"value", 100}})};
        *incHoverStyle            = *hoverStyle;
        incHoverStyle->Background = color {5, 5, 5, 5};
        auto incActiveStyle {s.create<button>(name, fl_active, {{"value", 80}, {"value", 100}})};
        *incActiveStyle            = *hoverStyle;
        incActiveStyle->Background = color {6, 6, 6, 6};

        auto decHoverStyle {s.create<button>(name, fl_hover, {{"value", 40}, {"value", 60}})};
        *decHoverStyle = *hoverStyle;
        auto decActiveStyle {s.create<button>(name, fl_active, {{"value", 40}, {"value", 60}})};
        *decActiveStyle = *hoverStyle;

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_hover, {{"value", 80}}))->Background)
                == std::get<color>(incHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_active, {{"value", 100}}))->Background)
                == std::get<color>(incActiveStyle->Background));

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_hover, {{"value", 40}}))->Background)
                == std::get<color>(decHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_active, {{"value", 60}}))->Background)
                == std::get<color>(decActiveStyle->Background));

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get(name, fl_hover, {{"value", 50}}))->Background)
                == std::get<color>(hoverStyle->Background));
    }
}
