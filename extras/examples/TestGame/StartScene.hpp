#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class StartScene : public Scene {
public:
    StartScene(Game& game);
    ~StartScene();

protected:
    void on_start() override;

    void on_draw(RenderTarget& target) override;

    void on_update(MilliSeconds deltaTime) override;
    void on_fixed_update(MilliSeconds deltaTime) override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    Text _text;
};
