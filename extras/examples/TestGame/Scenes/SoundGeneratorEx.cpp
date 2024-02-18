// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SoundGeneratorEx.hpp"

#include <iomanip>

SoundGeneratorEx::SoundGeneratorEx(game& game)
    : scene(game)
{
    _material->Texture = _canvas.get_texture();
}

SoundGeneratorEx::~SoundGeneratorEx() = default;

rect_f const crect {10, 10, 1000, 500};

void SoundGeneratorEx::on_start()
{
    auto sprite1 {_layer1.create_sprite()};
    sprite1->Material = _material;
    sprite1->Bounds   = {{0, 0}, crect.get_size()};
}

void SoundGeneratorEx::on_draw_to(render_target& target)
{
    _layer1.draw_to(target);
}

void SoundGeneratorEx::on_update(milliseconds deltaTime)
{
    if (_drawWave) {
        draw_wave();
    }

    _layer1.update(deltaTime);
}

void SoundGeneratorEx::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();

    get_window().Title = "TestGame " + stream.str();
}

void SoundGeneratorEx::on_key_down(keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case scan_code::D1:
        _wave1.WaveType = sound_wave::type::Triangle;
        _audioData      = _gen1.create_buffer(_wave1);
        _drawWave       = true;
        break;
    case scan_code::D2:
        _wave1.WaveType = sound_wave::type::Sawtooth;
        _audioData      = _gen1.create_buffer(_wave1);
        _drawWave       = true;
        break;
    case scan_code::D3:
        _wave1.WaveType = sound_wave::type::Sine;
        _audioData      = _gen1.create_buffer(_wave1);
        _drawWave       = true;
        break;
    case scan_code::D4:
        _wave1.WaveType = sound_wave::type::Square;
        _audioData      = _gen1.create_buffer(_wave1);
        _drawWave       = true;
        break;
    case scan_code::D5:
        _wave1.WaveType = sound_wave::type::Noise;
        _audioData      = _gen1.create_buffer(_wave1);
        _drawWave       = true;
        break;
    case scan_code::G:
        _wave1     = _gen1.generate_random();
        _audioData = _gen1.create_buffer(_wave1);
        _drawWave  = true;
        break;
    case scan_code::H:
        _wave1     = _gen1.generate_hit_hurt();
        _audioData = _gen1.create_buffer(_wave1);
        _drawWave  = true;
        break;
    case scan_code::M:
        _wave1     = _gen1.mutate_wave(_wave1);
        _audioData = _gen1.create_buffer(_wave1);
        _drawWave  = true;
        break;
    case scan_code::P:
        _sound1 = _gen1.create_sound(_wave1);
        _sound1.play();
        break;
    case scan_code::L: {
        config::object loadFile;
        loadFile.load("sound_wave0.ini");
        sound_wave::Deserialize(_wave1, loadFile["wave"]);
        _audioData = _gen1.create_buffer(_wave1);
        _drawWave  = true;
    } break;
    case scan_code::S: {
        config::object saveFile;
        sound_wave::Serialize(_wave1, saveFile["wave"]);
        saveFile.save("sound_wave0.ini");

        auto _ = _audioData.save("sound.opus");
        _      = _audioData.save("sound.ogg");
        _      = _audioData.save("sound.wav");
    } break;
    case scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

void SoundGeneratorEx::on_mouse_motion(mouse::motion_event& ev)
{
}

void SoundGeneratorEx::draw_wave()
{
    _canvas.begin_frame(static_cast<size_i>(crect.get_size()), 1.0f);

    color c {colors::DarkRed};
    c.A = 180;
    _canvas.set_fill_style(c);
    _canvas.fill_rect(crect);

    _canvas.set_stroke_width(1);
    _canvas.set_stroke_style(colors::Black);
    std::array<point_f, 2> line {{{crect.X, crect.Height / 2.f}, {crect.right(), crect.Height / 2.f}}};
    _canvas.stroke_lines(line);
    auto audioData {_audioData.get_data()};
    if (!audioData.empty()) {
        f32       currentSample {0.0f};
        f32 const sampleIncrement {audioData.size() / (crect.Width * 2.0f)};
        f32 const sampleScale {crect.Height};

        std::vector<point_f> points;
        points.emplace_back(crect.X, crect.Height / 2.f);

        for (i32 i {1}; i < crect.Width * 2; i++) {
            f32 const sample {std::clamp<f32>(audioData[static_cast<u64>(currentSample)] * sampleScale, -crect.Height / 2.f, crect.Height / 2.f)};
            points.emplace_back(crect.X + i / 2.0f, (crect.Y + crect.Height / 2.0f) - sample);
            currentSample += sampleIncrement;
        }

        _canvas.set_stroke_width(2);
        _canvas.set_stroke_style(colors::MediumSpringGreen);
        _canvas.stroke_lines(points);
    }

    _canvas.end_frame();

    _drawWave = false;
}
