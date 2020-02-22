#include "camera.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "game.h"

void CameraEditor::update(RenderCtx *ctx, Object *obj, float dt)
{
    (void)ctx; (void)dt;

    Camera &camera = obj->component<Camera>().value();
    bool cam_diff = false;

    bool cparam_open = true;
    ImGui::Begin("Camera Params", &cparam_open, ImGuiWindowFlags_AlwaysAutoResize);
    {
        cam_diff |= ImGui::InputFloat3("Position", &camera.pos[0]);
        cam_diff |= ImGui::InputFloat2("Direction (rad)", &camera.look[0]);

        float fov_degrees = camera_params.fov * 180 / M_PI;
        cam_diff |= ImGui::InputFloat("Field of View", &fov_degrees);
        camera_params.fov = fov_degrees * M_PI / 180;

        float clip_range[2] = { camera_params.near, camera_params.far };
        cam_diff |= ImGui::InputFloat2("Clipping Range", clip_range);
        camera_params.near = clip_range[0];
        camera_params.far = clip_range[1];

        cam_diff |= ImGui::InputFloat3("Movement Speed", &camera_params.move_speed);
        cam_diff |= ImGui::InputFloat2("Look Speed (rad/s)", &camera_params.look_speed[0]);
    }
    ImGui::End();

    if (cam_diff) {
        printf("Refreshing camera...\n");
        camera.set_params(camera_params);
    }
}


const glm::mat4 &Camera::xform()
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

void Camera::set_params(CameraParams params)
{
    this->projection = glm::perspective(params.fov / 2, params.aspect, params.near, params.far);        
}

void Camera::set_xform(glm::vec3 pos, glm::vec3 look)
{
    this->pos = pos;
    this->look = look;
    this->xform_dirty = true;
}

void Camera::update(RenderCtx *ctx, Object *obj, float dt)
{
    CameraEditor &editor = obj->component<CameraEditor>().value();

    float mv_speed = editor.camera_params.move_speed;
    glm::vec2 look_speed = editor.camera_params.look_speed;

    this->look += dt * look_speed * ctx->controller.look;
    glm::vec3 look_movement = glm::rotate(ctx->controller.movement, -this->look.x, glm::vec3(0, 1, 0));
    this->pos += dt * mv_speed * look_movement;
    if (this->look.y > M_PI/2)
        this->look.y = M_PI/2;
    if (this->look.y < -M_PI/2)
        this->look.y = -M_PI/2;
    this->xform_dirty = true;
}

