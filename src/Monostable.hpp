#pragma once

#include <cstdint>

class Monostable
{
public:
    Monostable() {}

    bool state() {
        if (m_state == 1) {
            m_state = 2;
            return true;
        }
        else
            return false;
    }

    void update(bool input) {
        if (input) {
            if (m_state == 0)
                m_state = 1;
        }
        else
            m_state = 0;
    }

private:
    uint8_t m_state{ 0 };

};