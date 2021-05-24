#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class AutomationEx : public Scene {
public:
    AutomationEx(Game& game);
    ~AutomationEx();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

private:
    SpriteBatch _layer1;
    std::vector<AutomationQueue> _queues;
    std::vector<std::shared_ptr<AutomationBase>> _autos;
};
