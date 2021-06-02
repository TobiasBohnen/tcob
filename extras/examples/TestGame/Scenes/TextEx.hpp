#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class TextEx : public Scene {
public:
    TextEx(Game& game);
    ~TextEx();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    Text _text0;
    Text _text1;
    Text _text2;
    Text _text3;
};
