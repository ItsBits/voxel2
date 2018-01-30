#include <iostream>
#include <thread>
#include <chrono>

#include "gl3w.h"

#include "Window.hpp"
#include "Input.hpp"
#include "VoxelStorage.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include <glm/gtx/string_cast.hpp>

int main() {
    Window::Hints window_hints;
    window_hints.gl_major = 3;
    window_hints.gl_minor = 1;
    window_hints.aa_samples = 0;
    window_hints.monitor = nullptr;
    window_hints.name = "Voxel";
    window_hints.r = 0.9f;
    window_hints.g = 0.9f;
    window_hints.b = 0.6f;
    window_hints.a = 1.0f;
    window_hints.v_sync = true;
    window_hints.width = 960;
    window_hints.height = 540;

    Window window{ window_hints };

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL); // sky box
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    VoxelStorage voxel_storage{};
    Camera<float> camera{};
    Player<float> player;
    Input input{ window.getWindowPtr() };

    auto last_loop = std::chrono::high_resolution_clock::now();
    while (!window.exitRequested()) {
        const auto loop_start = std::chrono::high_resolution_clock::now();
        const double dt = std::chrono::duration_cast<std::chrono::duration<double>>(loop_start - last_loop).count();
        last_loop = loop_start;
        glfwPollEvents();
        const auto mouse_pointer_movement = input.getPointerMovement();
        const auto scroll_movement = input.getScrollMovement();
        const std::array<bool, 6> keys{
            input.getKey(GLFW_KEY_U), input.getKey(GLFW_KEY_J), input.getKey(GLFW_KEY_K),
            input.getKey(GLFW_KEY_H), input.getKey(GLFW_KEY_L), input.getKey(GLFW_KEY_SPACE)
        };
        player.update(dt, keys, mouse_pointer_movement);
        camera.updateAspectRatio(window.aspectRatio());
        camera.update(player.getPosition(), player.getYaw(), player.getPitch());
        //std::cout << 1.0 / dt << std::endl;
        //std::cout << glm::to_string(player.getPosition()) << std::endl;
        //std::cout << glm::to_string(mouse_pointer_movement) << std::endl;
        //std::cout << player.getPitch() << '|' << player.getYaw() << std::endl;
        //std::cout << glm::to_string(input.getScrollMovement()) << std::endl;
        window.swapResizeClearBuffer();
    }

    window.unlockMouse();
    window.swapResizeClearBuffer();
}
