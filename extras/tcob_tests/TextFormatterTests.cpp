#include "tests.hpp"

class null_font : public font {
    auto shape_text(std::string_view, bool, bool) -> std::vector<glyph> override
    {
        return {};
    }

    void setup_texture() override
    {
    }

    auto get_info() const -> font::info const& override
    {
        return _info;
    }

    font::info _info;
};

TEST_CASE("GFX.Text.Tokenizer")
{
    null_font font;
    {
        auto tokens {text_formatter::shape("   ", font, true, true)};
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Whitespace);
    }
    {
        auto tokens {text_formatter::shape("   a", font, true, true)};
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape(" a ", font, true, true)};
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Whitespace);
    }
    {
        auto tokens {text_formatter::shape("a b c", font, true, true)};
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[3].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[4].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape("abc", font, true, true)};
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape("öäößह", font, true, true)};
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape("abc def", font, true, true)};
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[0].Text == "abc");
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[2].Text == "def");
    }
    {
        auto tokens {text_formatter::shape("a\n b   \nc\n ", font, true, true)};
        REQUIRE(tokens.size() == 9);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Newline);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[3].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[4].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[5].Type == text_formatter::token_type::Newline);
        REQUIRE(tokens[6].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[7].Type == text_formatter::token_type::Newline);
        REQUIRE(tokens[8].Type == text_formatter::token_type::Whitespace);
    }
    {
        auto tokens {text_formatter::shape("abc def {color=red}", font, true, true)};
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[3].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[4].Type == text_formatter::token_type::Command);
    }
    {
        auto tokens {text_formatter::shape("abc {color=red}def ", font, true, true)};
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Command);
        REQUIRE(tokens[3].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[4].Type == text_formatter::token_type::Whitespace);
    }
    {
        auto tokens {text_formatter::shape("a{  {", font, true, true)};
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Text == "{");
    }
    {
        auto tokens {text_formatter::shape("a{{bb", font, true, true)};
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[0].Text == "a");
        REQUIRE(tokens[1].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Text == "{bb");
    }
    {
        auto tokens {text_formatter::shape("a} {", font, true, true)};
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape("a }", font, true, true)};
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Text);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Whitespace);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Text);
    }
    {
        auto tokens {text_formatter::shape("{effect:1}{effect:25}{effect:42}", font, true, true)};
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].Type == text_formatter::token_type::Command);
        REQUIRE(std::get<u8>(tokens[0].Command.Value) == 1);
        REQUIRE(tokens[1].Type == text_formatter::token_type::Command);
        REQUIRE(std::get<u8>(tokens[1].Command.Value) == 25);
        REQUIRE(tokens[2].Type == text_formatter::token_type::Command);
        REQUIRE(std::get<u8>(tokens[2].Command.Value) == 42);
    }
}
