#include "tests.hpp"

#include <queue>

using namespace tcob::tweening;
using namespace tcob::literals;

TEST_CASE("Core.Tween.Vector")
{
    std::vector<std::unique_ptr<tween_base>> controllers;

    f32 out1 {};
    f32 out2 {};

    {
        auto contr {make_unique_tween<linear_tween<f32>>(milliseconds {1000.}, 50.f, 10.f)};
        contr->add_output(&out1);
        controllers.push_back(std::move(contr));
    }
    {
        auto contr {make_unique_tween<linear_tween<f32>>(milliseconds {1000.}, 50.f, 150.f)};
        contr->add_output(&out2);
        controllers.push_back(std::move(contr));
    }

    for (auto& contr : controllers) {
        contr->start();
        contr->update(250ms);
    }

    REQUIRE(out1 == 40.f);
    REQUIRE(out2 == 75.f);
}

TEST_CASE("Core.Tween.Queue")
{
    tweening::queue queue;

    std::vector<f32> out;
    auto             output1 {[&out](f32 val) { out.push_back(val); }};
    {
        auto contr {make_shared_tween<linear_tween<f32>>(milliseconds {1000.}, 50.f, 10.f)};
        contr->Value.Changed.connect(output1);
        queue.push(contr);
    }
    {
        auto contr {make_shared_tween<linear_tween<f32>>(milliseconds {1000.}, 50.f, 150.f)};
        contr->Value.Changed.connect(output1);
        queue.push(contr);
    }

    queue.start();
    while (!queue.is_empty()) {
        queue.update(250ms);
    }
    queue.stop();
    REQUIRE(out == std::vector<f32> {50.0f, 40.0f, 30.0f, 20.0f, 10.0f, 50.0f, 75.0f, 100.0f, 125.0f, 150.f});
}

TEST_CASE("Core.Tween.RandomFunction")
{
    f32               out {};
    rng               rnd {12345};
    random_tween<f32> contr {milliseconds {100}, {10.f, 50.f, rnd}};

    contr.Value.Changed.connect([&out](f32 newVal) { out = newVal; });
    contr.start(playback_style::Looped);
    REQUIRE(out == rnd(10.f, 50.f));
    contr.update(1ms);
    REQUIRE(out == rnd(10.f, 50.f));
    contr.update(1ms);
    REQUIRE(out == rnd(10.f, 50.f));
    contr.update(1ms);
    REQUIRE(out == rnd(10.f, 50.f));
    contr.update(1ms);
    REQUIRE(out == rnd(10.f, 50.f));
}

TEST_CASE("Core.Tween.LinearFunction")
{
    {
        f32 out1 {};
        f32 out2 {};
        f32 out3 {};

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 70.f}};

        contr.add_output(&out1);
        contr.add_output(&out2);
        contr.add_output(&out3);
        contr.start();
        contr.update(500ms);
        REQUIRE(out1 == 40.f);
        REQUIRE(out1 == out2);
        REQUIRE(out2 == out3);
    }
    {
        f32 out {};

        linear_tween<f32> contr {milliseconds {100}, {10.f, 50.f}};

        contr.Value.Changed.connect([&out](f32 newVal) { out = newVal; });
        contr.start(playback_style::Looped);

        contr.update(90ms);
        REQUIRE(out == 46.f);
        contr.update(20ms);
        REQUIRE(out == 14.f);
        contr.update(20ms);
        REQUIRE(out == 22.f);
        contr.update(20ms);
        REQUIRE(out == 30.f);
    }
    {
        u32 out {};

        linear_tween<u32> contr {milliseconds {100}, {10, 50}};

        contr.Value.Changed.connect([&out](u32 newVal) { out = newVal; });
        contr.start(playback_style::Looped);

        contr.update(90ms);
        REQUIRE(out == static_cast<u32>(10 + static_cast<f64>((50 - 10) * (90. / 100.))));
        contr.update(20ms);
        REQUIRE(out == static_cast<u32>(10 + static_cast<f64>((50 - 10) * (10. / 100.))));
        contr.update(20ms);
        REQUIRE(out == static_cast<u32>(10 + static_cast<f64>((50 - 10) * (30. / 100.))));
        contr.update(20ms);
        REQUIRE(out == static_cast<u32>(10 + static_cast<f64>((50 - 10) * (50. / 100.))));
    }
    {
        u32 out {};

        linear_tween<u32> contr {milliseconds {100}, {0, 50}};

        contr.Value.Changed.connect([&out](u32 newVal) { out = newVal; });
        contr.start(playback_style::Looped);

        contr.update(10ms);
        REQUIRE(out == 5);
        contr.update(10ms);
        REQUIRE(out == 10);
        contr.update(10ms);
        REQUIRE(out == 15);
        contr.update(10ms);
        REQUIRE(out == 20);
    }
    {
        auto out = std::make_shared<f32>();

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 50.f}};

        contr.add_output(out.get());
        contr.start();

        contr.update(250ms);
        REQUIRE(*out == 20.f);
        contr.update(250ms);
        REQUIRE(*out == 30.f);
        contr.update(250ms);
        REQUIRE(*out == 40.f);
        contr.update(250ms);
        REQUIRE(*out == 50.f);
    }
    {

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 50.f}};

        struct Foo {
            void set_bar(f32 newBar)
            {
                bar = newBar;
            }
            f32 bar;
        };
        Foo  foo {};
        auto f3 {[&foo](f32 value) { foo.set_bar(value); }};
        // auto f3{std::bind(&Foo::set_bar, &foo, std::placeholders::_1)};

        contr.Value.Changed.connect(f3);
        contr.start();

        contr.update(250ms);
        REQUIRE(foo.bar == 20.f);
        contr.update(250ms);
        REQUIRE(foo.bar == 30.f);
        contr.update(250ms);
        REQUIRE(foo.bar == 40.f);
        contr.update(250ms);
        REQUIRE(foo.bar == 50.f);
    }
    {
        f32 out1 {};
        f32 out2 {};
        f32 out3 {};

        f32 fval1 {};
        f32 fval2 {};
        f32 fval3 {};

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 50.f}};

        contr.add_output(&out1);
        contr.add_output(&out2);
        contr.add_output(&out3);

        contr.Value.Changed.connect([&fval1](f32 newVal) { fval1 = newVal; });
        contr.Value.Changed.connect([&fval2](f32 newVal) { fval2 = newVal; });
        contr.Value.Changed.connect([&fval3](f32 newVal) { fval3 = newVal; });

        contr.start();
        contr.update(500ms);
        REQUIRE(out1 == 30.f);
        REQUIRE(out1 == out2);
        REQUIRE(out2 == out3);
        REQUIRE(out3 == fval1);
        REQUIRE(fval1 == fval2);
        REQUIRE(fval2 == fval3);
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        linear_tween<color> contr {milliseconds {1000}, {c1, c2}};

        contr.add_output(&out);
        contr.start();
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xBF);
        REQUIRE(out.A == 0x3F);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
    }
}

TEST_CASE("Core.Tween.PowerFunction")
{
    {
        f32 out {};

        power_tween<f32> contr {milliseconds {1000}, {10.f, 50.f, 2.f}};

        contr.add_output(&out);
        contr.start();
        contr.update(250ms);
        REQUIRE(out == 12.5f);
        contr.update(250ms);
        REQUIRE(out == 20.f);
        contr.update(250ms);
        REQUIRE(out == 32.5f);
        contr.update(250ms);
        REQUIRE(out == 50.f);
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        power_tween<color> contr {milliseconds {1000}, {c1, c2, 2.f}};

        contr.add_output(&out);
        contr.start();
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 15);
        REQUIRE(out.A == 239);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x8F);
        REQUIRE(out.A == 0x6F);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
    }
}

TEST_CASE("Core.Tween.QuadBezierCurve")
{
    auto interpolate {[](point_f Start, point_f ControlPoint, point_f End, float t) {
        point_f const a {point_f::Lerp(Start, ControlPoint, t)};
        point_f const b {point_f::Lerp(ControlPoint, End, t)};
        return point_f::Lerp(a, b, t);
    }};

    {
        point_f out {};

        point_f start {0.0f, 5.0f};
        point_f cp {2.5f, 0.0f};
        point_f end {10.f, 5.0f};

        quad_bezier_curve_tween contr {milliseconds {1000}, {start, cp, end}};

        contr.add_output(&out);
        contr.start();
        contr.update(0ms);
        REQUIRE(out.equals(interpolate(start, cp, end, 0.0f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp, end, 0.25f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp, end, 0.5f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp, end, 0.75f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp, end, 1.0f), 0.001f));
    }
    {
        point_f out {};

        point_f start {0.0f, 2.0f};
        point_f cp {2.0f, 0.0f};
        point_f end {4.0f, 2.0f};

        quad_bezier_curve_tween contr {milliseconds {1000}, {start, cp, end}};

        contr.add_output(&out);
        contr.start();
        contr.update(500ms);
        REQUIRE(out.equals({2.0f, 1.0f}, 0.001f));
    }
}

TEST_CASE("Core.Tween.CubicBezierCurve")
{
    auto interpolate {[](point_f Start, point_f ControlPoint0, point_f ControlPoint1, point_f End, float t) {
        point_f const a {point_f::Lerp(Start, ControlPoint0, t)};
        point_f const b {point_f::Lerp(ControlPoint0, ControlPoint1, t)};
        point_f const c {point_f::Lerp(ControlPoint1, End, t)};
        return point_f::Lerp(point_f::Lerp(a, b, t), point_f::Lerp(b, c, t), t);
    }};

    {
        point_f out {};

        point_f start {0.0f, 5.0f};
        point_f cp0 {2.5f, 0.0f};
        point_f cp1 {7.5f, 10.0f};
        point_f end {10.f, 5.0f};

        cubic_bezier_curve_tween contr {milliseconds {1000}, {start, cp0, cp1, end}};

        contr.add_output(&out);
        contr.start();
        contr.update(0ms);
        REQUIRE(out.equals(interpolate(start, cp0, cp1, end, 0.0f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp0, cp1, end, 0.25f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp0, cp1, end, 0.5f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp0, cp1, end, 0.75f), 0.001f));
        contr.update(250ms);
        REQUIRE(out.equals(interpolate(start, cp0, cp1, end, 1.0f), 0.001f));
    }
}

TEST_CASE("Core.Tween.BezierCurve")
{
    {
        point_f out0 {};
        point_f out1 {};

        point_f start {0.0f, 5.0f};
        point_f cp0 {2.5f, 0.0f};
        point_f cp1 {7.5f, 10.0f};
        point_f end {10.f, 5.0f};

        cubic_bezier_curve_tween contr0 {milliseconds {1000}, {start, cp0, cp1, end}};
        bezier_curve_tween       contr1 {milliseconds {1000}, {{start, cp0, cp1, end}}};

        contr0.add_output(&out0);
        contr1.add_output(&out1);
        contr0.start();
        contr1.start();
        contr0.update(0ms);
        contr1.update(0ms);
        REQUIRE(out0 == out1);
        contr0.update(250ms);
        contr1.update(250ms);
        REQUIRE(out0 == out1);
        contr0.update(250ms);
        contr1.update(250ms);
        REQUIRE(out0 == out1);
        contr0.update(250ms);
        contr1.update(250ms);
        REQUIRE(out0 == out1);
        contr0.update(250ms);
        contr1.update(250ms);
        REQUIRE(out0 == out1);
    }
}

TEST_CASE("Core.Tween.SineWaveFunction")
{
    {
        f64 out = 15.;

        sine_wave_tween<f64> contr {milliseconds {1000}, {10., 50., 1.f, 0.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(500ms);
        REQUIRE(out == Approx(50.));
        contr.update(250ms);
        REQUIRE(out == Approx(30.));
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        sine_wave_tween<color> contr {milliseconds {1000}, {c1, c2, 1.f, 0.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(500ms);
        REQUIRE(out == c2);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
    }
    {
        sine_wave_tween<f32> contr {milliseconds {360}, {0.f, 1.f, 1.f, 0}};
        std::vector<f32>     out;
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 4; i++) {
            contr.update(90ms);
        }
        REQUIRE(out == std::vector<f32> {0.5f, 1.0f, 0.5f, 0.0f});
    }
}

TEST_CASE("Core.Tween.CircularFunction")
{
    {
        point_f out;

        circular_tween contr {milliseconds {1000}, {0_deg, 360_deg}};

        contr.add_output(&out);

        contr.start();
        contr.update(0ms);
        REQUIRE_MESSAGE(out.equals(point_f {1.0f, 0.0f}, 0.01f), out);
        contr.update(250ms);
        REQUIRE_MESSAGE(out.equals(point_f {0.0f, 1.0f}, 0.01f), out);
        contr.update(250ms);
        REQUIRE_MESSAGE(out.equals(point_f {-1.0f, 0.0f}, 0.01f), out);
        contr.update(250ms);
        REQUIRE_MESSAGE(out.equals(point_f {0.0f, -1.0f}, 0.01f), out);
        contr.update(250ms);
        REQUIRE_MESSAGE(out.equals(point_f {1.0f, 0.0f}, 0.01f), out);
    }
}

TEST_CASE("Core.Tween.SquareWaveFunction")
{
    {
        auto out = 15.;

        square_wave_tween<f64> contr {milliseconds {1000}, {10., 50., 1.f, 0.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(499ms);
        REQUIRE(out == 10.);
        contr.update(2ms);
        REQUIRE(out == 50.);
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        square_wave_tween<color> contr {milliseconds {1000}, {c1, c2, 1.f, 0}};

        contr.add_output(&out);

        contr.start();
        contr.update(499ms);
        REQUIRE(out == c1);
        contr.update(2ms);
        REQUIRE(out == c2);
    }
    {
        square_wave_tween<f32> contr {milliseconds {50}, {0.f, 1.f, 1.f, 0}};
        std::vector<f32>       out;
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });
        contr.start();
        for (i32 i = 0; i < 5; i++) {
            contr.update(10ms);
        }
        REQUIRE(out == std::vector<f32> {1.0f});
    }
    {
        square_wave_tween<bool>           contr {milliseconds {50}, {1.f, 0}};
        std::vector<std::pair<i32, bool>> out;
        contr.start();
        for (i32 i = 0; i < 5; i++) {
            out.emplace_back(i * 10, contr.Value);
            contr.update(10ms);
        }
        REQUIRE(out == std::vector<std::pair<i32, bool>> {{0, true}, {10, true}, {20, true}, {30, false}, {40, false}});
    }
}

TEST_CASE("Core.Tween.TriangeWaveFunction")
{
    {
        auto out = 15.;

        triange_wave_tween<f64> contr {milliseconds {1000}, {10., 50., 2.f, 1.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(250ms);
        REQUIRE(out == 50.);
        contr.update(250ms);
        REQUIRE(out == 10.);
        contr.update(125ms);
        REQUIRE(out == 30.);
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        triange_wave_tween<color> contr {milliseconds {1000}, {c1, c2, 1.f, 1.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xFF);
        REQUIRE(out.A == 0);
        contr.update(125ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0xBF);
        REQUIRE(out.A == 0x3F);
    }
    {
        triange_wave_tween<f32> contr {milliseconds {50}, {0.f, 2.f, 1.f, 0}};
        std::vector<f32>        out;
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 10; i++) {
            contr.update(5ms);
        }

        REQUIRE(out[0] == Approx(0.4f));
        REQUIRE(out[1] == Approx(0.8f));
        REQUIRE(out[2] == Approx(1.2f));
        REQUIRE(out[3] == Approx(1.6f));
        REQUIRE(out[4] == Approx(2.0f));
        REQUIRE(out[5] == Approx(1.6f));
        REQUIRE(out[6] == Approx(1.2f));
        REQUIRE(out[7] == Approx(0.8f));
        REQUIRE(out[8] == Approx(0.4f));
        REQUIRE(out[9] == Approx(0.0f));
    }
}

TEST_CASE("Core.Tween.SawtoothWaveFunction")
{
    {
        auto out = 15.;

        sawtooth_wave_tween<f64> contr {milliseconds {1000}, {10., 50., 5.f, 0.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(250ms);
        REQUIRE(out == 20.);
        contr.update(250ms);
        REQUIRE(out == 30.);
        contr.update(125ms);
        REQUIRE(out == 15.);
    }
    {
        color c1 {0xFF, 0, 0, 0xFF};
        color c2 {0xFF, 0, 0xFF, 0};
        color out;

        sawtooth_wave_tween<color> contr {milliseconds {1000}, {c1, c2, 5.f, 0.f}};

        contr.add_output(&out);

        contr.start();
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x3F);
        REQUIRE(out.A == 0xBF);
        contr.update(250ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x7F);
        REQUIRE(out.A == 0x7F);
        contr.update(125ms);
        REQUIRE(out.R == 0xFF);
        REQUIRE(out.G == 0);
        REQUIRE(out.B == 0x1F);
        REQUIRE(out.A == 0xDF);
    }
    {
        sawtooth_wave_tween<f32> contr {milliseconds {50}, {0.f, 2.f, 1.f, 0}};
        std::vector<f32>         out;
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start();
        for (i32 i = 0; i < 5; i++) {
            contr.update(10ms);
        }

        REQUIRE(out[0] == Approx(0.4f));
        REQUIRE(out[1] == Approx(0.8f));
        REQUIRE(out[2] == Approx(1.2f));
        REQUIRE(out[3] == Approx(1.6f));
        REQUIRE(out[4] == Approx(0.0f));
    }
}

TEST_CASE("Core.Tween.LinearChainFunction")
{
    {
        f32                     out {0};
        std::vector<f32>        points {{0, 10, 5, 25, 10}};
        linear_chain_tween<f32> contr {milliseconds {1000}, {points}};
        contr.add_output(&out);

        contr.start();

        REQUIRE(out == Approx(00.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(05.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(10.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(07.5f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(05.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(15.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(25.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(17.5f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out == Approx(10.0f).epsilon(0.0001f));
    }
    {
        point_f                     out1;
        std::vector<point_f>        points {{0.f, 0.f}, {10.f, 20.f}, {20.f, 10.f}, {40.f, 0.f}, {40.f, 80.f}};
        linear_chain_tween<point_f> contr {milliseconds {1000}, {points}};
        contr.add_output(&out1);

        contr.start();
        REQUIRE(out1.X == Approx(00.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(00.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(05.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(10.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(10.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(20.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(15.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(15.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(20.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(10.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(30.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(05.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(40.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(00.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(40.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(40.0f).epsilon(0.0001f));
        contr.update(125ms);
        REQUIRE(out1.X == Approx(40.0f).epsilon(0.0001f));
        REQUIRE(out1.Y == Approx(80.0f).epsilon(0.0001f));
    }
}

TEST_CASE("Core.Tween.Animation")
{
    SUBCASE("Normal")
    {
        std::string out;

        frame_animation ani;
        ani.Frames = {{"0", 10ms}, {"1", 10ms}, {"2", 10ms}, {"3", 10ms}, {"4", 10ms}};

        frame_animation_tween contr {ani.get_duration(), {ani}};
        contr.add_output(&out);

        contr.start(playback_style::Normal);
        contr.update(10ms);
        REQUIRE(out == "0");
        contr.update(10ms);
        REQUIRE(out == "1");
        contr.update(10ms);
        REQUIRE(out == "2");
        contr.update(10ms);
        REQUIRE(out == "3");
        contr.update(10ms);
        REQUIRE(out == "4");
        contr.update(10ms);
        REQUIRE(out == "4");
    }

    SUBCASE("Alternated")
    {
        std::string out;

        frame_animation ani;
        ani.Frames = {{"0", 100ms}, {"1", 100ms}, {"2", 100ms}, {"3", 100ms}, {"4", 50ms}};

        frame_animation_tween contr {ani.get_duration() * 2, {ani}};
        contr.add_output(&out);

        contr.start(playback_style::Alternated);
        contr.update(1ms);
        REQUIRE(out == "0");
        contr.update(101ms);
        REQUIRE(out == "1");
        contr.update(101ms);
        REQUIRE(out == "2");
        contr.update(101ms);
        REQUIRE(out == "3");
        contr.update(101ms);
        REQUIRE(out == "4");
        contr.update(101ms);
        REQUIRE(out == "3");
        contr.update(101ms);
        REQUIRE(out == "2");
        contr.update(101ms);
        REQUIRE(out == "1");
        contr.update(101ms);
        REQUIRE(out == "0");
    }
}

TEST_CASE("Core.Tween.PlaybackModes")
{
    SUBCASE("Normal")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::Normal);
        for (i32 i {0}; i < 10; i++) {
            contr.update(20ms);
        }
        REQUIRE(out.size() == 5);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 20.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 40.f);
        REQUIRE(out[4] == 50.f);
    }
    SUBCASE("Looped")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::Looped);
        for (i32 i {0}; i < 10; i++) {
            contr.update(20ms);
        }
        REQUIRE(out.size() == 10);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 20.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 40.f);
        REQUIRE(out[4] == 50.f);
        REQUIRE(out[5] == 10.f);
        REQUIRE(out[6] == 20.f);
        REQUIRE(out[7] == 30.f);
        REQUIRE(out[8] == 40.f);
        REQUIRE(out[9] == 50.f);
    }
    SUBCASE("Reversed")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::Reversed);
        for (i32 i {0}; i < 10; i++) {
            contr.update(20ms);
        }
        REQUIRE(out.size() == 6);
        REQUIRE(out[0] == 50.f);
        REQUIRE(out[1] == 40.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 20.f);
        REQUIRE(out[4] == 10.f);
        REQUIRE(out[5] == 0.f);
    }
    SUBCASE("ReversedLooped")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::ReversedLooped);
        for (i32 i {0}; i < 10; i++) {
            contr.update(20ms);
        }
        REQUIRE(out.size() == 11);
        REQUIRE(out[0] == 50.f);
        REQUIRE(out[1] == 40.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 20.f);
        REQUIRE(out[4] == 10.f);
        REQUIRE(out[5] == 0.f);
        REQUIRE(out[6] == 40.f);
        REQUIRE(out[7] == 30.f);
        REQUIRE(out[8] == 20.f);
        REQUIRE(out[9] == 10.f);
        REQUIRE(out[10] == 0.f);
    }
    SUBCASE("Alternated")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::Alternated);
        for (i32 i {0}; i < 10; i++) {
            contr.update(10ms);
        }
        REQUIRE(out.size() == 10);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 20.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 40.f);
        REQUIRE(out[4] == 50.f);
        REQUIRE(out[5] == 40.f);
        REQUIRE(out[6] == 30.f);
        REQUIRE(out[7] == 20.f);
        REQUIRE(out[8] == 10.f);
        REQUIRE(out[9] == 0.f);
    }
    SUBCASE("AlternatedLooped")
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {100}, {0.f, 50.f}};
        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });

        contr.start(playback_style::AlternatedLooped);
        for (i32 i {0}; i < 20; i++) {
            contr.update(10ms);
        }
        REQUIRE(out.size() == 20);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 20.f);
        REQUIRE(out[2] == 30.f);
        REQUIRE(out[3] == 40.f);
        REQUIRE(out[4] == 50.f);
        REQUIRE(out[5] == 40.f);
        REQUIRE(out[6] == 30.f);
        REQUIRE(out[7] == 20.f);
        REQUIRE(out[8] == 10.f);
        REQUIRE(out[9] == 0.f);
        REQUIRE(out[10] == 10.f);
        REQUIRE(out[11] == 20.f);
        REQUIRE(out[12] == 30.f);
        REQUIRE(out[13] == 40.f);
        REQUIRE(out[14] == 50.f);
        REQUIRE(out[15] == 40.f);
        REQUIRE(out[16] == 30.f);
        REQUIRE(out[17] == 20.f);
        REQUIRE(out[18] == 10.f);
        REQUIRE(out[19] == 0.f);
    }
}

TEST_CASE("Core.Tween.Interval")
{
    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 70.f}};

        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });
        contr.Interval = 100ms;

        contr.start();
        for (i32 i {0}; i <= 1000; i++) {
            contr.update(1ms);
        }
        REQUIRE(out.size() == 11);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 16.f);
        REQUIRE(out[2] == 22.f);
        REQUIRE(out[3] == 28.f);
        REQUIRE(out[4] == 34.f);
        REQUIRE(out[5] == 40.f);
        REQUIRE(out[6] == 46.f);
        REQUIRE(out[7] == 52.f);
        REQUIRE(out[8] == 58.f);
        REQUIRE(out[9] == 64.f);
        REQUIRE(out[10] == 70.f);
    }

    {
        std::vector<f32> out;

        linear_tween<f32> contr {milliseconds {1000}, {10.f, 70.f}};

        contr.Value.Changed.connect([&out](f32 val) { out.push_back(val); });
        contr.Interval = 500ms;

        contr.start();
        for (i32 i {0}; i <= 1000; i++) {
            contr.update(1ms);
        }
        REQUIRE(out.size() == 3);
        REQUIRE(out[0] == 10.f);
        REQUIRE(out[1] == 40.f);
        REQUIRE(out[2] == 70.f);
    }
}

TEST_CASE("Core.Tween.LambdaFunction")
{
    {
        i32 out {-1};

        auto constexpr lambda {[](f64 val) -> i32 {
            if (val <= 0.25) {
                return 0;
            }
            if (val <= 0.5) {
                return 1;
            }
            if (val <= 0.75) {
                return 2;
            }

            return 3;
        }};

        function_tween<lambda> contr {milliseconds {1000}, {}};
        contr.Value = out;
        contr.add_output(&out);

        contr.start();
        contr.update(250ms);
        REQUIRE(out == 0);
        contr.update(250ms);
        REQUIRE(out == 1);
        contr.update(250ms);
        REQUIRE(out == 2);
        contr.update(250ms);
        REQUIRE(out == 3);
    }
}
