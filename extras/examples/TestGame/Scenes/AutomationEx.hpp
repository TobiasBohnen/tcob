#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class AutomationEx : public Scene {
public:
    AutomationEx(Game& game);
    ~AutomationEx();

protected:
    void on_start() override;

    void on_draw(RenderTarget& target) override;

    void on_update(MilliSeconds deltaTime) override;
    void on_fixed_update(MilliSeconds deltaTime) override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    SpriteBatch _layer1;
    std::vector<AutomationQueue> _queues;
    std::vector<std::shared_ptr<AutomationBase>> _autos;
};
