#include <functional>
#include <cstring>
#include "app.hpp"
#include "error.hpp"

using namespace px;

const unsigned int App::DEFAULT_WINDOW_HEIGHT = 480;
const unsigned int App::DEFAULT_WINDOW_WIDTH = 640;
const char *App::DEFAULT_WINDOW_TITLE = "App powered by Pei Xu";

App* App::instance()
{
    static App* instance = nullptr;

    if (instance == nullptr)
        instance = new App;

    return instance;
}

App::App()
    : window(nullptr),
      window_width_(DEFAULT_WINDOW_WIDTH),
      window_height_(DEFAULT_WINDOW_HEIGHT),
      window_title_(DEFAULT_WINDOW_TITLE)
{}

App::~App()
{
    glfwDestroyWindow(window);
}

void App::error(std::string const &msg)
{
    throw AppError(msg);
}

void App::error(std::string const &msg, int code)
{
    throw AppError(msg, code);
}

void App::init(bool fullscreen)
{
    if (!scene())
        throw AppError("Failed to initialize App without specific scene.");

    static auto window_size_callback = [](GLFWwindow * win, int width, int height)
    {
        if (win == App::instance()->window)
            App::instance()->updateWindowSize(width, height);
    };
    static auto framebuffer_size_callback = [](GLFWwindow *win, int width, int height)
    {
        if (win == App::instance()->window)
            App::instance()->updateFramebufferSize(width, height);
    };
    static auto cursor_position_callback = [](GLFWwindow *win, double x, double y)
    {
        if (win == App::instance()->window)
            App::instance()->updateCursorPosition(static_cast<float>(x),
                                                  static_cast<float>(y));
    };
    static auto keypress_callback = [](GLFWwindow* win, int key, int, int action, int mods)
    {
        if (win == App::instance()->window)
            App::instance()->updateKeyPressedCondition(key, action);

    };
    static auto scroll_callback = [](GLFWwindow *win, double x_offset, double y_offset)
    {
        if (win == App::instance()->window)
            App::instance()->updateScrollOffset(static_cast<float>(x_offset),
                                                static_cast<float>(y_offset));
    };

    if (window) glfwDestroyWindow(window);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifdef USE_MSAA
    glfwWindowHint(GLFW_SAMPLES, 16);
#endif

    // init window
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(1, 1, windowTitle().c_str(), nullptr, nullptr);
    if (!window) error("Failed to initialize window");

    // init OpenGL
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) error("Failed to initialize GLEW.");
    glEnable(GL_DEPTH_TEST);
#ifdef USE_MSAA
    glEnable(GL_MULTISAMPLE);
#endif
    glDepthFunc(GL_LESS);

    // set up callback fns
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, keypress_callback);
    glfwSetScrollCallback(window, scroll_callback);

    time_gap = -1;
    fps_ = 0;
    prev_window_height_ = window_height_;
    prev_window_width_ = window_width_;
    prev_window_pos_x_ = -1234567;
    prev_window_pos_y_ = -1234567;
    std::memset(current_key_pressed_.data(), 0, sizeof(bool)*current_key_pressed_.size());
    std::memset(last_key_pressed_.data(), 0, sizeof(bool)*last_key_pressed_.size());
    glfwShowWindow(window);
    setFullscreen(fullscreen);
    scene()->init();
}

bool App::run()
{
    // pull events
    pollEvents();

    if (glfwWindowShouldClose(window))
        return false;

    if (scene() == nullptr)
        throw AppError("Failed to run App without specific scene.");

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene()->update(timeGap());
    scene()->render();

    glfwSwapBuffers(window);
    return true;
}

void App::close()
{
    glfwSetWindowShouldClose(window, 1);
}

void App::setWindowSize(unsigned int width, unsigned int height)
{
    glfwSetWindowSize(window, width, height);
    glfwGetWindowSize(window,
                      reinterpret_cast<int*>(&window_width_),
                      reinterpret_cast<int*>(&window_height_));
}

void App::setWindowTitle(const char title[])
{
    window_title_ = std::string(title);
    glfwSetWindowTitle(window, title);
}

void App::setWindowTitle(std::string title)
{
    window_title_ = std::move(title);
    glfwSetWindowTitle(window, window_title_.data());
}

void App::setFullscreen(bool enable)
{
    if (!window) return;

    auto m = glfwGetWindowMonitor(window);
    if (!m) m = glfwGetPrimaryMonitor();
    auto v = glfwGetVideoMode(m);

    if (enable)
    {
        prev_window_width_ = window_width_;
        prev_window_height_ = window_height_;
        glfwGetWindowPos(window, &prev_window_pos_x_, &prev_window_pos_y_);

        glfwSetWindowMonitor(window, m, 0, 0, v->width, v->height, GL_DONT_CARE);
        fullscreen_ = true;
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr,
                             prev_window_pos_x_ == -1234567 ? (v->width - window_width_)/2 : prev_window_pos_x_,
                             prev_window_pos_y_ == -1234567 ? (v->height - window_height_)/2 :prev_window_pos_y_,
                             prev_window_width_, prev_window_height_,
                             GL_DONT_CARE);
        glfwSetWindowSize(window, prev_window_width_, prev_window_height_);
        fullscreen_ = false;
    }

    int w, h;
    glfwGetWindowSize(window, &w, &h);
    updateWindowSize(w, h);
    glfwGetFramebufferSize(window, &w, &h);
    updateFramebufferSize(w, h);
}

void App::setScene(std::shared_ptr<px::Scene> scene)
{
    scene_ = std::move(scene);
}

void App::setCursorPosition(float x, float y)
{
    glfwSetCursorPos(window, static_cast<double>(x), static_cast<double>(y));
    updateCursorPosition(x, y);
}

void App::showCursor(bool enable)
{
    glfwSetInputMode(window, GLFW_CURSOR,
                     enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void App::pollEvents()
{
    std::swap(current_key_pressed_, last_key_pressed_);
    std::memcpy(current_key_pressed_.data(), last_key_pressed_.data(),
                sizeof(bool)*current_key_pressed_.size());

    updateTimeGap();
    glfwPollEvents();

    if (keyTriggered(Key::Escape))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void App::updateTimeGap()
{
    static decltype(glfwGetTime()) last_time = -1;
    static decltype(glfwGetTime()) last_counter_push_time = -1;
    static decltype(fps_) counter = 0;

    if (time_gap == -1 || last_time == -1)
    {   // first time to count time
        last_time = glfwGetTime();
        time_gap = 0;
        fps_ = 0;
        counter = 0;
        last_counter_push_time = last_time;
    }
    else
    {
        auto current_time = glfwGetTime();
        time_gap = static_cast<decltype(time_gap)>(current_time - last_time);
        last_time = current_time;

        if (current_time - last_counter_push_time > 1.f)
        {
            fps_ = counter;
            counter = 0;
            last_counter_push_time = current_time;
        }
        else
        {
            ++counter;
        }
    }
}

void App::updateWindowSize(int width, int height)
{
    window_width_ = static_cast<decltype(window_width_)>(width);
    window_height_ = static_cast<decltype(window_height_)>(height);
}

void App::updateFramebufferSize(int width, int height)
{
    framebuffer_height_ = height;
    framebuffer_width_ = width;
    if (scene())
        scene()->resize(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}


void App::updateCursorPosition(float x, float y)
{
    cursor_position_.x = x;
    cursor_position_.y = y;
}

void App::updateKeyPressedCondition(int glfw_key, int glfw_action)
{

    current_key_pressed_[glfw_key] = glfw_action != GLFW_RELEASE;
}

void App::updateScrollOffset(float x_offset, float y_offset)
{
    scroll_offset_.x = x_offset;
    scroll_offset_.y = y_offset;
}

bool App::keyPressed(Key key)
{
    return current_key_pressed_[static_cast<int>(key)] && !last_key_pressed_[static_cast<int>(key)];
}

bool App::keyHold(Key key)
{
    return current_key_pressed_[static_cast<int>(key)] && last_key_pressed_[static_cast<int>(key)];
}

bool App::keyTriggered(Key key)
{
    return current_key_pressed_[static_cast<int>(key)];
}

bool App::mouseButtonPressed(MouseButton btn)
{
    return glfwGetMouseButton(window, static_cast<int>(btn)) == GLFW_PRESS;
}
