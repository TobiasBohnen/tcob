#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class StartScene : public Scene {
public:
    StartScene(Game& game);
    ~StartScene();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    Text _text;
};
