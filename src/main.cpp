#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glfw.hpp"
#include "app.hpp"
#include "scenes/deferred_render_benchmark.hpp"

int main(int argc, char *argv[])
{
    px::glfw::init();

    auto a = px::App::instance();
    a->setScene(std::make_shared<px::scene::DeferredRenderBenchmark>());
    a->init(false);
    while (a->run());
    a->close();

    px::glfw::terminate();
    return 0;
}
