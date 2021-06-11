#include "AutomationEx.hpp"
#include "../StartScene.hpp"
#include <iomanip>

AutomationEx::AutomationEx(Game& game)
    : Scene(game)
{
}

AutomationEx::~AutomationEx()
{
}

MilliSeconds duration { 2000 };

void AutomationEx::on_start()
{
    auto& resMgr { game().resources() };

    auto circleMat { resMgr.get<Material>("res", "mat-circle") };
    f32 endX { 1.f * 800 / 600 - 0.1f };
    f32 y { -0.125f };

    Sprite circle {};
    circle.material(circleMat);
    circle.size({ 0.1f, 0.1f });

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Blue);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(0).position(point); } };
        auto auto0 { make_shared_automation<LinearFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }) };
        auto auto1 { make_shared_automation<LinearFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position()) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Red);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(1).position(point); } };
        auto auto0 { make_shared_automation<SmoothstepFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }) };
        auto auto1 { make_shared_automation<SmoothstepFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position()) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Yellow);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(2).position(point); } };
        auto auto0 { make_shared_automation<SmootherstepFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }) };
        auto auto1 { make_shared_automation<SmootherstepFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position()) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Green);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(3).position(point); } };
        auto auto0 { make_shared_automation<PowerFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }, 2.f) };
        auto auto1 { make_shared_automation<PowerFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position(), 2.f) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Orange);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(4).position(point); } };
        auto auto0 { make_shared_automation<PowerFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }, 0.75f) };
        auto auto1 { make_shared_automation<PowerFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position(), 0.75f) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::Brown);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(5).position(point); } };
        auto auto0 { make_shared_automation<InversePowerFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }, 2.f) };
        auto auto1 { make_shared_automation<InversePowerFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position(), 2.f) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::WhiteSmoke);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(6).position(point); } };
        auto auto0 { make_shared_automation<CubicBezierFunction>(duration, circle.position(), PointF { endX / 4, circle.position().Y + 0.25f }, PointF { endX / 4 * 3, circle.position().Y - 0.25f }, PointF { endX, circle.position().Y }) };
        auto auto1 { make_shared_automation<CubicBezierFunction>(duration, PointF { endX, circle.position().Y }, PointF { endX / 4 * 3, circle.position().Y - 0.25f }, PointF { endX / 4, circle.position().Y + 0.25f }, circle.position()) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }

    circle.position({ 0, y += 0.125f });
    circle.color(Colors::LawnGreen);
    _layer1.add_sprite(circle);
    {
        auto& queue { _queues.emplace_back() };
        auto lamb { [this](const PointF& point) { _layer1.at(7).position(point); } };
        auto auto0 { make_shared_automation<SquareWaveFunction<PointF>>(duration, circle.position(), PointF { endX, circle.position().Y }, 1.f, 0.f) };
        auto auto1 { make_shared_automation<SquareWaveFunction<PointF>>(duration, PointF { endX, circle.position().Y }, circle.position(), 1.f, 0.f) };
        auto0->ValueChanged.connect(lamb);
        auto1->ValueChanged.connect(lamb);
        queue.push(auto0, auto1);
        _autos.insert(_autos.end(), { auto0, auto1 });
    }
    ////////////////////////////////////////////////////////////
    for (auto& queue : _queues) {
        queue.start(true);
    }
}

void AutomationEx::on_draw(RenderTarget& target)
{
    _layer1.draw(target);
}

void AutomationEx::on_update(MilliSeconds deltaTime)
{
    for (auto& queue : _queues) {
        queue.update(deltaTime);
    }
    _layer1.update(deltaTime);
}

void AutomationEx::on_fixed_update(MilliSeconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << game().stats().average_fps();
    stream << " best FPS:" << game().stats().best_fps();
    stream << " worst FPS:" << game().stats().worst_fps();

    game().window().title("TestGame " + stream.str());
}

void AutomationEx::on_key_down(const KeyboardEvent& ev)
{
    switch (ev.Code) {
    case Scancode::D1:
        for (auto& auto0 : _autos) {
            auto0->interval(duration / 2);
        }
        break;
    case Scancode::D2:
        for (auto& auto0 : _autos) {
            auto0->interval(duration / 5);
        }
        break;
    case Scancode::D3:
        for (auto& auto0 : _autos) {
            auto0->interval(duration / 10);
        }
        break;
    case Scancode::D4:
        for (auto& auto0 : _autos) {
            auto0->interval(duration / 50);
        }
        break;
    case Scancode::D5:
        for (auto& auto0 : _autos) {
            auto0->interval(duration / 100);
        }
        break;
    case Scancode::D6:
        for (auto& auto0 : _autos) {
            auto0->interval(MilliSeconds { 0 });
        }
        break;
    case Scancode::BACKSPACE:
        game().pop_current_scene();
        break;
    default:
        break;
    }
}

void AutomationEx::on_mouse_motion(const MouseMotionEvent& ev)
{
}