#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "../gl3w/gl3w.h"

#include "Window.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VoxelScene.hpp"
#include "Shader.hpp"
#include "Monostable.hpp"
#include "VoxelContainer.hpp"
#include "LockedQueue.hpp"
#include "cfg.hpp"
#include "Print.hpp"
#include "Ray.hpp"
#include "LineCube.hpp"

int main() {
    std::unique_ptr<VoxelContainer> vc = std::make_unique<VoxelContainer>();
    LockedQueue<Mesh, cfg::MESH_QUEUE_SIZE_LIMIT> & q = vc->getQueue();

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

    LineCube center_marker;

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL); // sky box
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

    Monostable q_button;
    Monostable mouse_button;
    Camera<float> camera;
    Player<double> player;
    Input input{ window.getWindowPtr() };
    VoxelScene scene;
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

        const glm::dvec3 player_position = player.getPosition();
        glm::dvec3 camera_position_d;
        glm::dvec3 camera_offset_d;
        camera_position_d.x = std::modf(player_position.x, &camera_offset_d.x);
        camera_position_d.y = std::modf(player_position.y, &camera_offset_d.y);
        camera_position_d.z = std::modf(player_position.z, &camera_offset_d.z);
        const glm::vec3 camera_position{ camera_position_d };
        const glm::ivec3 camera_offset{ camera_offset_d };

        camera.update(camera_position, player.getYaw(), player.getPitch());

        // (un)lock mouse
        q_button.update(input.getKey(GLFW_KEY_Q));
        if (q_button.state()) window.toggleMouse();
        mouse_button.update(input.getButton(0));

        const glm::dvec3 center_d = player_position / glm::dvec3{ cfg::CHUNK_SIZE };
        const glm::ivec3 center{ std::lround(center_d.x), std::lround(center_d.y), std::lround(center_d.z) };
        // move after updating chunks to reduce change of getwritable failure?
        vc->moveCenterChunk(center);
        // TODO: offset ray same as camera and use float and offset (add offset to result)
        scene.update(center, q, player_position, player.getFacing(), *vc.get(), mouse_button.state());
        const auto VP_matrix = camera.getViewProjectionMatrix();
        const auto frustum_planes = Math::matrixToNormalizedFrustumPlanes(VP_matrix);
        scene.draw_cube(VP_matrix, -camera_offset);
        camera.update(player.getPosition(), player.getYaw(), player.getPitch());
        scene_shader.use();
        glUniformMatrix4fv(VP_uniform, 1, GL_FALSE, glm::value_ptr(VP_matrix));
        scene.draw(offset_uniform, frustum_planes, -camera_offset);

        static constexpr glm::vec3 CENTER_MARKER_SIZE{ 0.02f, 0.02f, 0.02f };
        center_marker.draw({}, -CENTER_MARKER_SIZE / 2.0f, CENTER_MARKER_SIZE * glm::vec3{ 1.0f, static_cast<float>(window.aspectRatio()), 1.0f });
        window.swapResizeClearBuffer();
    }

    window.unlockMouse();
    window.swapResizeClearBuffer();
}
