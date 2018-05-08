#include "forward_phong.hpp"
#include "config.h"

using namespace px;
#ifndef LIGHTING_BATCH_SIZE
#define LIGHTING_BATCH_SIZE 0
#endif
const int shader::ForwardPhong::MAX_LIGHTS = LIGHTING_BATCH_SIZE;

const char *shader::ForwardPhong::VERTEX_SHADER =
#include "shaders/glsl/deferred_lighting_pass.vs"
;
const char *shader::ForwardPhong::FRAGMENT_SHADER =
#include "shaders/glsl/phong.fs"
;

shader::ForwardPhong::ForwardPhong()
    : Shader()
{}

void shader::ForwardPhong::init()
{
    std::string tmp(FRAGMENT_SHADER);
    tmp.insert(tmp.find_first_of("c")+4, "\n#define MAX_LIGHTS " + std::to_string(std::max(1, MAX_LIGHTS)));
    Shader::init(VERTEX_SHADER, tmp.c_str());
    Shader::activate(true);
    set("material.diffuse", 0);
    set("material.normal", 1);
    set("material.specular", 2);
    set("material.displace", 3);
    Shader::activate(false);
}
