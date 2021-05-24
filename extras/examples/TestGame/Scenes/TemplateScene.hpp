#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class TemplateScene : public Scene {
public:
    TemplateScene(Game& game);
    ~TemplateScene();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    SpriteBatch layer1;
};
