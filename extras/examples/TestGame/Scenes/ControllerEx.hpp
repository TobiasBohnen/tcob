#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class ControllerEx : public Scene {
public:
    ControllerEx(Game& game);
    ~ControllerEx();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    Text _text;
    Text _controllerDesc;
};
