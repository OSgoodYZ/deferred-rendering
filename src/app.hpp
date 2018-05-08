#ifndef PX_CG_APP_HPP
#define PX_CG_APP_HPP

#include <string>
#include <memory>
#include "glfw.hpp"
#include "scene.hpp"

namespace px
{
class App;
}

class px::App
{
public:
    static const unsigned int DEFAULT_WINDOW_HEIGHT;
    static const unsigned int DEFAULT_WINDOW_WIDTH;
    static const char * DEFAULT_WINDOW_TITLE;

public:
    enum class Key : int
    {
        W = GLFW_KEY_W,
        S = GLFW_KEY_S,
        A = GLFW_KEY_A,
        D = GLFW_KEY_D,
        Q = GLFW_KEY_Q,
        E = GLFW_KEY_E,
        M = GLFW_KEY_M,
        F = GLFW_KEY_F,
        L = GLFW_KEY_L,
        B = GLFW_KEY_B,
        P = GLFW_KEY_P,
        O = GLFW_KEY_O,
        N = GLFW_KEY_N,
        Up = GLFW_KEY_UP,
        Down = GLFW_KEY_DOWN,
        Shift = GLFW_KEY_LEFT_SHIFT,
        Space = GLFW_KEY_SPACE,
        Escape = GLFW_KEY_ESCAPE
    };
    enum class MouseButton : int
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT
    };

    static App* instance();
    virtual void init(bool fullscreen);
    virtual bool run();
    virtual void close();
    [[noreturn]]
    virtual void error(std::string const &msg);
    [[noreturn]]
    virtual void error(std::string const &msg, int code);

    virtual void setWindowSize(unsigned int width, unsigned int height);
    virtual void setWindowTitle(const char title[]);
    virtual void setWindowTitle(std::string title);
    virtual void setFullscreen(bool enable);
    virtual void setScene(std::shared_ptr<Scene> scene);
    virtual void setCursorPosition(float x, float y);
    virtual void showCursor(bool enable);

    inline unsigned int windowHeight() const noexcept
    { return window_height_; }
    inline unsigned int windowWidth() const noexcept
    { return window_width_; }
    inline std::string const &windowTitle() const noexcept
    { return window_title_; }
    inline int framebufferWidth() const noexcept
    { return framebuffer_width_; }
    inline int framebufferHeight() const noexcept
    { return framebuffer_height_; }
    inline float timeGap() const noexcept { return time_gap; }
    inline int fps() const noexcept { return fps_; }
    inline bool fullscreen() const noexcept { return fullscreen_; }
    inline std::shared_ptr<Scene> scene() { return scene_; }

    bool keyPressed(Key key);
    bool keyHold(Key key);
    bool keyTriggered(Key key);
    bool mouseButtonPressed(MouseButton btn);
    inline glm::vec2 const &cursorPosition() { return cursor_position_; }
    inline glm::vec2 const &scrollOffset() { return scroll_offset_; }

    virtual ~App();

protected:
    App();

    virtual void pollEvents();
    virtual void updateTimeGap();
    virtual void updateWindowSize(int width, int height);
    virtual void updateFramebufferSize(int width, int height);
    virtual void updateCursorPosition(float x, float y);
    virtual void updateKeyPressedCondition(int glfw_key, int glfw_action);
    virtual void updateScrollOffset(float x_offset, float y_offset);

protected:
    GLFWwindow *window;
    float time_gap;

private:
    unsigned int window_width_;
    unsigned int window_height_;
    std::string window_title_;
    int framebuffer_width_;
    int framebuffer_height_;
    std::shared_ptr<Scene> scene_;
    glm::vec2 cursor_position_;
    glm::vec2 scroll_offset_;
    int fps_;
    bool fullscreen_;

    unsigned int prev_window_width_;
    unsigned int prev_window_height_;
    int prev_window_pos_x_;
    int prev_window_pos_y_;

    std::array<bool, GLFW_KEY_LAST+1> last_key_pressed_;
    std::array<bool, GLFW_KEY_LAST+1> current_key_pressed_;
public:
    App &operator=(App const &) = delete;
    App &operator=(App &&) = delete;
};


#endif // PX_CG_APP_HPP