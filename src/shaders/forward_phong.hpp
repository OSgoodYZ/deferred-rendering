#ifndef PX_CG_SHADERS_FORWARD_PHONG_HPP
#define PX_CG_SHADERS_FORWARD_PHONG_HPP

#include "shader.hpp"
#include "deferred_lighting.hpp"

namespace px { namespace shader
{
class ForwardPhong;
}}

class px::shader::ForwardPhong : public Shader
{
public:
    static const int MAX_LIGHTS; // this should not be larger than the one defined in the shader

    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

    using PointLight = DeferredLighting::PointLight;

    ForwardPhong();
    ~ForwardPhong() override = default;

    void init();
};
#endif // PX_CG_SHADERS_FORWARD_PHONG_HPP
