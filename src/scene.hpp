#ifndef PX_CG_SCENE_HPP
#define PX_CG_SCENE_HPP

#include "camera.hpp"

namespace px
{
class Scene;
}

class px::Scene
{
public:
    Scene();
    virtual ~Scene();

    virtual void init();
    virtual void update(float dt);
    virtual void render();
    virtual void resize(unsigned int width, unsigned int height);

    inline Camera &camera() { return camera_; }

public:
    Camera camera_;

protected:
    unsigned int camera_param_ubo;
};

#endif // PX_CG_SCENE_HPP
