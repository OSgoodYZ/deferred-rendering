#ifndef PX_CG_SCENES_DEFERRED_RENDER_HPP
#define PX_CG_SCENES_DEFERRED_RENDER_HPP

#include <vector>

#include "controllable_camera.hpp"
#include "shaders/text.hpp"
#include "shaders/skybox.hpp"
#include "shaders/deferred_lighting.hpp"
#include "shaders/forward_phong.hpp"
#include "shaders/lamp.hpp"

namespace px { namespace scene
{
class DeferredRenderBenchmark;
}}

class px::scene::DeferredRenderBenchmark : public scene::ControllableCamera
{
public:
    bool deferred_rendering_flag;
    bool show_light_sources;
    bool display_spheres;
    bool pause;
    int show_only;
    int max_lights_deferred;

    DeferredRenderBenchmark();
    ~DeferredRenderBenchmark() override;

    void init() override;
    void update(float dt) override;
    void render() override;
    void resize(unsigned int width, unsigned int height) override;

    void resetCamera();
    void deferredRender();
    void forwardRender();
    void renderGUI();

protected:
    class Spheres
    {
    public:
        Spheres();
        ~Spheres();

        void init(float start_x, float grid_size_x, float end_x,
                  float start_y, float grid_size_y, float end_y,
                  float h, float radius);
        void update(float dt);
        void render(Shader *shader);
        inline std::size_t size() { return position.size(); }
    protected:
        unsigned int vao;
        unsigned int vbo[5];
        unsigned int texture[4];
        std::vector<glm::vec3> position;
    private:
        std::vector<float> speed_;
        std::size_t n_indices_;
        float avg_height_;
    } spheres;
    class Floor
    {
    public:
        Floor();
        ~Floor();
        void init(float texture_repeat_x, float texture_repeat_y);
        void render(Shader *shader);

        void position(glm::vec3 const &pos);
        void scale(glm::vec3 const &scal);
        inline glm::vec3 const &position() const noexcept { return position_; }
        inline glm::vec3 const &scale() const noexcept { return scale_; }
    protected:
        unsigned int vao;
        unsigned int vbo;
        unsigned int texture[4];
    private:
        glm::vec3 position_;
        glm::vec3 scale_;
    } floor;
    class Lights : public shader::Lamp
    {
    public:
        Lights();
        ~Lights() override = default;
        void init(float start_x, float grid_size_x, float end_x,
                  float start_y, float grid_size_y, float end_y,
                  float h, float radius);
        void update(float dt);
        void render();
        inline std::size_t size() { return position_.size(); }
        inline std::vector<glm::vec3> const &position() const noexcept { return position_; }
        inline std::vector<glm::vec3> const &color() const noexcept { return color_; }
        inline std::vector<glm::vec3> const &attenuation() const noexcept { return attenuation_; }
        inline glm::vec3 const &moveRadius() const noexcept { return move_radius_; }
        void moveRadius(glm::vec3 const &r);
    protected:
        std::vector<glm::vec3> position_;
        std::vector<glm::vec3> color_;
        std::vector<glm::vec3> attenuation_;
        std::vector<glm::vec3> speed_;
        std::vector<glm::vec3> dest_;
        std::vector<glm::vec3> init_position_;
        glm::vec3 move_radius_;
    } lights;
    class Skybox : public shader::Skybox
    {
    public:
        Skybox();
        ~Skybox() override = default;
        void init();
    } skybox;
    class TextShader : public shader::Text
    {
    public:
        TextShader();
        ~TextShader() override = default;
        void init() override;
    } text;
protected:
    shader::DeferredLightingPass deferred_pass_shader;
    shader::DeferredLighting deferred_lighting_shader;
    shader::ForwardPhong forward_shader;
};

#endif // PX_CG_SCENES_DEFERRED_RENDER_HPP
