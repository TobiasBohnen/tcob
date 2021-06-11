#pragma once

#include <tcob/tcob.hpp>
using namespace tcob;
using namespace tcob::gl;

class MiscScene : public Scene {
public:
    MiscScene(Game& game);
    ~MiscScene();

protected:
    void on_start() override;

    void on_draw(RenderTarget& target) override;

    void on_update(f64 deltaTime) override;
    void on_fixed_update(f64 deltaTime) override;

    void on_key_down(const KeyboardEvent& ev) override;
    void on_mouse_motion(const MouseMotionEvent& ev) override;

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
    Sound sound0;
    Music music0;
};
