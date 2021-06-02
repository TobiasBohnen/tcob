#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class MiscScene : public Scene {
public:
    MiscScene(Game& game);
    ~MiscScene();

    virtual void draw(RenderTarget& target) override;
    void update(f64 deltaTime) override;
    void fixed_update(f64 deltaTime) override;

protected:
    void on_start() override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;
    void on_controller_axis_motion(const ControllerAxisEvent& ev) override;

private:
    void setFloat(float x);

    TileMap tileMap;
    SpriteBatch layer1;
    ParticleSystem partSystem1;

    RenderTexture rtt;

    int rttID { 0 };
    int scrollID { 0 };

    ResourcePtr<Font> font;
    Text text;
    PointCloud pointcloud;
    AutomationQueue _rvc;
    NinePatch ninepatch;

    WebpAnimation webp;
    Music sound;
};
