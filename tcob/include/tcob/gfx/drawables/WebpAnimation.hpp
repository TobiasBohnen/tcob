// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/tcob_config.hpp>

#include <tcob/gfx/Material.hpp>
#include <tcob/gfx/Quad.hpp>
#include <tcob/gfx/Transformable.hpp>
#include <tcob/gfx/drawables/Drawable.hpp>
#include <tcob/gfx/gl/GLRenderer.hpp>
#include <tcob/gfx/gl/GLTexture.hpp>

struct WebPAnimDecoder;
struct WebPData;

namespace tcob {
namespace detail {
    enum class DecodeResult {
        NewFrame,
        OldFrame,
        NoMoreFrames,
        DecodeFailure
    };

    class WebpAnimDecoder final {
    public:
        explicit WebpAnimDecoder(const std::string& file);
        ~WebpAnimDecoder();

        auto get_frame(i32 timestamp, u8** buffer) -> DecodeResult;

        void reset();

        auto size() const -> SizeU;

    private:
        SizeU _size { SizeU::Zero };
        i32 _currentTimeStamp { 0 };

        WebPAnimDecoder* _dec;
        WebPData* _data;
    };
}

class WebpAnimation final : public RectTransformable, public Drawable {
public:
    WebpAnimation();
    ~WebpAnimation();

    WebpAnimation(const WebpAnimation& other);
    auto operator=(const WebpAnimation& other) -> WebpAnimation&;

    auto load(const std::string& file) -> bool;

    void start(bool looped = false);
    void restart();
    void toggle_pause();
    void stop();

    auto is_running() const -> bool;

    auto material() const -> ResourcePtr<Material>;
    void material(ResourcePtr<Material> material);

    void update(f64 deltaTime) override;

    void draw(gl::RenderTarget& target) override;

private:
    Quad _quad {};
    gl::StaticQuadRenderer _renderer {};
    ResourcePtr<Material> _material;
    std::shared_ptr<gl::Texture2D> _texture;

    SizeU _frameSize { SizeU::Zero };
    f64 _elapsedTime { 0 };

    bool _isRunning { false };
    bool _looped { false };

    std::shared_ptr<detail::WebpAnimDecoder> _decoder;
};
}
