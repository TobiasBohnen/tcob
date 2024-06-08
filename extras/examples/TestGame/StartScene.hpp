// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"
#include <iostream>

using namespace tcob;

using Function = std::function<std::shared_ptr<scene>(game&)>;
struct SceneEntry {
    std::string Name;
    Function    Func;
};

class start_scene : public scene {

public:
    start_scene(game& g);
    ~start_scene() override;

protected:
    void on_start() override;

    void on_draw_to(render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(keyboard::event& ev) override;
    void on_mouse_motion(mouse::motion_event& ev) override;

private:
    std::shared_ptr<text> _text;
};
