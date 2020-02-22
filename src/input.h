#pragma once

#include <deque>

#include "glm.h"

enum class InputType {
    Reset,
    Key,
    Analog1d,
    Analog2d,
};
struct Input {
    InputType type;

    struct Digital {
        int source;
        bool press;
    };
    struct Analog1d {
        int source;
        float vel;
    };
    struct Analog2d {
        int source;
        glm::vec2 vel;
    };

    union {
        Input::Digital digital;
        Input::Analog1d analog1d;
        Input::Analog2d analog2d;
    };
};

struct InputBuffer {
    std::deque<Input> buf;

    void push(Input input)
    {
        buf.push_back(input);
    }

    std::optional<Input> pop()
    {
        if (buf.empty())
            return {};
        Input out = buf[0];
        buf.pop_front();
        return out;
    }
};

struct Controller {
    glm::vec3 movement = glm::zero<glm::vec3>();
    glm::vec2 look = glm::zero<glm::vec2>();
};
