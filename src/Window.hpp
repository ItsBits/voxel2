#pragma once

#include <string>
#include "../gl3w/gl3w.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "Input.hpp"

class Window {
public:
    struct Hints {
        int gl_major, gl_minor;
        int aa_samples;
        GLFWmonitor * monitor;
        std::string name;
        float r, g, b, a;
        bool v_sync;
        int width, height;
    };

    Window(const Window::Hints & hints) {
        // Initialize GLFW
        if (glfwInit() != GL_TRUE) throw std::runtime_error("Failed to initialize GLFW.");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, hints.gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, hints.gl_minor);
        if (hints.aa_samples > 0) glfwWindowHint(GLFW_SAMPLES, hints.aa_samples);

        // Create window
        m_window = glfwCreateWindow(hints.width, hints.height, hints.name.c_str(), hints.monitor, nullptr);
        if (m_window == nullptr) throw std::runtime_error("Failed to create window.");
        glfwMakeContextCurrent(m_window);

        glfwSwapInterval(hints.v_sync ? 1 : 0);

        // Initialize GL3W
        if (gl3wInit() != 0) throw std::runtime_error("Failed to initialize GL3W.");
        if (gl3wIsSupported(hints.gl_major, hints.gl_minor) != 1) throw std::runtime_error("OpenGL " + std::to_string(hints.gl_major) + "." + std::to_string(hints.gl_minor) + " is not supported.");

        // Init mouse
        unlockMouse();

        // OpenGL settings
        glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        glClearColor(hints.r, hints.g, hints.b, hints.a);
    }

    ~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void makeContextCurrent() {
        glfwMakeContextCurrent(m_window);
    }

    void toggleMouse() {
        if (m_mouse_locked) unlockMouse();
        else lockMouse();
    }

    void lockMouse() {
        m_mouse_locked = true;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void unlockMouse() {
        m_mouse_locked = false;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    double aspectRatio() const {
        return static_cast<double>(fb_width) / static_cast<double>(fb_height);
    }

    void swapResizeClearBuffer() {
        glfwSwapBuffers(m_window);
        glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    bool exitRequested() {
        return glfwWindowShouldClose(m_window) != 0;
    }

    GLFWwindow * getWindowPtr() {
        return m_window;
    }

private:
    GLFWwindow * m_window;
    int fb_width, fb_height;
    bool m_mouse_locked;

};
