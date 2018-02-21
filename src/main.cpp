#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "../gl3w/gl3w.h"

#include "Window.hpp"
#include "Input.hpp"
#include "VoxelMap.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VoxelScene.hpp"
#include "Shader.hpp"
#include "Monostable.hpp"
#include "VoxelArray.hpp"
#include "VoxelContainer.hpp"
#include "LockedQueue.hpp"

#include "cfg.hpp"
#include "Print.hpp"

int main() {
    std::unique_ptr<VoxelContainer> vc = std::make_unique<VoxelContainer>();
    LockedQueue<Mesh> & q = vc->getQueue();

    //std::system("rm world/*");
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
    glEnable(GL_CULL_FACE);

    Monostable q_button;
    VoxelMap voxel_storage{};
    Camera<float> camera{};
    Player<float> player;
    Input input{ window.getWindowPtr() };
    VoxelScene scene{};
    Shader scene_shader{
        {
            { "shader/block.vert", GL_VERTEX_SHADER },
            { "shader/block.frag", GL_FRAGMENT_SHADER }
        }
    };

    GLint offset_uniform = glGetUniformLocation(scene_shader.id(), "offset");
    GLint VP_uniform = glGetUniformLocation(scene_shader.id(), "VP_matrix");

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

        // (un)lock mouse
        q_button.update(input.getKey(GLFW_KEY_Q));
        if (q_button.state()) window.toggleMouse();

        scene_shader.use();
        // TODO: more precise calculation (correct rounding)
        glm::ivec3 center = (player.getPosition() - glm::vec3(cfg::MESH_OFFSET)) / glm::vec3{ cfg::MESH_SIZE };
        vc->moveCenterChunk(center);
        scene.update(center, q);
        const auto VP_matrix = camera.getViewProjectionMatrix();
        const auto frustum_planes = Math::matrixToNormalizedFrustumPlanes(VP_matrix);
        glUniformMatrix4fv(VP_uniform, 1, GL_FALSE, glm::value_ptr(VP_matrix));
        scene.draw(offset_uniform, frustum_planes);
        window.swapResizeClearBuffer();
    }

    window.unlockMouse();
    window.swapResizeClearBuffer();
}
