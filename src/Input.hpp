#pragma once

#include <array>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class Input {
public:
    Input(GLFWwindow * window) {
        std::fill(m_keys.begin(), m_keys.end(), false);
        std::fill(m_buttons.begin(), m_buttons.end(), false);
        m_cursor_position.x = 0.0;
        m_cursor_position.y = 0.0;
        m_last_cursor_position.x = 0.0;
        m_last_cursor_position.y = 0.0;
        m_scroll_movement.x = 0.0;
        m_scroll_movement.y = 0.0;
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, Input::keyCallback);
        glfwSetCursorPosCallback(window, Input::cursorPosCallback);
        glfwSetMouseButtonCallback(window, Input::mouseButtonCallback);
        glfwSetScrollCallback(window, Input::scrollCallback);
    }

    glm::dvec2 getPointerMovement() {
        const glm::dvec2 movement{ m_cursor_position - m_last_cursor_position };
        m_last_cursor_position = m_cursor_position;
        return movement;
    }

    glm::dvec2 getScrollMovement() {
        const glm::dvec2 movement{ m_scroll_movement };
        m_scroll_movement = glm::dvec2{ 0.0, 0.0 };
        return movement;
    }

    bool getKey(int key) const {
        if (key < 0 || key >= KEY_COUNT) return false;
        return m_keys[key];
    }

    bool getButton(int button) const {
        if (button < 0 || button >= BUTTON_COUNT) return false;
        return m_buttons[button];
    }

private:
    static constexpr size_t KEY_COUNT{ 350 };
    static constexpr size_t BUTTON_COUNT{ 8 };
    std::array<bool, KEY_COUNT> m_keys;
    std::array<bool, BUTTON_COUNT> m_buttons;
    glm::dvec2 m_cursor_position, m_last_cursor_position;
    glm::dvec2 m_scroll_movement;

    static void cursorPosCallback(GLFWwindow * window, double x_pos, double y_pos) {
        Input * const i = static_cast<Input *>(glfwGetWindowUserPointer(window));
        i->m_cursor_position.x = x_pos;
        i->m_cursor_position.y = y_pos;
    }

    static void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods) {
        Input * const i = static_cast<Input *>(glfwGetWindowUserPointer(window));

        if (key < 0 || key >= KEY_COUNT) return;

        if (action == GLFW_PRESS)
            i->m_keys[key] = true;
        else if (action == GLFW_RELEASE)
            i->m_keys[key] = false;

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods) {
        Input * const i = static_cast<Input *>(glfwGetWindowUserPointer(window));
        if (button < 0 || button >= BUTTON_COUNT) return;
        if (action == GLFW_PRESS)
            i->m_buttons[button] = true;
        else if (action == GLFW_RELEASE)
            i->m_buttons[button] = false;
    }

    static void scrollCallback(GLFWwindow * window, double x_offset, double y_offset) {
        Input * const i = static_cast<Input *>(glfwGetWindowUserPointer(window));
        i->m_scroll_movement += glm::dvec2{ x_offset, y_offset };
    }

};
