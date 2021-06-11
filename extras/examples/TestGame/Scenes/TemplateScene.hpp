#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class TemplateScene : public Scene {
public:
    TemplateScene(Game& game);
    ~TemplateScene();

protected:
    void on_start() override;

    void on_draw(RenderTarget& target) override;

    void on_update(f64 deltaTime) override;
    void on_fixed_update(f64 deltaTime) override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    SpriteBatch layer1;
};
