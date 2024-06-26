// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Misc.hpp"

#include "tcob/audio/Music.hpp"
#include "tcob/audio/SoundFont.hpp"
#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include <cmath>

#include <iomanip>
#include <memory>
#include <string>

using namespace std::chrono_literals;
using namespace tcob::literals;

MiscScene::MiscScene(game& game)
    : scene(game)
{
    game.DropFile.connect([](path const& file) {
        std::cout << file << "\n";
    });
}

MiscScene::~MiscScene() = default;

f32 pointSize {1.0f};

void MiscScene::on_start()
{
    auto* resGrp {locate_service<assets::library>().get_group("res")};

    _htmlDoc = std::make_shared<html::document>(
        html::document::config {.AssetGroup      = resGrp,
                                .Fonts           = resGrp->get<font_family>("Roboto"),
                                .DefaultFontSize = 32,
                                .Window          = &get_window()});

    _htmlDoc->AnchorClick.connect([](auto&& ev) { std::cout << ev << "\n"; });

    _htmlDoc->Bounds = {{0.f, 0.f}, {800, 1200}};

    _htmlDoc->load("res/test.html");

    get_root_node()->attach_entity(_htmlDoc);

    rng                                  rnd;
    std::vector<std::shared_ptr<sprite>> sprites;
    for (i32 i {0}; i < 1000; i++) {
        f32 x {rnd(0.0f, 6000.f)};
        f32 y {rnd(0.0f, 6000.f)};

        auto sprite0 {std::make_shared<sprite>()};
        sprite0->Material = resGrp->get<material>("spriteMat");
        sprite0->Bounds   = {{x, y}, {150, 150}};
        sprites.push_back(sprite0);
    }
    _layer1 = std::make_shared<static_sprite_batch>(sprites);

    // _particleSystem0 = *resGrp->get_asset_ptr<particle_system>("system1");
    _particleSystem0->Material = resGrp->get<material>("particleMat");

    auto& emi0     = _particleSystem0->create_emitter();
    emi0.SpawnArea = {0, 60, 600, 150};
    emi0.Lifetime  = 1000ms;
    emi0.IsLooping = true;
    emi0.SpawnRate = 50;
    emi0.Template  = {
         .Acceleration = std::minmax(15.f, 15.f),
         .Direction    = std::minmax(175_deg, 185_deg),
         .Lifetime     = std::minmax(7500ms, 12500ms),
         .Scale        = std::minmax(0.75f, 1.5f),
         .Size         = {30, 30},
         .Speed        = std::minmax(30.f, 150.f),
         .Spin         = std::minmax(-150_deg, 150_deg),
         .Texture      = "snowflake",
         .Transparency = std::minmax(0.0f, 0.55f)};

    _particleSystem0->ParticleUpdate.connect([](particle& p) {
        if (std::any_cast<i32>(p.UserData) == 0) {
            if (p.get_lifetime_ratio() <= 0.95f) {
                p.Direction = p.Direction - 180_deg;
                p.UserData  = 1;
                p.Color     = colors::Red;
            }
        } else if (std::any_cast<i32>(p.UserData) == 1) {
            if (p.get_lifetime_ratio() <= 0.75f) {
                p.Direction = p.Direction - 45_deg;
                p.UserData  = 2;
                p.Color     = colors::Yellow;
            }
        } else if (std::any_cast<i32>(p.UserData) == 2) {
            if (p.get_lifetime_ratio() <= 0.50f) {
                p.Direction = p.Direction + 90_deg;
                p.UserData  = 3;
                p.Color     = colors::Blue;
            }
        }
    });

    _particleSystem0->ParticleUpdate.connect([](particle& p) {
        p.Color.A = static_cast<u8>(255.f * p.get_lifetime_ratio());
    });
    _particleSystem0->Position = {0.1f, 0.1f};

    _font = resGrp->get<font>("raster-font2");

    _text         = std::make_shared<text>(_font);
    _text->Text   = "SÖ5";
    // text.text("ö±\nad to");
    _text->Bounds = {{800, 600}, {400, 60}};
    _text->Style  = {.Color = colors::White, .Alignment = {horizontal_alignment::Left, vertical_alignment::Top}, .KerningEnabled = true};

    for (f32 y = 0; y < pointSize * _numPoints.Height; y += pointSize) {
        for (f32 x = 0; x < pointSize * _numPoints.Width; x += pointSize) {
            auto& v     = _pointCloud->create_point();
            v.Position  = {(x + pointSize / 2), (y + pointSize / 2)};
            v.Color     = {255, 255, 255, 255};
            v.TexCoords = {x / (pointSize * _numPoints.Width),
                           y / (pointSize * _numPoints.Height),
                           0};
            _points.push_back(v.Position);
        }
    }

    _pointCloud->Material = resGrp->get<material>("pointMat2");
    _uniPoints.bind_base(_pointCloud->Material->Shader->get_uniform_block_binding("Point"));
    _uniPoints.update(_numPoints, 0);
    _pointTween.Interval = 20ms;
    _pointTween.Value.Changed.connect([&](point_f value) {
        for (usize i {0}; i < _numPoints.Height; i++) {
            usize idx     = i * _numPoints.Width + _pointIdx;
            auto& p       = _pointCloud->get_point_at(idx);
            p.Position[1] = _points[idx][1] + value.Y;
        }

        _pointIdx++;
        if (_pointIdx >= _numPoints.Width) {
            _pointIdx = 0;
        }
    });

    _aniTexSprite           = _layer0.create_sprite();
    _aniTexSprite->Bounds   = {{450, 0}, {320, 240}};
    _aniTexSprite->Material = resGrp->get<material>("aniSpriteMat");

    auto sprite1      = _layer0.create_sprite();
    sprite1->Bounds   = {point_f::Zero, {320, 240}};
    sprite1->Material = resGrp->get<material>("uniforms-buffer-test");
    _uniBuf.bind_base(1);
    std::array<f32, 5> bufData {1, 0, 0, 1, 0.5f};
    _uniBuf.update<f32>(bufData, 0);

    _sound_mp3  = *resGrp->get<sound>("mp3-test");
    _sound_wav  = *resGrp->get<sound>("wav-test");
    _sound_ogg  = *resGrp->get<sound>("ogg-test");
    _sound_opus = *resGrp->get<sound>("opus-test");
    _sound_flac = *resGrp->get<sound>("flac-test");
    _sound_it   = *resGrp->get<sound>("it-test");

    _music0                = resGrp->get<music>("test");
    _soundFont0            = resGrp->get<sound_font>("test");
    _midi0                 = std::make_unique<music>();
    _midi0->DecoderContext = _soundFont0;

    /*
        auto lambda {
            [&](milliseconds deltaTime) {
                if (!_flag) {
                }
            }
        };
        _timer.Tick.connect(lambda);
        _timer.start(1ms, true);


    _poly.set_material(resGrp->get<material>("emptyMat"));
    std::vector<vertex> vs{
        {.Position = {0, 0}, .Color = colors::Red.as_array()},
        {.Position = {100, 0}, .Color = colors::Red.as_array()},
        {.Position = {50, 50}, .Color = colors::Red.as_array()},
        {.Position = {100, 100}, .Color = colors::Green.as_array()},
        {.Position = {0, 100}, .Color = colors::Green.as_array()}};
    _poly.set_geometry(vs, std::vector<u32>{0, 1, 2, 1, 3, 2, 4, 2, 3, 0, 2, 4});
       */

    {
        _tileMap = std::make_shared<tilemap>(tileset {{
            {1, {"stone1"}},
            {2, {"stone2"}},
            {3, {"stone3"}},
            {4, {"stone4"}},
            {5, {"dirt1"}},
            {6, {"dirt2"}},
            {7, {"dirt3"}},
            {8, {"dirt4"}},
            {9, {"grass1"}},
            {10, {"grass2"}},
            {11, {"grass3"}},
            {12, {"grass4"}},
        }});

        _tileMap->Material = resGrp->get<material>("mat-tiles");
        _tileMap->Grid     = {.TileSize = {12, 12}};

        rng           rnd;
        u32 const     tmHeight {800}, tmWidth {600};
        tilemap_layer layer;
        layer.Tiles.reserve(tmWidth * tmWidth);
        for (u32 i {0}; i < tmWidth * tmHeight; i++) {
            layer.Tiles.push_back(rnd(1, 12));
        }
        layer.Size = {tmWidth, tmHeight};
        _tileMap->add_layer(layer);
    }

    auto q0 {get_root_node()->create_child()};
    auto ent {std::make_shared<simple_entity>()};
    ent->Drawable = _tileMap;
    q0->attach_entity(ent);

    //   auto renderQ1 {std::make_shared<render_queue>(_rq1Cam)};
    // _rq1Cam->set_size({400, 400});
    // _rq1Cam->set_offset({10, 10});
    //  renderQ1->add_drawable(_tileMap);
    // add_render_queue(201, renderQ1);
}

void MiscScene::on_finish()
{
    _flag = true;
    _timer.stop();
    _music0->stop();
}

void MiscScene::on_draw_to(render_target& target)
{
    /*

    _text.draw_to(target);
    _layer0.draw_to(target);    _poly.render_to_target(target);
      */
    _layer1->draw_to(target);
    _htmlDoc->draw_to(target);
    _pointCloud->draw_to(target);
    _particleSystem0->draw_to(target);
}

void MiscScene::on_update(milliseconds deltaTime)
{
    /*
    _text.update(deltaTime);
    _rvc.update(deltaTime);

    */
    _layer0.update(deltaTime);
    _layer1->update(deltaTime);
    _particleSystem0->update(deltaTime);
    //  _htmlDoc->update(deltaTime);
    _pointCloud->update(deltaTime);
    _pointTween.update(deltaTime);
    _tileMap->update(deltaTime);

    asset_ptr<animated_texture> aniTex = locate_service<assets::library>().get_group("res")->get<texture>("test-ani");
    aniTex->update(deltaTime);
}

void MiscScene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << "avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();
    if (_music0.is_ready()) {
        stream << "|" << _music0->get_duration().count() / 1000;
        stream << "|" << _music0->get_playback_position().count() / 1000;
    }

    if (_soundFont0.is_ready()) {
        stream << " soundfont ready! ";
    }

    get_window().Title = "TestGame " + stream.str();
}

void MiscScene::on_key_down(keyboard::event& ev)
{
    float moveFactor {10};
    auto& window {get_window()};
    auto& camera {*window.Camera};
    auto& resMgr {locate_service<assets::library>()};

    if (ev.ScanCode == scan_code::D1) {
        _particleSystem0->restart();
    } else if (ev.ScanCode == scan_code::D2) {
    } else if (ev.ScanCode == scan_code::D3) {
        if (!resMgr.is_loading_complete()) {
            return;
        }
        resMgr.unload_all_groups();
        resMgr.load_all_groups();

        get_window().Cursor->ActiveMode = "cursor1";
        _text->force_reshape();
        /* static std::map<asset_status, std::string> statusMap{{asset_status::Loaded, "Loaded"}, {asset_status::Loading, "Loading"}, {asset_status::Unloaded, "Unloaded"}, {asset_status::Created, "Created"}, {asset_status::Error, "Error"}};

            auto& resMgr = get_game().AssetLibrary();
            auto  map{resMgr.get_asset_stats("res")};
            for (auto& [assetTypeName, bucketStats] : map.Buckets) {
                std::cout << assetTypeName << ":" << "\n";
                for (auto& [stat, statCount] : bucketStats.Statuses) {
                    std::cout << "\t" << statusMap[stat] << ": " << statCount << "\n";
                    for (auto& [assetName, assetStat] : bucketStats.Assets) {
                        if (assetStat.Status == stat) {
                            std::cout << "\t" << assetName << "| use count: " << assetStat.UseCount << "\n";
                        }
                    }
                }
            }
            */

    } else if (ev.ScanCode == scan_code::D4) {
        u64 const seed = static_cast<u64>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        using namespace tcob::random;
        std::cout << "----xoroshiro_128_plus" << '\n';
        draw_dice(dice<20, xoroshiro_128_plus> {seed});
        std::cout << "----xoroshiro_128_plus_plus" << '\n';
        draw_dice(dice<20, xoroshiro_128_plus_plus> {seed});
        std::cout << "----xoroshiro_128_star_star" << '\n';
        draw_dice(dice<20, xoroshiro_128_star_star> {seed});
        std::cout << "----xoshiro_256_plus" << '\n';
        draw_dice(dice<20, xoshiro_256_plus> {seed});
        std::cout << "----xoshiro_256_plus_plus" << '\n';
        draw_dice(dice<20, xoshiro_256_plus_plus> {seed});
        std::cout << "----xoshiro_256_star_star" << '\n';
        draw_dice(dice<20, xoshiro_256_star_star> {seed});
        std::cout << "----split_mix_64" << '\n';
        draw_dice(dice<20, split_mix_64> {seed});
        std::cout << "----xorshift_64" << '\n';
        draw_dice(dice<20, xorshift_64> {seed});
        std::cout << "----xorshift_64_star" << '\n';
        draw_dice(dice<20, xorshift_64_star> {seed});
        std::cout << "----split_mix_32" << '\n';
        draw_dice(dice<20, split_mix_32> {static_cast<u32>(seed)});
        std::cout << "----game_rand" << '\n';
        draw_dice(dice<20, game_rand> {static_cast<u32>(seed)});
        std::cout << "----well_512_a" << '\n';
        draw_dice(dice<20, well_512_a> {static_cast<u32>(seed)});

        random_number_generator<xorshift_64, normal_distribution> rng0 {static_cast<u32>(seed), 5.f, 3.f};
        std::cout << "----normal xorshift_64" << '\n';
        draw_rng(rng0);

        random_number_generator<xorshift_64, poisson_distribution> rng1 {static_cast<u32>(seed), 15.f};
        std::cout << "----poisson xorshift_64" << '\n';
        draw_rng(rng1);

        random_number_generator<xorshift_64, triangular_distribution> rng2 {static_cast<u32>(seed), 0.f, 30.f, 25.f};
        std::cout << "----triangular xorshift_64" << '\n';
        draw_rng(rng2);

        random_number_generator<xorshift_64, beta_distribution> rng3 {static_cast<u32>(seed), 2.f, 2.f};
        std::cout << "----beta xorshift_64" << '\n';
        draw_rng(rng3, 10);

        random_number_generator<xorshift_64, weibull_distribution> rng4 {static_cast<u32>(seed), 1.f, 10.f};
        std::cout << "----weibull xorshift_64" << '\n';
        draw_rng(rng4);

        random_number_generator<xorshift_64, pareto_distribution> rng5 {static_cast<u32>(seed), 2.f, 10.f};
        std::cout << "----pareto xorshift_64" << '\n';
        draw_rng(rng5);

        random_number_generator<xorshift_64, exponential_distribution> rng6 {static_cast<u32>(seed), 0.5f};
        std::cout << "----exp xorshift_64" << '\n';
        draw_rng(rng6);

        random_number_generator<xorshift_64, discrete_distribution> rng7 {static_cast<u32>(seed), std::array {0.1f, 0.2f, 0.2f, 0.1f, 0.4f}};
        std::cout << "----discrete xorshift_64" << '\n';
        draw_rng(rng7);

        random_number_generator<xorshift_64, cauchy_distribution> rng8 {static_cast<u32>(seed), -2.f, 1.f};
        std::cout << "----cauchy xorshift_64" << '\n';
        draw_rng(rng8);

        random_number_generator<xorshift_64, bernoulli_distribution> rng9 {static_cast<u32>(seed), 0.75f};
        std::cout << "----bernoulli  xorshift_64" << '\n';
        draw_rng(rng9);

        random_number_generator<xorshift_64, bag_distribution> rng10 {static_cast<u32>(seed), 1, 20, 5};
        std::cout << "----bag xorshift_64" << '\n';
        draw_rng(rng10);
    } else if (ev.ScanCode == scan_code::D5) {
        asset_ptr<animated_texture> aniTex = resMgr.get_group("res")->get<texture>("test-ani");
        aniTex->start(true);
        _pointTween.start(playback_style::AlternatedLooped);
    } else if (ev.ScanCode == scan_code::D6) {
        std::array<f32, 5> bufData {0, 1, 0, 1, 1};
        _uniBuf.update<f32>(bufData, 0);
        _aniTexSprite->rotate_by(45);
    } else if (ev.ScanCode == scan_code::D7) {
        asset_ptr<animated_texture> aniTex = resMgr.get_group("res")->get<texture>("test-ani");
        aniTex->toggle_pause();
    } else if (ev.ScanCode == scan_code::A) {
        camera.move_by({-moveFactor, 0});
    } else if (ev.ScanCode == scan_code::D) {
        camera.move_by({moveFactor, 0});
    } else if (ev.ScanCode == scan_code::S) {
        camera.move_by({0.0f, moveFactor});
    } else if (ev.ScanCode == scan_code::W) {
        camera.move_by({0.0f, -moveFactor});
    } else if (ev.ScanCode == scan_code::Q) {
        camera.zoom_by({1.25f, 1.25f});
        _pointCloud->Material->PointSize = (std::ceil(camera.get_zoom().Width * pointSize));
    } else if (ev.ScanCode == scan_code::E) {
        camera.zoom_by({0.8f, 0.8f});
        _pointCloud->Material->PointSize = (std::ceil(camera.get_zoom().Width * pointSize));
    } else if (ev.ScanCode == scan_code::F) {
        sound_font_commands commands;
        auto                addNote = [&commands](auto&& dur, auto&& preset, auto&& note) {
            commands.start_new_section(dur);
            commands.add<note_on_command>(preset, note, 1.0f);
            commands.start_new_section(0.0s);
            commands.add<note_off_all_command>();
        };

        seconds const full {2s};
        seconds const half {full / 2};
        seconds const quarter {half / 2};
        seconds const eighth {quarter / 2};

        for (i32 preset {0}; preset < std::min(4, _soundFont0->get_preset_count()); ++preset) {
            addNote(quarter, preset, midi_note::C4);
            addNote(quarter, preset, midi_note::E4);
            addNote(quarter, preset, midi_note::G4);
            addNote(quarter, preset, midi_note::A4);
            addNote(half, preset, midi_note::G4);
            addNote(eighth, preset, midi_note::F4);
            addNote(eighth, preset, midi_note::E4);
            addNote(quarter, preset, midi_note::D4);
            addNote(half, preset, midi_note::C4);
        }
        _sound_wav = _soundFont0->create_sound(commands);
        _sound_wav.play();

        auto buffer {_soundFont0->create_buffer(commands)};
        auto _ = buffer.save("midiX.opus");
        _      = buffer.save("midiX.ogg");
        _      = buffer.save("midiX.wav");
    } else if (ev.ScanCode == scan_code::C) {
        get_window().Cursor = nullptr;
    } else if (ev.ScanCode == scan_code::KP_1) {
        _sound_mp3.play();
        std::cout << _sound_mp3.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::KP_2) {
        _sound_wav.play();
        std::cout << _sound_wav.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::KP_3) {
        _sound_ogg.play();
        std::cout << _sound_ogg.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::KP_4) {
        _sound_flac.play();
        std::cout << _sound_flac.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::KP_5) {
        _sound_it.play();
        std::cout << _sound_it.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::KP_6) {
        _sound_opus.play();
        std::cout << _sound_opus.get_duration().count() / 1000 << "\n";
    } else if (ev.ScanCode == scan_code::K) {
        _music0->play();
    } else if (ev.ScanCode == scan_code::J) {

    } else if (ev.ScanCode == scan_code::V) {
        _particleSystem0->VisibilityMask = ~_particleSystem0->VisibilityMask;
    } else if (ev.ScanCode == scan_code::L) {
        _midi0->stop();
        [[maybe_unused]] auto _ = _midi0->open("res/audio/42337.mid");
        _midi0->play();
    } else if (ev.ScanCode == scan_code::M) {
        static std::vector<animated_image_encoder::frame> frames;

        if (frames.size() < 5) {
            auto img {get_window().copy_to_image()};
            frames.push_back({img, milliseconds {frames.size() * 250}});
        }

        if (frames.size() == 5) {
            io::ofstream of {"test_ani1.webp"};
            locate_service<animated_image_encoder::factory>().create(".webp")->encode(frames, of);
            frames.clear();
        }
        // get_window().copy_to_image().save("screen1abc.png");
    } else if (ev.ScanCode == scan_code::BACKSPACE) {
        get_game().pop_current_scene();
    }
}

void MiscScene::on_mouse_motion(mouse::motion_event& ev)
{
}

////////////////////////////////////////////////////////////

void simple_entity::on_update(milliseconds deltaTime)
{
    Drawable->update(deltaTime);
}

void simple_entity::on_fixed_update(milliseconds)
{
}

auto simple_entity::can_draw() const -> bool
{
    return Drawable != nullptr;
}

void simple_entity::on_draw_to(render_target& target)
{
    Drawable->draw_to(target);
}
