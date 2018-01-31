#pragma once

#include <string>

struct GLFWwindow;
struct GLFWmonitor;

class Window
{
public:
    struct Hints
    {
        int gl_major, gl_minor;
        int aa_samples;
        GLFWmonitor * monitor;
        std::string name;
        float r, g, b, a;
        bool v_sync;
        int width, height;
    };

    Window(const Window::Hints & hints);
    ~Window();

    void makeContextCurrent();

    void toggleMouse();
    void lockMouse();
    void unlockMouse();

    double aspectRatio() const;

    void swapResizeClearBuffer();

    bool exitRequested();
    GLFWwindow * getWindowPtr() { return m_window; }

private:
    GLFWwindow * m_window;
    int fb_width, fb_height;
    bool m_mouse_locked;

};
