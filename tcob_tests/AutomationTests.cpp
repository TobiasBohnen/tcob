#include "tests.hpp"
#include <queue>

using Catch::Matchers::Approx;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Core.Automation.Vector")
{
    std::vector<std::unique_ptr<AutomationBase>> controllers;

    f32 output1;
    f32 output2;

    {
        auto contr { make_unique_automation<LinearFunction<f32>>(1000., 50.f, 10.f) };
        contr->add_output(&output1);
        controllers.push_back(std::move(contr));
    }
    {
        auto contr { make_unique_automation<LinearFunction<f32>>(1000., 50.f, 150.f) };
        contr->add_output(&output2);
        controllers.push_back(std::move(contr));
    }

    for (auto& contr : controllers) {
        contr->start();
        contr->update(250);
    }

    REQUIRE(output1 == 40.f);
    REQUIRE(output2 == 75.f);
}

TEST_CASE("Core.Automation.Queue")
{
    AutomationQueue queue;

    std::vector<f32> output;
    auto output1 { [&output](f32 val) { output.push_back(val); } };
    {
        auto contr { make_shared_automation<LinearFunction<f32>>(1000, 50.f, 10.f) };
        contr->add_output(output1);
        queue.push(contr);
    }
    {
        auto contr { make_shared_automation<LinearFunction<f32>>(1000, 50.f, 150.f) };
        contr->add_output(output1);
        queue.push(contr);
    }

    queue.start();
    while (!queue.is_empty()) {
        queue.update(250);
    }
    queue.stop_and_clear();
    REQUIRE(output == std::vector<f32> { 50.0f, 40.0f, 30.0f, 20.0f, 10.0f, 50.0f, 75.0f, 100.0f, 125.0f, 150.f });
}
TEST_CASE("Core.Automation.RandomFunction")
{
    f32 out;
    Random rand { 12345 };
    Automation<RandomFunction<f32>> contr { { 100, 10.f, 50.f, rand } };

    contr.add_output([&out](f32 newVal) { out = newVal; });
    contr.start(true);
    REQUIRE(out == rand(10.f, 50.f));
    contr.update(1);
    REQUIRE(out == rand(10.f, 50.f));
    contr.update(1);
    REQUIRE(out == rand(10.f, 50.f));
    contr.update(1);
    REQUIRE(out == rand(10.f, 50.f));
    contr.update(1);
    REQUIRE(out == rand(10.f, 50.f));
}

TEST_CASE("Core.Automation.LinearFunction")
{
    {
        f32 out1;
        f32 out2;
        f32 out3;

        Automation<LinearFunction<f32>> contr { { 1000, 10.f, 70.f } };

        contr.add_output(&out1);
        contr.add_output(&out2);
        contr.add_output(&out3);
        contr.start();
        contr.update(500);
        REQUIRE(out1 == 40.f);
        REQUIRE(out1 == out2);
        REQUIRE(out2 == out3);
    }
    {
        f32 out;

        Automation<LinearFunction<f32>> contr { { 100, 10.f, 50.f } };

        contr.add_output([&out](f32 newVal) { out = newVal; });
        contr.start(true);

        contr.update(90);
        REQUIRE(out == 46.f);
        contr.update(20);
        REQUIRE(out == 14.f);
        contr.update(20);
        REQUIRE(out == 22.f);
        contr.update(20);
        REQUIRE(out == 30.f);
    }
    {
        u32 out;

        Automation<LinearFunction<u32>> contr { { 100, 10, 50 } };

        contr.add_output([&out](u32 newVal) { out = newVal; });
        contr.start(true);

        contr.update(90);
        REQUIRE(out == 46);
        contr.update(20);
        REQUIRE(out == 14);
        contr.update(20);
        REQUIRE(out == 22);
        contr.update(20);
        REQUIRE(out == 30);
    }
    {
        u32 out;

        Automation<LinearFunction<u32>> contr { { 100, 0, 50 } };

        contr.add_output([&out](u32 newVal) { out = newVal; });
        contr.start(false);

        contr.update(10);
        REQUIRE(out == 5);
        contr.update(10);
        REQUIRE(out == 10);
        contr.update(10);
        REQUIRE(out == 15);
        contr.update(10);
        REQUIRE(out == 20);
    }
    {
        auto out = std::make_shared<f32>();

        Automation contr {
            LinearFunction<f32> { 1000, 10.f, 50.f }
        };

        contr.add_output(out.get());
        contr.start();

        contr.update(250);
        REQUIRE(*out == 20.f);
        contr.update(250);
        REQUIRE(*out == 30.f);
        contr.update(250);
        REQUIRE(*out == 40.f);
        contr.update(250);
        REQUIRE(*out == 50.f);
    }
    {

        Automation<LinearFunction<f32>> contr { { 1000, 10.f, 50.f } };

        struct Foo {
            void set_bar(f32 newBar)
            {
                bar = newBar;
            }
            f32 bar;
        };
        Foo foo;
        auto f3 { std::bind(&Foo::set_bar, &foo, std::placeholders::_1) };

        contr.add_output(f3);
        contr.start();

        contr.update(250);
        REQUIRE(foo.bar == 20.f);
        contr.update(250);
        REQUIRE(foo.bar == 30.f);
        contr.update(250);
        REQUIRE(foo.bar == 40.f);
        contr.update(250);
        REQUIRE(foo.bar == 50.f);
    }
    {
        f32 out1;
        f32 out2;
        f32 out3;

        f32 fval1;
        f32 fval2;
        f32 fval3;

        Automation contr {
            LinearFunction<f32> { 1000, 10.f, 50.f }
        };

        contr.add_output(&out1);
        contr.add_output(&out2);
        contr.add_output(&out3);

        contr.add_output([&fval1](f32 newVal) { fval1 = newVal; });
        contr.add_output([&fval2](f32 newVal) { fval2 = newVal; });
        contr.add_output([&fval3](f32 newVal) { fval3 = newVal; });

        contr.start();
        contr.update(500);
        REQUIRE(out1 == 30.f);
        REQUIRE(out1 == out2);
        REQUIRE(out2 == out3);
        REQUIRE(out3 == fval1);
        REQUIRE(fval1 == fval2);
        REQUIRE(fval2 == fval3);
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            LinearFunction<Color> { 1000, c1, c2 }
        };

        contr.add_output(&out);
        contr.start();
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xBF);
        REQUIRE(out.A == 0x3F);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
    }
}

TEST_CASE("Core.Automation.PowerFunctionFunction")
{
    {
        f32 out;

        Automation contr {
            PowerFunction<f32> { 1000, 10.f, 50.f, 2.f }
        };

        contr.add_output(&out);
        contr.start();
        contr.update(250);
        REQUIRE(out == 12.5f);
        contr.update(250);
        REQUIRE(out == 20.f);
        contr.update(250);
        REQUIRE(out == 32.5f);
        contr.update(250);
        REQUIRE(out == 50.f);
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            PowerFunction<Color> { 1000, c1, c2, 2.f }
        };

        contr.add_output(&out);
        contr.start();
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 15);
        REQUIRE(out.A == 239);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x8F);
        REQUIRE(out.A == 0x6F);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
    }
}

TEST_CASE("Core.Automation.SineWaveFunction")
{
    {
        auto out1 = 15.;

        Automation contr {
            SineWaveFunction<f64> { 1000, 10., 50., 1.f, 0.f }
        };

        contr.add_output(&out1);

        contr.start();
        contr.update(500);
        REQUIRE_THAT(out1, WithinRel(50.));
        contr.update(250);
        REQUIRE_THAT(out1, WithinRel(30.));
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            SineWaveFunction<Color> { 1000, c1, c2, 1.f, 0.f }
        };

        contr.add_output(&out);

        contr.start();
        contr.update(500);
        REQUIRE(out == c2);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
    }
    {
        Automation contr {
            SineWaveFunction<f32> { 360, 0.f, 1.f, 1.f, 0 }
        };
        std::vector<f32> output;
        contr.add_output([&output](f32 val) { output.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 4; i++) {
            contr.update(90);
        }
        REQUIRE(output == std::vector<f32> { 0.0f, 0.5f, 1.0f, 0.5f, 0.0f });
    }
}

TEST_CASE("Core.Automation.SquareWaveFunction")
{
    {
        auto out1 = 15.;

        Automation contr {
            SquareWaveFunction<f64> { 1000, 10., 50., 1.f, 0.f }
        };

        contr.add_output(&out1);

        contr.start();
        contr.update(499);
        REQUIRE(out1 == 10.);
        contr.update(2);
        REQUIRE(out1 == 50.);
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            SquareWaveFunction<Color> { 1000, c1, c2, 1.f, 0 }
        };

        contr.add_output(&out);

        contr.start();
        contr.update(499);
        REQUIRE(out == c1);
        contr.update(2);
        REQUIRE(out == c2);
    }
    {
        Automation contr {
            SquareWaveFunction<f32> { 50, 0.f, 1.f, 1.f, 0 }
        };
        std::vector<f32> output;
        contr.add_output([&output](f32 val) { output.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 5; i++) {
            contr.update(10);
        }
        REQUIRE(output == std::vector<f32> { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });
    }
}

TEST_CASE("Core.Automation.TriangeWaveFunction")
{
    {
        auto out1 = 15.;

        Automation contr {
            TriangeWaveFunction<f64> { 1000, 10., 50., 2.f, 1.f }
        };

        contr.add_output(&out1);

        contr.start();
        contr.update(250);
        REQUIRE(out1 == 50.);
        contr.update(250);
        REQUIRE(out1 == 10.);
        contr.update(125);
        REQUIRE(out1 == 30.);
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            TriangeWaveFunction<Color> { 1000, c1, c2, 1.f, 1.f }
        };

        contr.add_output(&out);

        contr.start();
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
        contr.update(125);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xBF);
        REQUIRE(out.A == 0x3F);
    }
    {
        Automation contr {
            TriangeWaveFunction<f32> { 50, 0.f, 2.f, 1.f, 0 }
        };
        std::vector<f32> output;
        contr.add_output([&output](f32 val) { output.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 5; i++) {
            contr.update(10);
        }
        REQUIRE_THAT(output, Approx(std::vector<f32> { 0.0f, 0.8f, 1.6f, 1.6f, 0.8f, 0.0f }));
    }
}

TEST_CASE("Core.Automation.SawtoothWaveFunction")
{
    {
        auto out1 = 15.;

        Automation contr {
            SawtoothWaveFunction<f64> { 1000, 10., 50., 5.f, 0.f }
        };

        contr.add_output(&out1);

        contr.start();
        contr.update(250);
        REQUIRE(out1 == 20.);
        contr.update(250);
        REQUIRE(out1 == 30.);
        contr.update(125);
        REQUIRE(out1 == 15.);
    }
    {
        Color c1 { 0xFF, 0, 0, 0xFF };
        Color c2 { 0xFF, 0, 0xFF, 0 };
        Color out;

        Automation contr {
            SawtoothWaveFunction<Color> { 1000, c1, c2, 5.f, 0.f }
        };

        contr.add_output(&out);

        contr.start();
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(125);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x1F);
        REQUIRE(out.A == 0xDF);
    }
    {
        Automation contr {
            SawtoothWaveFunction<f32> { 50, 0.f, 2.f, 1.f, 0 }
        };
        std::vector<f32> output;
        contr.add_output([&output](f32 val) { output.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 5; i++) {
            contr.update(10);
        }
        REQUIRE_THAT(output, Approx(std::vector<f32> { 0.0f, 0.4f, 0.8f, 1.2f, 1.6f, 0.0f }));
    }
}

TEST_CASE("Core.Automation.LinearChainFunction")
{
    {
        f32 out;

        auto wi { LinearFunctionChain<f32>(1000, { 0, 10, 5, 25, 10 }) };

        Automation contr { std::move(wi) };
        contr.add_output(&out);

        contr.start();
        REQUIRE_THAT(out, WithinAbs(00.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(05.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(10.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(07.5f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(05.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(15.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(25.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(17.5f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out, WithinAbs(10.0f, 0.0001f));
    }
    {
        PointF out1;

        auto wi { LinearFunctionChain<PointF>(1000, { { 0.f, 0.f }, { 10.f, 20.f }, { 20.f, 10.f }, { 40.f, 0.f }, { 40.f, 80.f } }) };

        Automation<LinearFunctionChain<PointF>> contr { std::move(wi) };
        contr.add_output(&out1);

        contr.start();
        REQUIRE_THAT(out1.X, WithinAbs(00.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(00.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(05.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(10.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(10.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(20.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(15.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(15.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(20.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(10.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(30.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(05.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(00.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(40.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(80.0f, 0.0001f));
    }
    {
        PointF out1;

        auto wi { LinearFunctionChain<PointF>(1000,
            std::vector<PointF> { { 0.f, 0.f }, { 10.f, 20.f }, { 20.f, 10.f }, { 40.f, 0.f }, { 40.f, 80.f } }) };

        Automation contr { std::move(wi) };
        contr.add_output(&out1);

        contr.start();
        REQUIRE_THAT(out1.X, WithinAbs(00.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(00.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(05.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(10.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(10.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(20.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(15.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(15.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(20.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(10.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(30.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(05.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(00.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(40.0f, 0.0001f));
        contr.update(125);
        REQUIRE_THAT(out1.X, WithinAbs(40.0f, 0.0001f));
        REQUIRE_THAT(out1.Y, WithinAbs(80.0f, 0.0001f));
    }
}

TEST_CASE("Core.Automation.Animation")
{
    {
        std::string out;

        FrameAnimation ani;
        ani.Frames = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
        ani.Duration = 160;
        ani.Mode = AnimationPlaybackMode::Normal;
        Automation<FrameAnimationFunction> contr { FrameAnimationFunction(160, ani) };
        contr.add_output(&out);

        contr.start();
        REQUIRE(out == "0");
        contr.update(10);
        REQUIRE(out == "1");
        contr.update(10);
        REQUIRE(out == "2");
        contr.update(10);
        REQUIRE(out == "3");
        contr.update(10);
        REQUIRE(out == "4");
        contr.update(10);
        REQUIRE(out == "5");
        contr.update(10);
        REQUIRE(out == "6");
        contr.update(10);
        REQUIRE(out == "7");
        contr.update(10);
        REQUIRE(out == "8");
    }
}

TEST_CASE("Core.Automation.Interval")
{
    {
        std::vector<f32> output;

        Automation<LinearFunction<f32>> contr { { 1000, 10.f, 70.f } };

        contr.add_output([&output](f32 val) { output.push_back(val); });
        contr.interval(100);

        contr.start();
        for (i32 i { 0 }; i <= 1000; i++) {
            contr.update(1);
        }
        REQUIRE(output.size() == 11);
        REQUIRE(output[0] == 10.f);
        REQUIRE(output[1] == 16.f);
        REQUIRE(output[2] == 22.f);
        REQUIRE(output[3] == 28.f);
        REQUIRE(output[4] == 34.f);
        REQUIRE(output[5] == 40.f);
        REQUIRE(output[6] == 46.f);
        REQUIRE(output[7] == 52.f);
        REQUIRE(output[8] == 58.f);
        REQUIRE(output[9] == 64.f);
        REQUIRE(output[10] == 70.f);
    }

    {
        std::vector<f32> output;

        Automation<LinearFunction<f32>> contr { { 1000, 10.f, 70.f } };

        contr.add_output([&output](f32 val) { output.push_back(val); });
        contr.interval(500);

        contr.start();
        for (i32 i { 0 }; i <= 1000; i++) {
            contr.update(1);
        }
        REQUIRE(output.size() == 3);
        REQUIRE(output[0] == 10.f);
        REQUIRE(output[1] == 40.f);
        REQUIRE(output[2] == 70.f);
    }
}