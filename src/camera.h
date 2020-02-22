#pragma once

#include "glm.h"

struct CameraParams {
    float fov = M_PI / 2;
    float aspect = 16.0/9.0;
    float near = 0.1;
    float far = 1000;

    float move_speed = 10.f;
    glm::vec2 look_speed = glm::vec2(1.f, 0.5f);
};

struct Camera {
    glm::mat4 projection;
    glm::mat4 cached_xform;
    bool xform_dirty = true;

    glm::vec3 pos = glm::zero<glm::vec3>();
    glm::vec2 look = glm::zero<glm::vec2>();

    Camera(CameraParams params)
    {
        set_params(params);
    }

    const glm::mat4 &xform()
    {
        if (xform_dirty) {
            cached_xform = glm::identity<glm::mat4>();
            cached_xform = glm::translate(cached_xform, pos);
            cached_xform = glm::rotate(cached_xform, -look.x, glm::vec3(0, 1, 0));
            cached_xform = glm::rotate(cached_xform, look.y, glm::vec3(1, 0, 0));
            cached_xform = glm::inverse(cached_xform);
            xform_dirty = false;
        }
        return cached_xform;
    }

    void set_params(CameraParams params)
    {
        this->projection = glm::perspective(params.fov / 2, params.aspect, params.near, params.far);        
    }
};
