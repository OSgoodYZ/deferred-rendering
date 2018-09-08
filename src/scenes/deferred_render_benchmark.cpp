#include "deferred_render_benchmark.hpp"
#include "stb_image.h"
#include "config.h"
#include "util/random.hpp"
#include "util/shape_generator.hpp"

#include <iostream>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#ifndef MAX_LIGHT_SOURCES
#define MAX_LIGHT_SOURCES 0
#endif
#ifndef INIT_LIGHT_NUM
#define INIT_LIGHT_NUM 0
#endif

using namespace px;

scene::DeferredRenderBenchmark::DeferredRenderBenchmark()
    : scene::ControllableCamera(),
      deferred_rendering_flag(true),
      show_light_sources(false),
      display_spheres(true),
      pause(false),
      show_only(-1)
{}

scene::DeferredRenderBenchmark::~DeferredRenderBenchmark()
{}

void scene::DeferredRenderBenchmark::init()
{
    max_lights_deferred = std::min(INIT_LIGHT_NUM, shader::DeferredLighting::MAX_LIGHTS_PER_BATCH);

    // init camera controller mode
    scene::ControllableCamera::init();
    // add some background
    skybox.init();
    // construct main scene
    resetCamera();
    if (pause) App::instance()->showCursor(true);
    else App::instance()->showCursor(false);

    constexpr float field_height = -.2f;
    constexpr float scene_width = 25.f;
    constexpr float scene_height = 25.f;

#ifndef LIGHTS_OBJ_NUMBER
#define LIGHTS_OBJ_NUMBER 0
#endif
    constexpr float light_gap_x = scene_width / (LIGHTS_OBJ_NUMBER+1e-5f);
    constexpr float light_gap_y = scene_height / (LIGHTS_OBJ_NUMBER+1e-5f);
#ifndef SPHERES_OBJ_NUMBER
#define SPHERES_OBJ_NUMBER 0
#endif
    constexpr float object_gap_x = (scene_width - 1) / (SPHERES_OBJ_NUMBER+1e-5f);
    constexpr float object_gap_y = (scene_height - 1) / (SPHERES_OBJ_NUMBER+1e-5f);

    constexpr float margin_x = light_gap_x * 2.5f;
    constexpr float margin_y = light_gap_y * 2.5f;
    constexpr float light_avg_height = 0.25f;
    constexpr float light_ball_radius = .025f;

    const glm::vec3 light_movement(light_gap_x*2.f, (light_avg_height-field_height)*.5f, light_gap_y*2.f);
    floor.init(scene_width+margin_x+margin_x, scene_height+margin_y+margin_y);
    floor.position(glm::vec3(-scene_width*.5f-margin_x, field_height, -scene_height*.5f-margin_y));
    floor.scale(glm::vec3(scene_width+margin_x+margin_x, 1.f, scene_height+margin_y+margin_y));
    lights.init(-scene_width*.5f, light_gap_x, scene_width*.5f,
                -scene_height*.5f, light_gap_y, scene_height*.5f,
                light_avg_height, light_ball_radius);
    lights.moveRadius(light_movement);
    spheres.init(-scene_width*.5f, object_gap_x, scene_width*.5f,
                 -scene_height*.5f, object_gap_y, scene_height*.5f,
                 light_avg_height, light_ball_radius*7.5f);

    // init GUI-based shaders
    text.init();

    // init main rendering shaders
    deferred_pass_shader.init();
    deferred_lighting_shader.init();
    forward_shader.init();

    deferred_pass_shader.activate(true);
    deferred_pass_shader.set("global_ambient", glm::vec3(.5f, .5f, .5f));
    forward_shader.activate(true);
    forward_shader.set("global_ambient", glm::vec3(.5f, .5f, .5f));
    forward_shader.activate(false);
}

void scene::DeferredRenderBenchmark::resize(unsigned int width, unsigned int height)
{
    ControllableCamera::resize(width, height);
    deferred_pass_shader.setBufferSize(width, height);
    deferred_lighting_shader.setBufferSize(width, height);
}

void scene::DeferredRenderBenchmark::update(float dt)
{
    auto app = App::instance();
    if (app->keyTriggered(App::Key::P))
    {
        if (!pause) app->showCursor(true);
        else app->showCursor(false);
        pause = !pause;
    }
    if (app->keyTriggered(App::Key::M))
        deferred_rendering_flag = !deferred_rendering_flag;
    if (app->keyTriggered(App::Key::O))
        display_spheres = !display_spheres;
    if (app->keyTriggered(App::Key::L))
        show_light_sources = !show_light_sources;
    if (app->keyTriggered(App::Key::F))
        app->setFullscreen(!app->fullscreen());
    if (app->keyTriggered(App::Key::B))
        resetCamera();
    if (app->keyTriggered(App::Key::N) && deferred_rendering_flag)
    {
        if (show_only < -1) show_only = -1;
        show_only += 1;
        if (show_only > 5) show_only = -1;
    }
    if (app->keyHold(App::Key::Up) &&
            ((deferred_rendering_flag && max_lights_deferred < static_cast<int>(lights.size())) ||
             (!deferred_rendering_flag && max_lights_deferred <= shader::ForwardPhong::MAX_LIGHTS)))
        ++max_lights_deferred;
    else if (app->keyHold(App::Key::Down) && max_lights_deferred > 0)
    {
        if (deferred_rendering_flag)
            --max_lights_deferred;
        else
            max_lights_deferred = std::min(shader::ForwardPhong::MAX_LIGHTS, max_lights_deferred) - 1;
    }

    if (pause) return;

    scene::ControllableCamera::update(dt);
    if (display_spheres) spheres.update(dt);
    lights.update(dt);
}

void scene::DeferredRenderBenchmark::render()
{
    if (deferred_rendering_flag)
        deferredRender();
    else
        forwardRender();
    renderGUI();
}

void scene::DeferredRenderBenchmark::resetCamera()
{
    camera().position(glm::vec3(-25, 18.f, 0));
    camera().pitch(43.f);
    camera().roll(0.f);
    camera().yaw(90.f);
}

#define __LIGHT_SET_HELPER(shader)  \
shader.set("lights[" + std::to_string(counter) + "].position", p[i]);         \
shader.set("lights[" + std::to_string(counter) + "].ambient",  l[i]*.0f);     \
shader.set("lights[" + std::to_string(counter) + "].diffuse",  l[i]*1.2f);    \
shader.set("lights[" + std::to_string(counter) + "].specular", l[i]);         \
shader.set("lights[" + std::to_string(counter) + "].coef",     a[i]);

void scene::DeferredRenderBenchmark::deferredRender()
{
    deferred_pass_shader.activate(true);
    if (display_spheres) spheres.render(&deferred_pass_shader);
    floor.render(&deferred_pass_shader);
    deferred_pass_shader.activate(false);

    deferred_lighting_shader.activate(true);
    deferred_lighting_shader.set("show_only", show_only);
    auto counter = 0;
    if (max_lights_deferred > 0 && shader::DeferredLighting::MAX_LIGHTS_PER_BATCH > 0)
    {
        const auto &p = lights.position();
        const auto &l = lights.color();
        const auto &a = lights.attenuation();
        for (int i = 0, tot = static_cast<int>(lights.size());
             i < tot && i < max_lights_deferred; ++i)
        {
            __LIGHT_SET_HELPER(deferred_lighting_shader)
            ++counter;
            if (counter == shader::DeferredLighting::MAX_LIGHTS_PER_BATCH && i + 1 < max_lights_deferred)
            {
                deferred_lighting_shader.renderCache(counter, deferred_pass_shader);
                counter = 0;
            }
        }
    }
    deferred_lighting_shader.render(counter, deferred_pass_shader);
    deferred_lighting_shader.activate(false);

    deferred_pass_shader.extractDepthBuffer();
    if (show_light_sources) lights.render();
    skybox.render();
}

void scene::DeferredRenderBenchmark::forwardRender()
{
    forward_shader.activate(true);
    auto counter = 0;
    if (max_lights_deferred > 0 && shader::ForwardPhong::MAX_LIGHTS > 0)
    {
        auto tot = std::min(max_lights_deferred, shader::ForwardPhong::MAX_LIGHTS);
        const auto &p = lights.position();
        const auto &l = lights.color();
        const auto &a = lights.attenuation();
        for (decltype(tot) i = 0; i < tot; ++i)
        {
            __LIGHT_SET_HELPER(forward_shader)

            ++counter;
        }
    }
    forward_shader.set("n_lights", counter);
    if (display_spheres) spheres.render(&forward_shader);
    floor.render(&forward_shader);
    forward_shader.activate(false);

    if (show_light_sources) lights.render();
    skybox.render();
}

void scene::DeferredRenderBenchmark::renderGUI()
{
    constexpr float vertical_gap = 20.f;
    constexpr float scale = .4f;
    const static glm::vec4 color(1.f);

    auto app = App::instance();
    auto screen_width = app->framebufferWidth();
    auto screen_height = app->framebufferHeight();

    auto h = 10.f;
    // fps, right top corner
    text.render("FPS: " + std::to_string(app->fps()),
                app->framebufferWidth() - 10, h, scale, color,
                screen_width, screen_height, shader::Text::Anchor::RightTop);
    if (show_only == 0)
        text.render("Ambient Color",
                    app->framebufferWidth() - 10, h+vertical_gap, scale, color,
                    screen_width, screen_height, shader::Text::Anchor::RightTop);
    else if (show_only == 1)
        text.render("Diffuse Map",
                    app->framebufferWidth() - 10, h+vertical_gap, scale, color,
                    screen_width, screen_height, shader::Text::Anchor::RightTop);
    else if (show_only == 2)
        text.render("Specular Map",
                    app->framebufferWidth() - 10, h+vertical_gap, scale, color,
                    screen_width, screen_height, shader::Text::Anchor::RightTop);
    else if (show_only == 3)
        text.render("Position Map",
                    app->framebufferWidth() - 10, h+vertical_gap, scale, color,
                    screen_width, screen_height, shader::Text::Anchor::RightTop);
    else if (show_only == 4)
        text.render("Normal Map",
                    app->framebufferWidth() - 10, h+vertical_gap, scale, color,
                    screen_width, screen_height, shader::Text::Anchor::RightTop);

    // rendering mode, left top corner
    text.render(std::string("Rendering Mode: ") + (deferred_rendering_flag ?
                     "Deferred Rendering" : "Forward Rendering"),
                10, h, scale, color,
                screen_width, screen_height, shader::Text::Anchor::LeftTop);
    // # of lights
    h += vertical_gap;
    text.render("Number of Lights: " + std::to_string(
            deferred_rendering_flag ? max_lights_deferred : std::min(shader::ForwardPhong::MAX_LIGHTS, max_lights_deferred)
            ) +
                "/ " + std::to_string(lights.size()),
                10, h, scale, color,
                screen_width, screen_height, shader::Text::Anchor::LeftTop);
    // # of spheres
    h += vertical_gap;
    text.render("Number of Sphere Objects: " + std::to_string(spheres.size()),
                10, h, scale, color,
                screen_width, screen_height, shader::Text::Anchor::LeftTop);

    // pause or not
    if (pause)
    text.render("Pausing......",
                10, screen_height-20, scale*1.2f, color,
                screen_width, screen_height, shader::Text::Anchor::LeftBottom);
}

scene::DeferredRenderBenchmark::Lights::Lights()
    : shader::Lamp()
{}

void scene::DeferredRenderBenchmark::Lights::moveRadius(glm::vec3 const &r)
{
    move_radius_.x = std::abs(r.x);
    move_radius_.y = std::abs(r.y);
    move_radius_.z = std::abs(r.z);
}

void scene::DeferredRenderBenchmark::Lights::init(float start_x, float grid_size_x, float end_x,
                                                  float start_y, float grid_size_y, float end_y,
                                                  float h, float radius)
{
    auto grid_x = static_cast<int>((end_x - start_x) / grid_size_x)+1;
    auto grid_y = static_cast<int>((end_y - start_y) / grid_size_y)+1;

    auto half_x = grid_size_x * .5f;
    auto half_y = grid_size_y * .5f;

    init_position_.clear(); color_.clear(); attenuation_.clear();
    init_position_.reserve(3*grid_x*grid_y);
    color_.reserve(3*grid_x*grid_y);
    attenuation_.reserve(3*grid_x*grid_y);
    start_x += half_x;
    for (auto x = 0; x < grid_x; ++x)
    {
        auto tmp_y = start_y + half_y;
        for (auto y = 0; y < grid_y; ++y)
        {
            init_position_.emplace_back(start_x, h, tmp_y);
            color_.emplace_back(rnd()*.5f + .5f, rnd()*.5f + .5f, rnd()*.5f + .5f);
            attenuation_.emplace_back(0.f, 0.f, 12.5f+2.5f*(rnd()-.5f));

            tmp_y += grid_size_y;
        }
        start_x += grid_size_x;
    }
    std::shuffle(init_position_.begin(), init_position_.end(),
                 std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
    position_.resize(init_position_.size());
    dest_.resize(init_position_.size());
    speed_.resize(init_position_.size());
    std::memcpy(position_.data(), init_position_.data(), sizeof(glm::vec3)*init_position_.size());
    std::memcpy(dest_.data(), init_position_.data(), sizeof(glm::vec3)*init_position_.size());
    std::memset(speed_.data(), 0, sizeof(glm::vec3)*init_position_.size());

    auto sphere = generator::sphere(12, radius);

    shader::Lamp::init();
    shader::Lamp::setVertices(sphere.first.data(), sphere.first.size(),
                              sphere.second.data(), sphere.second.size());
    shader::Lamp::setInstances(reinterpret_cast<const float*>(position().data()),
                               reinterpret_cast<const float*>(color().data()),
                               size());
}

void scene::DeferredRenderBenchmark::Lights::update(float dt)
{
    constexpr float eps = 1e-6f;
    glm::vec3 m;
    auto tot = static_cast<int>(size());
#pragma omp parallel for num_threads(6)
    for (auto i = 0; i < tot; ++i)
    {
        m.x = dt*speed_[i].x;
        m.y = dt*speed_[i].y;
        m.z = dt*speed_[i].z;

                auto stopped = std::abs(m[0]) < eps && std::abs(m[1]) < eps && std::abs(m[2]) < eps;

#define __DEST_REACHED(axis)   \
        ((speed_[i].axis > 0 && (position_[i].axis + m.axis > dest_[i].axis)) ||  \
         (speed_[i].axis < 0 && (position_[i].axis + m.axis < dest_[i].axis)))

        if (stopped || __DEST_REACHED(x))
        {
            speed_[i].x = .1f + rnd()*.5f;
            dest_[i].x = init_position_[i].x +
                         (speed_[i].x > 0 ? moveRadius().x : -moveRadius().x);
        }
        if (stopped || __DEST_REACHED(y))
        {
            speed_[i].y = .1f + rnd()*.5f;
            dest_[i].y = init_position_[i].y +
                         (speed_[i].y > 0 ? moveRadius().y : -moveRadius().y);
        }
        if (stopped || __DEST_REACHED(z))
        {
            speed_[i].z = .1f + rnd()*.5f;
            dest_[i].z = init_position_[i].z +
                         (speed_[i].z > 0 ? moveRadius().z : -moveRadius().z);
        }
        position_[i] += m;
    }
#undef __DEST_REACHED
}

void scene::DeferredRenderBenchmark::Lights::render()
{
    shader::Lamp::setInstances(reinterpret_cast<const float*>(position().data()),
                               nullptr,
                               size());
    shader::Lamp::activate(true);
    shader::Lamp::render(GL_TRIANGLES);
    shader::Lamp::activate(false);
}


scene::DeferredRenderBenchmark::Skybox::Skybox()
    : shader::Skybox()
{}
void scene::DeferredRenderBenchmark::Skybox::init()
{
    int xp_w, xp_h;
    int xn_w, xn_h;
    int yp_w, yp_h;
    int yn_w, yn_h;
    int zp_w, zp_h;
    int zn_w, zn_h;
    int ch;

    constexpr auto right_face  = ASSET_PATH "/texture/skybox/right.jpg";
    constexpr auto left_face   = ASSET_PATH "/texture/skybox/left.jpg";
    constexpr auto top_face    = ASSET_PATH "/texture/skybox/top.jpg";
    constexpr auto bottom_face = ASSET_PATH "/texture/skybox/bottom.jpg";
    constexpr auto back_face   = ASSET_PATH "/texture/skybox/back.jpg";
    constexpr auto front_face  = ASSET_PATH "/texture/skybox/front.jpg";

    auto xp = stbi_load(right_face, &xp_w, &xp_h, &ch, 3);
    if (!xp) error("Failed to load texture: " + std::string(right_face));
    auto xn = stbi_load(left_face, &xn_w, &xn_h, &ch, 3);
    if (!xn) error("Failed to load texture: " + std::string(left_face));
    auto yp = stbi_load(top_face, &yp_w, &yp_h, &ch, 3);
    if (!yp) error("Failed to load texture: " + std::string(top_face));
    auto yn = stbi_load(bottom_face, &yn_w, &yn_h, &ch, 3);
    if (!yn) error("Failed to load texture: " + std::string(bottom_face));
    auto zp = stbi_load(back_face, &zp_w, &zp_h, &ch, 3);
    if (!zp) error("Failed to load texture: " + std::string(back_face));
    auto zn = stbi_load(front_face, &zn_w, &zn_h, &ch, 3);
    if (!zn) error("Failed to load texture: " + std::string(front_face));

    shader::Skybox::init(xp, xp_w, xp_h, xn, xn_w, xn_h,
                         yp, yp_w, yp_h, yn, yn_w, yn_h,
                         zp, zp_w, zp_h, zn, zn_w, zn_h);

    stbi_image_free(xp);
    stbi_image_free(xn);
    stbi_image_free(yp);
    stbi_image_free(yn);
    stbi_image_free(zp);
    stbi_image_free(zn);
}

scene::DeferredRenderBenchmark::TextShader::TextShader()
    : shader::Text()
{}

void scene::DeferredRenderBenchmark::TextShader::init()
{
    shader::Text::init();
    static const unsigned char font_dat[] = {
#include "font/Just_My_Type.dat"
    };
    setFont(font_dat, sizeof(font_dat), 40);
}

scene::DeferredRenderBenchmark::Floor::Floor()
    : vao(0), vbo(0), texture{0}, position_(0.f), scale_(1.f)
{}

scene::DeferredRenderBenchmark::Floor::~Floor()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(4, texture);
}

void scene::DeferredRenderBenchmark::Floor::init(float texture_repeat_x, float texture_repeat_y)
{
    glDeleteVertexArrays(1, &vao); vao = 0;
    glDeleteBuffers(1, &vbo); vbo = 0;
    glDeleteTextures(4, texture);
    texture[0] = 0; texture[1] = 0; texture[2] = 0; texture[3] = 0;

    float vertices[] = {
            //   vertex                  texture                      norm          tangent
            // x    y   z                u    v                    x   y   z       x   y   z
            0.f, 0.f, 1.f,              0.f, texture_repeat_y,   0.f, 1.f, 0.f,   1.f, 0.f, 0.f,
            0.f, 0.f, 0.f,              0.f, 0.f,                0.f, 1.f, 0.f,   1.f, 0.f, 0.f,
            1.f, 0.f, 0.f, texture_repeat_x, 0.f,                0.f, 1.f, 0.f,   1.f, 0.f, 0.f,

            0.f, 0.f, 1.f,              0.f, texture_repeat_y,   0.f, 1.f, 0.f,   1.f, 0.f, 0.f,
            1.f, 0.f, 0.f, texture_repeat_x, 0.f,                0.f, 1.f, 0.f,   1.f, 0.f, 0.f,
            1.f, 0.f, 1.f, texture_repeat_x, texture_repeat_y,   0.f, 1.f, 0.f,   1.f, 0.f, 0.f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(4, texture);

    int w, h, ch;
    auto ptr = stbi_load(ASSET_PATH "/texture/floor7_d.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[0], w, h, ptr, RGB, REPEAT);         // diffuse texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/floor7_n.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[1], w, h, ptr, RGB, REPEAT);         // normal texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/floor7_s.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[2], w, h, ptr, RGB, REPEAT);         // specular texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/floor7_h.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[3], w, h, ptr, RGB, REPEAT);         // height/displacement texture
    stbi_image_free(ptr);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, nullptr);
    glEnableVertexAttribArray(1);   // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*3));
    glEnableVertexAttribArray(2);   // norm
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*5));
    glEnableVertexAttribArray(3);   // tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*8));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void scene::DeferredRenderBenchmark::Floor::render(Shader *shader)
{
    shader->set("use_tangent", 1);
    shader->set("material.ambient", glm::vec3(1.f));
    shader->set("material.shininess", 32.f);
    shader->set("material.parallax_scale", 0.f);
    shader->set("material.displace_scale", 0.f);
    shader->set("material.displace_mid", 0.5f);

    auto model = glm::scale(glm::translate(glm::mat4(1.f), position()), scale());
    shader->set("model", model);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[3]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void scene::DeferredRenderBenchmark::Floor::position(glm::vec3 const &pos)
{
    position_ = pos;
}

void scene::DeferredRenderBenchmark::Floor::scale(glm::vec3 const &scal)
{
    scale_ = scal;
}

scene::DeferredRenderBenchmark::Spheres::Spheres()
    : vao(0), vbo{0}, texture{0}
{}

scene::DeferredRenderBenchmark::Spheres::~Spheres()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(5, vbo);
    glDeleteTextures(4, texture);
}

void scene::DeferredRenderBenchmark::Spheres::init(float start_x, float grid_size_x, float end_x,
                                                   float start_y, float grid_size_y, float end_y,
                                                   float height, float radius)
{
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(5, vbo);
    vbo[0] = 0;
    vbo[1] = 0;
    vbo[2] = 0;
    vbo[3] = 0;
    vbo[4] = 0;
    glDeleteTextures(4, texture);
    texture[0] = 0;
    texture[1] = 0;
    texture[2] = 0;
    texture[3] = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(5, vbo);
    glGenTextures(4, texture);

    auto grid_x = static_cast<int>((end_x - start_x) / grid_size_x);
    auto grid_y = static_cast<int>((end_y - start_y) / grid_size_y);

    auto half_x = grid_size_x * .5f;
    auto half_y = grid_size_y * .5f;

    position.clear();
    position.reserve(grid_x*grid_y);
    start_x += half_x;
    for (auto x = 0; x < grid_x; ++x)
    {
        auto tmp_y = start_y + half_y;
        for (auto y = 0; y < grid_y; ++y)
        {
            position.emplace_back(start_x, height, tmp_y);
            tmp_y += grid_size_y;
        }
        start_x += grid_size_x;
    }
    avg_height_ = height;
    speed_.resize(position.size());
    std::memset(speed_.data(), 0, sizeof(float)*speed_.size());

    int w, h, ch;
    auto ptr = stbi_load(ASSET_PATH "/texture/fire_d.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[0], w, h, ptr, RGB, REPEAT);         // diffuse texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/fire_n.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[1], w, h, ptr, RGB, REPEAT);         // normal texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/fire_s.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[2], w, h, ptr, RGB, REPEAT);         // specular texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/fire_h.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[3], w, h, ptr, RGB, REPEAT);         // height/displacement texture
    stbi_image_free(ptr);

    std::vector<float> vertices, uv, norm, tangent;
    std::vector<unsigned short> vertex_indices;
    std::tie(vertices, vertex_indices, uv, norm, tangent) = generator::sphereWithNormUVTangle(48, radius);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*vertex_indices.size(), vertex_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*uv.size(), uv.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);   // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*norm.size(), norm.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);   // norm
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangent.size(), tangent.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);   // tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    n_indices_ = vertex_indices.size();
}

void scene::DeferredRenderBenchmark::Spheres::update(float dt)
{
    constexpr float eps = 1e-4f;
    for (auto i = 0, tot = static_cast<int>(position.size()); i < tot; ++i)
    {
        if (position[i].y > avg_height_ + .5f)
        {
            speed_[i] = - rnd()*.1f - .1f;
        }
        else if (position[i].y < avg_height_ - .5f)
        {
            speed_[i] = rnd()*.1f + .1f;
        }
        else if (std::abs(speed_[i]) < eps)
        {
            speed_[i] = rnd()*.1f - .2f;
        }
        position[i].y += speed_[i]*dt;
    }
}

void scene::DeferredRenderBenchmark::Spheres::render(Shader *shader)
{
    shader->set("use_tangent", 1);
    shader->set("material.ambient", glm::vec3(1.0f, 0.45f, 0.f));
    shader->set("material.shininess", 50.f);
    shader->set("material.parallax_scale", 0.f);
    shader->set("material.displace_scale", 0.02f);
    shader->set("material.displace_mid", 0.5f);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[3]);


    for (const auto &p : position)
    {
        auto model = glm::translate(glm::mat4(1.f), p);
        shader->set("model", model);
        glDrawElements(GL_TRIANGLES, n_indices_, GL_UNSIGNED_SHORT, nullptr);
    }
//    std::cout << n_indices_/3 << std::endl;

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}