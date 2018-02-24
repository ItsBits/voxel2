#pragma once

#include <glm/vec3.hpp>

template<typename T>
class Player
{
public:
    Player();
    T getYaw() const { return m_yaw; }
    T getPitch() const { return m_pitch; }
    const glm::tvec3<T> & getPosition() const { return m_position; }
    const glm::tvec3<T> & getFacing() const { return m_facing; }
    void update(T dt, const std::array<bool, 6> & keys, glm::tvec2<T> mouse_movement);

private:
    glm::tvec3<T> m_position;
    T m_yaw;
    T m_pitch;
    glm::tvec3<T> m_facing;
    glm::tvec3<T> m_front;
    glm::tvec3<T> m_right;
    glm::tvec3<T> m_up;

    // because static constexpr is too much hassle
    const T PI{ 3.14159 };
    const T MAX_PITCH{ 3.14159 / 2.0 - 0.001 };
    const T INVERSE_MOUSE_SENSITIVITY{ 300.0 };
    const T SPEED{ 20.0 };
};

//==============================================================================
template<typename T>
Player<T>::Player() :
    m_position{ T{ 0 }, T{ 0 }, T{ 0 } },
    m_yaw{ 1.57 },
    m_pitch{ 0 },
    m_up{ T{ 0 }, T{ 1 }, T{ 0 } }
{
    std::array<bool, 6> dummy{ { false, false, false, false, false, false } };
    update(T{ 0 }, dummy, glm::tvec2<T>{ T{ 0 }, T{ 0 } });
}

//==============================================================================
template<typename T>
void Player<T>::update(T dt, const std::array<bool, 6> & keys, glm::tvec2<T> mouse_movement) {
    // keys: forwards, backwards, right, left, up, down
    glm::tvec3<T> velocity{ T{ 0 }, T{ 0 }, T{ 0 } };
    if (keys[0] != keys[1])
        if (keys[0]) velocity += m_front;
        else velocity -= m_front;
    if (keys[2] != keys[3])
        if (keys[2]) velocity += m_right;
        else velocity -= m_right;
    if (keys[4] != keys[5])
        if (keys[4]) velocity += m_up;
        else velocity -= m_up;
    
    m_position += velocity * dt * SPEED;

    mouse_movement.y = -mouse_movement.y;
    mouse_movement /= INVERSE_MOUSE_SENSITIVITY;
    m_yaw += mouse_movement.x;
    m_pitch += mouse_movement.y;
    m_pitch = std::min(std::max(m_pitch, -Player<T>::MAX_PITCH), Player<T>::MAX_PITCH);
    m_yaw = std::fmod(m_yaw, T{ 2 } * PI);

    const T cp = std::cos(m_pitch);
    const T sp = std::sin(m_pitch);
    const T cy = std::cos(m_yaw);
    const T sy = std::sin(m_yaw);
    m_facing = glm::normalize(glm::tvec3<T>{ cp * cy, sp, cp * sy });
    m_right = glm::normalize(glm::cross(m_facing, m_up));
    m_front = glm::normalize(glm::tvec3<T>{ m_facing.x, T{ 0 }, m_facing.z });
}
