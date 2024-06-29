#include "ExStyle_Skin.hpp"

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

skin::skin(std::string const& color, assets::group* resGrp)
    : Button00 {.Texture = resGrp->get<texture>(color + "_button00"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 49.f, 185 / 190.f, 40 / 49.f)}
    , Button01 {.Texture = resGrp->get<texture>(color + "_button01"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 45.f, 185 / 190.f, 40 / 45.f)}
    , Button02 {.Texture = resGrp->get<texture>(color + "_button02"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 49.f, 185 / 190.f, 40 / 49.f)}
    , Button03 {.Texture = resGrp->get<texture>(color + "_button03"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 45.f, 185 / 190.f, 40 / 45.f)}
    , Button04 {.Texture = resGrp->get<texture>(color + "_button04"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 49.f, 185 / 190.f, 40 / 49.f)}
    , Button05 {.Texture = resGrp->get<texture>(color + "_button05"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 45.f, 185 / 190.f, 40 / 45.f)}
    , Button06 {.Texture = resGrp->get<texture>(color + "_button06"), .UV = rect_f::FromLTRB(5 / 49.f, 5 / 45.f, 43 / 49.f, 40 / 49.f)}
    , Button13 {.Texture = resGrp->get<texture>(color + "_button13"), .UV = rect_f::FromLTRB(5 / 190.f, 5 / 49.f, 185 / 190.f, 40 / 49.f)}
    , BoxCheckmark {.Texture = resGrp->get<texture>(color + "_boxCheckmark"), .UV = rect_f::FromLTRB(6 / 38.f, 5 / 36.f, 31 / 38.f, 31 / 36.f)}
    , BoxCross {.Texture = resGrp->get<texture>(color + "_boxCross"), .UV = rect_f::FromLTRB(6 / 38.f, 5 / 36.f, 31 / 38.f, 31 / 36.f)}
    , BoxTick {.Texture = resGrp->get<texture>(color + "_boxTick"), .UV = {0, 0, 1, 1}}
    , Circle {.Texture = resGrp->get<texture>(color + "_circle"), .UV = {0, 0, 1, 1}}
    , Checkmark {.Texture = resGrp->get<texture>(color + "_checkmark"), .UV = {0, 0, 1, 1}}
    , Cross {.Texture = resGrp->get<texture>(color + "_cross"), .UV = {0, 0, 1, 1}}
    , SliderUp {.Texture = resGrp->get<texture>(color + "_sliderUp"), .UV = {0, 0, 1, 1}}
    , SliderDown {.Texture = resGrp->get<texture>(color + "_sliderDown"), .UV = {0, 0, 1, 1}}
    , SliderRight {.Texture = resGrp->get<texture>(color + "_sliderRight"), .UV = {0, 0, 1, 1}}
    , Panel {.Texture = resGrp->get<texture>(color + "_panel"), .UV = rect_f::FromLTRB(5 / 100.f, 5 / 100.f, 95 / 100.f, 95 / 100.f)}
{
}

auto create_skinned_styles() -> style_collection
{
    using namespace tcob::literals;
    auto* resGrp {locate_service<assets::library>().get_group("ui")};

    skin blue {"blue", resGrp};
    skin green {"green", resGrp};
    skin grey {"grey", resGrp};
    skin red {"red", resGrp};
    skin yellow {"yellow", resGrp};

    style_collection retValue;
    {
        auto style {retValue.create<button>("button", {})};
        style->Background          = green.Button00;
        style->Border.Size         = 5_px;
        style->Text.Style          = {false, font::weight::Normal};
        style->Text.Font           = resGrp->get<font_family>("Poppins");
        style->Text.Size           = 50_pct;
        style->Text.Color          = colors::Black;
        style->Text.Shadow.Color   = colors::White;
        style->Text.Shadow.OffsetX = 0_px;
        style->Text.Shadow.OffsetY = 1_px;
        style->Text.Alignment      = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->Margin              = {5_px};
        style->Padding             = {2_px};

        auto disabledStyle {retValue.create<button>("button", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;

        auto hoverStyle {retValue.create<button>("button", {.Hover = true})};
        *hoverStyle                = *style;
        hoverStyle->Background     = blue.Button02;
        hoverStyle->Text.Transform = element::text::transform::Lowercase;

        auto focusStyle {retValue.create<button>("button", {.Focus = true})};
        *focusStyle = *style;

        auto focusHoverStyle {retValue.create<button>("button", {.Focus = true, .Hover = true})};
        *focusHoverStyle            = *focusStyle;
        focusHoverStyle->Background = blue.Button02;

        auto focusHoverActiveStyle {retValue.create<button>("button", {.Focus = true, .Active = true, .Hover = true})};
        *focusHoverActiveStyle            = *focusStyle;
        focusHoverActiveStyle->Background = blue.Button03;
        focusHoverActiveStyle->Text.Color = colors::White;
    }
    {
        auto style {retValue.create<label>("label", {})};
        style->Text.Style     = {false, font::weight::Normal};
        style->Text.Font      = resGrp->get<font_family>("Poppins");
        style->Text.Size      = 50_pct;
        style->Text.Color     = colors::Black;
        style->Text.Alignment = {horizontal_alignment::Left, vertical_alignment::Middle};
    }
    {
        auto style {retValue.create<checkbox>("checkbox", {})};
        style->Background  = blue.BoxCross;
        style->Border.Size = 5_px;
        style->Margin      = {5_px};
        style->Padding     = {2_px};
        style->Tick.Type   = element::tick::type::None;
        style->Tick.Size   = 100_pct;

        auto disabledStyle {retValue.create<checkbox>("checkbox", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;

        auto checkedStyle {retValue.create<checkbox>("checkbox", {.Checked = true})};
        *checkedStyle            = *style;
        checkedStyle->Background = blue.BoxCheckmark;
    }
    {
        auto style {retValue.create<radio_button>("radio_button", {})};
        style->Background = blue.Circle;
        style->Margin     = {5_px};
        style->Padding    = {5_px};
        style->Cursor     = "cursor2";
        style->Tick.Type  = element::tick::type::None;

        auto disabledStyle {retValue.create<radio_button>("radio_button", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;

        auto checkedStyle {retValue.create<radio_button>("radio_button", {.Checked = true})};
        *checkedStyle            = *style;
        checkedStyle->Background = blue.BoxTick;
    }
    {
        auto style {retValue.create<toggle>("toggle", {})};
        style->Background      = blue.Button13;
        style->Border.Size     = 3_px;
        style->Margin          = {5_px};
        style->Padding         = {2_px};
        style->Delay           = 500ms;
        style->Tick.Foreground = red.Cross;
        style->Tick.Type       = element::tick::type::None;

        auto checkedStyle {retValue.create<toggle>("toggle", {.Checked = true})};
        *checkedStyle                 = *style;
        checkedStyle->Tick.Foreground = green.Checkmark;
        checkedStyle->Tick.Type       = element::tick::type::None;

        auto disabledStyle {retValue.create<toggle>("toggle", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto hstyle {retValue.create<slider>("slider", {}, {{"orientation", orientation::Horizontal}})};
        hstyle->Margin               = {5_px};
        hstyle->Padding              = {2_px, 15_px};
        hstyle->ThumbClass           = "h_slider_thumb";
        hstyle->Bar.Type             = element::bar::type::Continuous;
        hstyle->Bar.Size             = 50_pct;
        hstyle->Bar.LowerBackground  = blue.Panel;
        hstyle->Bar.HigherBackground = blue.Panel;
        hstyle->Bar.Border.Size      = 5_px;
        hstyle->Bar.Delay            = 200ms;

        auto vstyle {retValue.create<slider>("slider", {}, {{"orientation", orientation::Vertical}})};
        *vstyle            = *hstyle;
        vstyle->ThumbClass = "v_slider_thumb";

        auto disabledStyle {retValue.create<slider>("slider", {.Disabled = true})};
        disabledStyle->Bar.LowerBackground  = colors::FireBrick;
        disabledStyle->Bar.HigherBackground = colors::FireBrick;
    }
    {
        auto style {retValue.create<progress_bar>("progress_bar", {})};
        style->Background           = blue.Button13;
        style->Border.Size          = 3_px;
        style->Margin               = {5_px};
        style->Padding              = {5_px, 5_px};
        style->Bar.Type             = element::bar::type::Continuous;
        style->Bar.LowerBackground  = blue.Button00;
        style->Bar.HigherBackground = blue.Button13;
        style->Bar.Border           = style->Border;
        style->Bar.Delay            = 250ms;

        auto disabledStyle {retValue.create<progress_bar>("progress_bar", {.Disabled = true})};
        disabledStyle->Background           = colors::FireBrick;
        disabledStyle->Bar.LowerBackground  = colors::FireBrick;
        disabledStyle->Bar.HigherBackground = colors::FireBrick;
    }
    {
        auto style {retValue.create<spinner>("spinner", {})};
        style->Background          = green.Button00;
        style->Border.Size         = 5_px;
        style->Text.Style          = {false, font::weight::Normal};
        style->Text.Font           = resGrp->get<font_family>("Poppins");
        style->Text.Size           = 50_pct;
        style->Text.Color          = colors::Black;
        style->Text.Shadow.Color   = colors::White;
        style->Text.Shadow.OffsetX = 0_px;
        style->Text.Shadow.OffsetY = 1_px;
        style->Text.Alignment      = {horizontal_alignment::Left, vertical_alignment::Middle};
        style->Margin              = {5_px};
        style->Padding             = {5_px};
        style->NavArrowClass       = "nav_arrows";

        auto disabledStyle {retValue.create<spinner>("spinner", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<cycle_button>("cycle_button", {})};
        style->Background          = blue.Button00;
        style->Border.Size         = 3_px;
        style->Text.Style          = {false, font::weight::Normal};
        style->Text.Font           = resGrp->get<font_family>("Poppins");
        style->Text.Size           = 50_pct;
        style->Text.Color          = colors::Black;
        style->Text.Shadow.Color   = colors::White;
        style->Text.Shadow.OffsetX = 0_px;
        style->Text.Shadow.OffsetY = 1_px;
        style->Text.Alignment      = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->Margin              = {5_px};
        style->Padding             = {5_px};

        auto disabledStyle {retValue.create<cycle_button>("cycle_button", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<image_box>("image_box", {})};
        style->Background       = blue.Panel;
        style->Border.Size      = 3_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};
        style->Fit              = image_box::fit_mode::Contain;
        style->Alignment        = alignments {horizontal_alignment::Centered, vertical_alignment::Middle};

        auto disabledStyle {retValue.create<image_box>("image_box", {.Disabled = true})};
        disabledStyle->Background = colors::Green;
    }
    {
        auto style {retValue.create<panel>("panel", {})};
        style->Background       = grey.Panel;
        style->Border.Size      = 3_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};

        style->VScrollBar.Bar.Type             = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size             = 30_px;
        style->VScrollBar.Bar.LowerBackground  = yellow.Panel;
        style->VScrollBar.Bar.HigherBackground = yellow.Panel;
        style->VScrollBar.Bar.Delay            = 250ms;
        style->VScrollBar.ThumbClass           = "v_scrollbar_thumb";
        style->HScrollBar                      = style->VScrollBar;
        style->HScrollBar.ThumbClass           = "h_scrollbar_thumb";

        auto disabledStyle {retValue.create<panel>("panel", {.Disabled = true})};
        disabledStyle->Background = colors::Green;
    }
    {
        auto style {retValue.create<tooltip>("tooltip", {})};
        style->Background  = yellow.Panel;
        style->Border.Size = 5_px;
        style->Delay       = 1000ms;
        style->FadeIn      = 250ms;
    }
    {
        auto style {retValue.create<list_box>("list_box", {})};
        style->Background                      = grey.Panel;
        style->Border.Size                     = 3_px;
        style->Margin                          = {5_px};
        style->Padding                         = {5_px};
        style->DropShadow.Color                = color {0, 0, 0, 128};
        style->ItemHeight                      = 20_pct;
        style->ItemClass                       = "list_items";
        style->VScrollBar.ThumbClass           = "v_scrollbar_thumb";
        style->VScrollBar.Bar.Type             = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size             = 30_px;
        style->VScrollBar.Bar.LowerBackground  = yellow.Panel;
        style->VScrollBar.Bar.HigherBackground = yellow.Panel;
        style->VScrollBar.Bar.Delay            = 250ms;

        auto disabledStyle {retValue.create<list_box>("list_box", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<drop_down_list>("drop_down_list", {})};
        style->Background                      = grey.Panel;
        style->Border.Size                     = 3_px;
        style->Margin                          = {5_px};
        style->Padding                         = {5_px};
        style->DropShadow.Color                = color {0, 0, 0, 128};
        style->Text.Style                      = {false, font::weight::Normal};
        style->Text.Font                       = resGrp->get<font_family>("Poppins");
        style->Text.Size                       = 50_pct;
        style->Text.Shadow.OffsetX             = 0_px;
        style->Text.Shadow.OffsetY             = 1_px;
        style->Text.Alignment                  = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->ItemHeight                      = 150_pct;
        style->ItemClass                       = "list_items";
        style->VScrollBar.ThumbClass           = "v_scrollbar_thumb";
        style->VScrollBar.Bar.Type             = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size             = 30_px;
        style->VScrollBar.Bar.LowerBackground  = yellow.Panel;
        style->VScrollBar.Bar.HigherBackground = yellow.Panel;
        style->VScrollBar.Bar.Delay            = 250ms;

        auto disabledStyle {retValue.create<drop_down_list>("drop_down_list", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<grid_view>("grid_view", {})};
        style->Background                      = grey.Panel;
        style->Border.Size                     = 3_px;
        style->Margin                          = {5_px};
        style->Padding                         = {5_px};
        style->DropShadow.Color                = color {0, 0, 0, 128};
        style->RowHeight                       = 20_pct;
        style->HeaderClass                     = "header_items";
        style->RowClass                        = "row_items";
        style->VScrollBar.ThumbClass           = "v_scrollbar_thumb";
        style->VScrollBar.Bar.Type             = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size             = 30_px;
        style->VScrollBar.Bar.LowerBackground  = yellow.Panel;
        style->VScrollBar.Bar.HigherBackground = yellow.Panel;
        style->VScrollBar.Bar.Delay            = 250ms;

        auto disabledStyle {retValue.create<grid_view>("grid_view", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<tab_container>("tab_container", {})};
        style->Background       = grey.Panel;
        style->Border.Size      = 5_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};
        style->TabBarHeight     = 10_pct;
        style->TabItemClass     = "tab_items";
        style->TabBarPosition   = tab_container::position::Bottom;

        auto disabledStyle {retValue.create<tab_container>("tab_container", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<accordion>("accordion", {})};
        style->Background       = grey.Panel;
        style->Border.Size      = 5_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};
        style->SectionBarHeight = 10_pct;
        style->SectionItemClass = "section_items";

        auto disabledStyle {retValue.create<accordion>("accordion", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    {
        auto style {retValue.create<text_box>("text_box", {})};
        style->Background          = blue.Button13;
        style->Border.Size         = 5_px;
        style->Text.Style          = {false, font::weight::Normal};
        style->Text.Font           = resGrp->get<font_family>("Poppins");
        style->Text.Size           = 50_pct;
        style->Text.Color          = colors::Black;
        style->Text.Shadow.Color   = colors::White;
        style->Text.Shadow.OffsetX = 0_px;
        style->Text.Shadow.OffsetY = 1_px;
        style->Text.Alignment      = {horizontal_alignment::Left, vertical_alignment::Middle};
        style->Margin              = {5_px};
        style->Padding             = {2_px};
        style->Caret.BlinkRate     = 500ms;
        style->Caret.Color         = colors::DarkOliveGreen;

        auto disabledStyle {retValue.create<text_box>("text_box", {.Disabled = true})};
        disabledStyle->Background = colors::FireBrick;
    }
    // THUMBS
    {
        auto style {retValue.create<thumb_style>("v_slider_thumb", {}, {})};
        style->Thumb.Type       = element::thumb::type::Rect;
        style->Thumb.LongSide   = 25_pct;
        style->Thumb.ShortSide  = 80_pct;
        style->Thumb.Background = blue.SliderRight;

        auto hoveredStyle {retValue.create<thumb_style>("v_slider_thumb", {.Hover = true})};
        hoveredStyle->Thumb = style->Thumb;

        auto activeStyle {retValue.create<thumb_style>("v_slider_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;
    }
    {
        auto style {retValue.create<thumb_style>("h_slider_thumb", {}, {})};
        style->Thumb.Type       = element::thumb::type::Rect;
        style->Thumb.LongSide   = 25_pct;
        style->Thumb.ShortSide  = 80_pct;
        style->Thumb.Background = blue.SliderUp;

        auto hoveredStyle {retValue.create<thumb_style>("h_slider_thumb", {.Hover = true})};
        hoveredStyle->Thumb = style->Thumb;

        auto activeStyle {retValue.create<thumb_style>("h_slider_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;
    }
    {
        auto style {retValue.create<thumb_style>("v_scrollbar_thumb", {}, {})};
        style->Thumb.Background = blue.SliderRight;
        style->Thumb.Type       = element::thumb::type::Rect;
        style->Thumb.LongSide   = 5_pct;
        style->Thumb.ShortSide  = 80_pct;

        auto hoveredStyle {retValue.create<thumb_style>("v_scrollbar_thumb", {.Hover = true})};
        hoveredStyle->Thumb = style->Thumb;

        auto activeStyle {retValue.create<thumb_style>("v_scrollbar_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;
    }
    {
        auto style {retValue.create<thumb_style>("h_scrollbar_thumb", {}, {})};
        style->Thumb.Background = blue.SliderUp;
        style->Thumb.Type       = element::thumb::type::Rect;
        style->Thumb.LongSide   = 5_pct;
        style->Thumb.ShortSide  = 80_pct;

        auto hoveredStyle {retValue.create<thumb_style>("h_scrollbar_thumb", {.Hover = true})};
        hoveredStyle->Thumb = style->Thumb;

        auto activeStyle {retValue.create<thumb_style>("h_scrollbar_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;
    }
    // NAV ARROWS
    {
        auto style {retValue.create<nav_arrows_style>("nav_arrows", {}, {})};
        style->NavArrow.Type          = element::nav_arrow::type::None;
        style->NavArrow.Size.Height   = {0.75f, length::type::Relative};
        style->NavArrow.Size.Width    = {0.25f, length::type::Relative};
        style->NavArrow.IncBackground = blue.SliderUp;
        style->NavArrow.DecBackground = blue.SliderDown;
    }
    // TAB ITEMS
    {
        auto style {retValue.create<item_style>("tab_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Background     = blue.Button00;
        style->Item.Text.Style     = {false, font::weight::Normal};
        style->Item.Text.Font      = resGrp->get<font_family>("Poppins");
        style->Item.Text.Size      = 25_pct;
        style->Item.Text.Color     = colors::Black;
        style->Item.Text.Alignment = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->Item.Border.Size    = 3_px;

        auto hoveredStyle {retValue.create<item_style>("tab_items", {.Hover = true})};
        hoveredStyle->Item            = style->Item;
        hoveredStyle->Item.Background = green.Button00;

        auto activeStyle {retValue.create<item_style>("tab_items", {.Active = true})};
        activeStyle->Item            = style->Item;
        activeStyle->Item.Background = red.Button00;
    }
    // SECTION ITEMS
    {
        auto style {retValue.create<item_style>("section_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Background     = blue.Button00;
        style->Item.Text.Style     = {false, font::weight::Normal};
        style->Item.Text.Font      = resGrp->get<font_family>("Poppins");
        style->Item.Text.Size      = 50_pct;
        style->Item.Text.Color     = colors::Black;
        style->Item.Text.Alignment = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->Item.Border.Size    = 3_px;

        auto hoveredStyle {retValue.create<item_style>("section_items", {.Hover = true})};
        hoveredStyle->Item            = style->Item;
        hoveredStyle->Item.Background = green.Button00;

        auto activeStyle {retValue.create<item_style>("section_items", {.Active = true})};
        activeStyle->Item            = style->Item;
        activeStyle->Item.Background = red.Button00;
    }
    // LIST ITEMS
    {
        auto style {retValue.create<item_style>("list_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Background     = yellow.Button13;
        style->Item.Text.Style     = {false, font::weight::Normal};
        style->Item.Text.Font      = resGrp->get<font_family>("Poppins");
        style->Item.Text.Size      = 50_pct;
        style->Item.Text.Color     = colors::Black;
        style->Item.Text.Alignment = {horizontal_alignment::Left, vertical_alignment::Middle};
        style->Item.Border.Size    = 5_px;

        auto hoveredStyle {retValue.create<item_style>("list_items", {.Hover = true})};
        hoveredStyle->Item            = style->Item;
        hoveredStyle->Item.Background = blue.Button13;

        auto activeStyle {retValue.create<item_style>("list_items", {.Active = true})};
        activeStyle->Item            = style->Item;
        activeStyle->Item.Background = red.Button13;
    }
    // GRID HEADER ITEMS
    {
        auto style {retValue.create<item_style>("header_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Background     = grey.Button13;
        style->Item.Text.Style     = {false, font::weight::Bold};
        style->Item.Text.Font      = resGrp->get<font_family>("Poppins");
        style->Item.Text.Size      = 25_pct;
        style->Item.Text.Color     = colors::Black;
        style->Item.Text.Alignment = {horizontal_alignment::Centered, vertical_alignment::Middle};
        style->Item.Border.Size    = 5_px;

        auto hoveredStyle {retValue.create<item_style>("header_items", {.Hover = true})};
        hoveredStyle->Item            = style->Item;
        hoveredStyle->Item.Background = blue.Button13;

        auto activeStyle {retValue.create<item_style>("header_items", {.Active = true})};
        activeStyle->Item            = style->Item;
        activeStyle->Item.Background = red.Button13;
    }
    // GRID ROW ITEMS
    {
        auto style {retValue.create<item_style>("row_items", {}, {})};
        style->Item.Padding        = {7_px};
        style->Item.Background     = yellow.Button13;
        style->Item.Text.Style     = {false, font::weight::Normal};
        style->Item.Text.Font      = resGrp->get<font_family>("Poppins");
        style->Item.Text.Size      = 25_pct;
        style->Item.Text.Color     = colors::Black;
        style->Item.Text.Alignment = {horizontal_alignment::Left, vertical_alignment::Middle};
        style->Item.Border.Size    = 5_px;

        auto hoveredStyle {retValue.create<item_style>("row_items", {.Hover = true})};
        hoveredStyle->Item            = style->Item;
        hoveredStyle->Item.Background = blue.Button13;

        auto activeStyle {retValue.create<item_style>("row_items", {.Active = true})};
        activeStyle->Item            = style->Item;
        activeStyle->Item.Background = red.Button13;
    }
    return retValue;
}

////////////////////////////////////////////////////////////
