#pragma once

#include "glm.h"
#include "object.h"

struct CameraParams {
    float fov = M_PI / 2;
    float aspect = 16.0/9.0;
    float near = 0.1;
    float far = 1000;

    float move_speed = 10.f;
    glm::vec2 look_speed = glm::vec2(1.f, 0.5f);
};


struct CameraEditor : Component {
    CameraParams camera_params;

    void update(GameState *ctx, Object *obj, float dt);
};


struct Camera : Component {
    glm::mat4 projection;
    glm::mat4 cached_xform;
    bool xform_dirty = true;

    glm::vec3 pos = glm::zero<glm::vec3>();
    glm::vec2 look = glm::zero<glm::vec2>();

    Camera(CameraParams params)
    {
        set_params(params);
    }

    const glm::mat4 &xform();
    void set_params(CameraParams params);
    void set_xform(glm::vec3 pos, glm::vec3 look);

    void update(GameState *ctx, Object *obj, float dt);
};


inline Object CreateCamera()
{
    Object object;

    CameraEditor camera_editor;
    Camera camera(camera_editor.camera_params);
    camera.pos = glm::vec3(7.f, 5.f, 7.f);
    camera.look = glm::vec2((float)M_PI / -4.f, (float)M_PI / -8.f);

    object.add_component(std::move(camera_editor));
    object.add_component(std::move(camera));

    return object;
}