#include "controllable_camera.hpp"
#include "app.hpp"

// FIXME remove debug header file
#include <iostream>

using namespace px;

const float scene::ControllableCamera::FORWARD_SPEED = 5.f;
const float scene::ControllableCamera::BACKWARD_SPEED = 2.5f;
const float scene::ControllableCamera::SIDESTEP_SPEED = 3.f;
const float scene::ControllableCamera::RUNNING_ACCEL = 2.f;
const float scene::ControllableCamera::TURN_SPEED = 60.f;
const float scene::ControllableCamera::MOUSE_SENSITIVITY = 1.f;
const float scene::ControllableCamera::UP_SPEED = 3.f;

const App::Key scene::ControllableCamera::FORWARD_KEY = App::Key::W;
const App::Key scene::ControllableCamera::BACKWARD_KEY = App::Key::S;
const App::Key scene::ControllableCamera::SIDESTEP_LEFT_KEY = App::Key::A;
const App::Key scene::ControllableCamera::SIDESTEP_RIGHT_KEY = App::Key::D;
const App::Key scene::ControllableCamera::TURN_LEFT_KEY = App::Key::Q;
const App::Key scene::ControllableCamera::TURN_RIGHT_KEY = App::Key::E;
const App::Key scene::ControllableCamera::UP_KEY = App::Key::Space;
const App::Key scene::ControllableCamera::RUN_MODIFIER_KEY = App::Key::Shift;
const bool scene::ControllableCamera::MOUSE_CONTROL = true;
const bool scene::ControllableCamera::INVERT_X_AXIS = false;
const bool scene::ControllableCamera::INVERT_Y_AXIS = true;

scene::ControllableCamera::ControllableCamera()
        : Scene()
{
    resetParams();
    resetShortcuts();
}

void scene::ControllableCamera::init()
{
    Scene::init();
    mouse_detected = false;
}

void scene::ControllableCamera::resize(unsigned int width, unsigned int height)
{
    mouse_detected = false;
    Scene::resize(width, height);
}

void scene::ControllableCamera::update(float dt)
{
    // movement controller
    auto app = App::instance();
    // run
    auto acc_coef = app->keyPressed(runModifierKey()) ? runningAccel() : 1.f;
    // for/backward
    if (app->keyTriggered(forwardKey()) && !app->keyTriggered(backwardKey()))
        camera().translate(camera().forward() * (forwardSpeed() * acc_coef * dt));
    else if (app->keyTriggered(backwardKey()) && !app->keyTriggered(forwardKey()))
        camera().translate(camera().forward() * (-backwardSpeed() * acc_coef * dt));
    // left/right sidestep
    if (app->keyTriggered(sidestepLeftKey()) && !app->keyTriggered(sidestepRightKey()))
        camera().translate(camera().strafe() * (sidestepSpeed()* acc_coef * dt));
    else if (app->keyTriggered(sidestepRightKey()) && !app->keyTriggered(sidestepLeftKey()))
        camera().translate(camera().strafe() * (-sidestepSpeed() * acc_coef * dt));
    // left/right turn
    if (app->keyTriggered(turnLeftKey()) && !app->keyTriggered(turnRightKey()))
        camera().yaw(camera().yaw() - turnSpeed()*dt);
    else if (app->keyTriggered(turnRightKey()) && !app->keyTriggered(turnLeftKey()))
        camera().yaw(camera().yaw() + turnSpeed()*dt);
    // up
    if (app->keyTriggered(upKey()) || app->keyTriggered(upKey()))
        camera().translate(camera().up() * (upSpeed()*dt));

    if (mouseControl())
    {
        auto center_y = app->windowHeight() * .5f;
        auto center_x = app->windowWidth() * .5f;
        if (mouse_detected)
        {
            auto x_offset = app->cursorPosition().x - center_x;
            auto y_offset = app->cursorPosition().y - center_y;
            if (invertXAxis())
                camera().yaw(camera().yaw() - x_offset * mouseSensitivity() * dt);
            else
                camera().yaw(camera().yaw() + x_offset * mouseSensitivity() * dt);
            if (invertYAxis())
                camera().pitch(camera().pitch() + y_offset * mouseSensitivity() * dt);
            else
                camera().pitch(camera().pitch() - y_offset * mouseSensitivity() * dt);
        }
        else
            mouse_detected = true;

        camera().zoom(app->scrollOffset().y);

        app->setCursorPosition(center_x, center_y);
    }

    Scene::update(dt);
}

void scene::ControllableCamera::resetParams()
{
    forwardSpeed(FORWARD_SPEED);
    backwardSpeed(BACKWARD_SPEED);
    sidestepSpeed(SIDESTEP_SPEED);
    runningAccel(RUNNING_ACCEL);
    turnSpeed(TURN_SPEED);
    upSpeed(UP_SPEED);
    invertXAxis(INVERT_X_AXIS);
    invertYAxis(INVERT_Y_AXIS);
    mouseSensitivity(MOUSE_SENSITIVITY);
}

void scene::ControllableCamera::resetShortcuts()
{
    forwardKey(FORWARD_KEY);
    backwardKey(BACKWARD_KEY);
    sidestepLeftKey(SIDESTEP_LEFT_KEY);
    sidestepRightKey(SIDESTEP_RIGHT_KEY);
    turnLeftKey(TURN_LEFT_KEY);
    turnRightKey(TURN_RIGHT_KEY);
    upKey(UP_KEY);
    runModifierKey(RUN_MODIFIER_KEY);
    mouseControl(MOUSE_CONTROL);
}

void scene::ControllableCamera::forwardKey(App::Key key)
{
    forward_key_ = key;
}

void scene::ControllableCamera:: backwardKey(App::Key key)
{
    backward_key_ = key;
}
void scene::ControllableCamera:: sidestepLeftKey(App::Key key)
{
    sidestep_left_key_ = key;
}
void scene::ControllableCamera:: sidestepRightKey(App::Key key)
{
    sidestep_right_key_ = key;
}
void scene::ControllableCamera:: turnLeftKey(App::Key key)
{
    turn_left_key_ = key;
}
void scene::ControllableCamera:: turnRightKey(App::Key key)
{
    turn_right_key_ = key;
}
void scene::ControllableCamera:: upKey(App::Key key)
{
    up_key_ = key;
}
void scene::ControllableCamera:: runModifierKey(App::Key key)
{
    run_modifier_key_ = key;
}

void scene::ControllableCamera::forwardSpeed(float sp)
{
    forward_speed_ = sp;
}

void scene::ControllableCamera::backwardSpeed(float sp)
{
    backward_speed_ = sp;
}

void scene::ControllableCamera::sidestepSpeed(float sp)
{
    sidestep_speed_ = sp;
}

void scene::ControllableCamera::runningAccel(float acc)
{
    running_accel_ = acc;
}

void scene::ControllableCamera::turnSpeed(float sp)
{
    turn_speed_ = sp;
}

void scene::ControllableCamera::upSpeed(float sp)
{
    up_speed_ = sp;
}

void scene::ControllableCamera::mouseControl(bool enable)
{
    mouse_control_ = enable;
}

void scene::ControllableCamera::invertYAxis(bool enable)
{
    invert_y_axis_ = enable;
}

void scene::ControllableCamera::invertXAxis(bool enable)
{
    invert_x_axis_ = enable;
}

void scene::ControllableCamera::mouseSensitivity(float sen)
{
    mouse_sensitivity_ = sen;
}
