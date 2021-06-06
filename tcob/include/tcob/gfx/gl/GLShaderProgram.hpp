// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <unordered_map>

#include <tcob/core/Helper.hpp>
#include <tcob/core/data/Size.hpp>
#include <tcob/core/data/Transform.hpp>
#include <tcob/gfx/gl/GLObject.hpp>

namespace tcob::gl {
class ShaderProgram : public Object {
public:
    ShaderProgram();
    ~ShaderProgram() override;

    auto create(const char* vertexShaderSource, const char* fragmentShaderSource) -> bool;

    void use() const;

    template <typename T>
    void set_uniform(const std::string& name, T&& x) const
    {
        const i32 loc { uniform_location(name) };
        set_uniform(loc, x);
    }

    template <typename T>
    void set_uniform(i32 loc, T&& x) const
    {
        if (loc != -1)
            set_uniform_impl(loc, x);
    }

    void set_uniform_matrix4(const std::string& name, const mat4& x) const;

protected:
    void do_destroy() override;

private:
    void set_uniform_impl(i32 loc, i32 x) const;
    void set_uniform_impl(i32 loc, const ivec2& x) const;
    void set_uniform_impl(i32 loc, const ivec3& x) const;
    void set_uniform_impl(i32 loc, const ivec4& x) const;
    void set_uniform_impl(i32 loc, const SizeI& x) const;
    void set_uniform_impl(i32 loc, const PointI& x) const;

    void set_uniform_impl(i32 loc, u32 x) const;
    void set_uniform_impl(i32 loc, const uvec2& x) const;
    void set_uniform_impl(i32 loc, const uvec3& x) const;
    void set_uniform_impl(i32 loc, const uvec4& x) const;
    void set_uniform_impl(i32 loc, const SizeU& x) const;
    void set_uniform_impl(i32 loc, const PointU& x) const;

    void set_uniform_impl(i32 loc, f32 x) const;
    void set_uniform_impl(i32 loc, const vec2& x) const;
    void set_uniform_impl(i32 loc, const vec3& x) const;
    void set_uniform_impl(i32 loc, const vec4& x) const;
    void set_uniform_impl(i32 loc, const SizeF& x) const;
    void set_uniform_impl(i32 loc, const PointF& x) const;

    auto uniform_location(const std::string& name) const -> i32;

    mutable std::unordered_map<std::string, i32> _uniformLocations {};
};
}