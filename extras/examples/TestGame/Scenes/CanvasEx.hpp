#pragma once
#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::gl;

class CanvasEx : public Scene {
public:
    CanvasEx(Game& game);
    ~CanvasEx();

protected:
    void on_start() override;

    void on_draw(RenderTarget& target) override;

    void on_update(f64 deltaTime) override;
    void on_fixed_update(f64 deltaTime) override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    void prepare_canvas();
    void paint_to_canvas();

    tcob::gl::RenderTexture _rtt;
    std::unique_ptr<LuaScript> _script;

    std::shared_ptr<SpriteBatch> _layer1;
    ScriptApi _api;
};
