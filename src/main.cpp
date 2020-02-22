#include <optional>
#include <cstdio>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include <stb_image.h>

#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include "glm.h"
#include "camera.h"
#include "defer.h"
#include "resource.h"
#include "surface.h"
#include "object.h"
#include "renderer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct RenderCtx;
void ProvideCamera(RenderCtx *ctx);

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

struct RenderCtx {
    std::optional<Camera> camera;

    std::unordered_map<UuidRef, Object> objects;

    InputBuffer input_buf;
    Controller controller;

    ModelParams model_params;
    CameraParams camera_params;
    std::optional<float> recompile_timeout;

    bool running;
    float time;
    float dt;

    SDL_Window *window;
    ImGuiIO *imgui_io;
};

void DrawFrame(RenderCtx *ctx)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Camera &cam = ctx->camera.value();

    for (auto it = ctx->objects.begin(); it != ctx->objects.end(); it++) {
        Object &object = it->second;
        auto renderer = object.component<Renderer>();
        if (renderer) {
            renderer->get().draw(&object, &cam, ctx->time);
        }
    }
}

void Update(RenderCtx *ctx, float dt)
{
    glm::vec3 &movement = ctx->controller.movement;
    glm::vec2 &look = ctx->controller.look;

    std::optional<Input> input;
    while ((input = ctx->input_buf.pop())) {
        switch (input->type) {
        case InputType::Reset:
            movement *= 0;
            look *= 0;
            break;
        case InputType::Key: {
            int digital_press_vector = input->digital.press - !input->digital.press;
        
            switch (input->digital.source) {
            case SDL_SCANCODE_W:            movement.z -= digital_press_vector; break;
            case SDL_SCANCODE_S:            movement.z += digital_press_vector; break;
            case SDL_SCANCODE_A:            movement.x -= digital_press_vector; break;
            case SDL_SCANCODE_D:            movement.x += digital_press_vector; break;
            case SDL_SCANCODE_SPACE:        movement.y += digital_press_vector; break;
            case SDL_SCANCODE_LSHIFT:       movement.y -= digital_press_vector; break;
            case SDL_SCANCODE_UP:           look.y += digital_press_vector; break;
            case SDL_SCANCODE_DOWN:         look.y -= digital_press_vector; break;
            case SDL_SCANCODE_LEFT:         look.x -= digital_press_vector; break;
            case SDL_SCANCODE_RIGHT:        look.x += digital_press_vector; break;
            }
            break;
        }
        default:
            break;
        }
    }

    //Model &model = ctx->verts.value();
    // // want to rotate pi rad/sec
    //model.xform = glm::rotate(model.xform, dt * (float)M_PI, glm::vec3(0.f, 1.f, 0.f));

    Camera &camera = ctx->camera.value();
    CameraParams &cparams = ctx->camera_params;
    float mv_speed = cparams.move_speed;
    glm::vec2 look_speed = cparams.look_speed;

    camera.look += dt * look_speed * look;
    glm::vec3 look_movement = glm::rotate(movement, -camera.look.x, glm::vec3(0, 1, 0));
    camera.pos += dt * mv_speed * look_movement;
    if (camera.look.y > M_PI/2)
        camera.look.y = M_PI/2;
    if (camera.look.y < -M_PI/2)
        camera.look.y = -M_PI/2;
    camera.xform_dirty = true;


    for (auto it = ctx->objects.begin(); it != ctx->objects.end(); it++) {
        Object &object = it->second;
        object.update(ctx, dt);
    }
}

void DrawObjectUi(RenderCtx *ctx)
{
    Camera &camera = ctx->camera.value();
    CameraParams &cparams = ctx->camera_params;

    bool cam_diff = false;

    bool cparam_open = true;
    ImGui::Begin("Camera Params", &cparam_open, ImGuiWindowFlags_AlwaysAutoResize);
    {
        cam_diff |= ImGui::InputFloat3("Position", &camera.pos[0]);
        cam_diff |= ImGui::InputFloat2("Direction (rad)", &camera.look[0]);

        float fov_degrees = cparams.fov * 180 / M_PI;
        cam_diff |= ImGui::InputFloat("Field of View", &fov_degrees);
        cparams.fov = fov_degrees * M_PI / 180;

        float clip_range[2] = { cparams.near, cparams.far };
        cam_diff |= ImGui::InputFloat2("Clipping Range", clip_range);
        cparams.near = clip_range[0];
        cparams.far = clip_range[1];

        cam_diff |= ImGui::InputFloat3("Movement Speed", &cparams.move_speed);
        cam_diff |= ImGui::InputFloat2("Look Speed (rad/s)", &cparams.look_speed[0]);
    }
    ImGui::End();

    if (cam_diff) {
        printf("Refreshing camera...\n");
        camera.set_params(cparams);
    }
}

void ProvideCamera(RenderCtx *ctx)
{
    Camera camera(ctx->camera_params);
    camera.pos = glm::vec3(0.f, 5.f, 10.f);
    camera.look = glm::vec2(0.f, (float)M_PI / -8.f);
    ctx->camera = camera;
}

void AddObject(RenderCtx *ctx, Object object)
{
    ctx->objects[object.uuid] = std::move(object);
}

void QuitLoop(RenderCtx *ctx)
{
#ifdef __EMSCRIPTEN__
    (void)ctx;
    emscripten_cancel_main_loop();
#else
    ctx->running = false;
#endif
}

void MainLoop(RenderCtx *ctx)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        ImGui_ImplSDL2_ProcessEvent(&ev);

        switch (ev.type) {
        case SDL_QUIT:
            QuitLoop(ctx);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (ctx->imgui_io->WantCaptureKeyboard)
                break;
            if (ev.key.repeat)
                break;

            Input input;
            input.type = InputType::Key;
            input.digital = {
                ev.key.keysym.scancode,
                ev.key.state == SDL_PRESSED,
            };
            ctx->input_buf.push(input);
            break;
        }
        case SDL_WINDOWEVENT: {
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_FOCUS_LOST: {
                Input input;
                input.type = InputType::Reset;
                ctx->input_buf.push(input);
                break;
            }
            default: {
                int window_w, window_h;
                SDL_GetWindowSize(ctx->window, &window_w, &window_h);
                glViewport(0, 0, window_w, window_h);
                ctx->camera_params.aspect = (float)window_w / (float)window_h;

                Camera &cam = ctx->camera.value();
                cam.set_params(ctx->camera_params);
                break;
            }
            }
        }

        default: break;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(ctx->window);
    ImGui::NewFrame();

    Update(ctx, ctx->dt);

    DrawObjectUi(ctx);
    ImGui::Render();
    
    DrawFrame(ctx);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(ctx->window);

    float now = SDL_GetTicks() / 1000.f;
    ctx->dt = now - ctx->time;
    ctx->time = now;
}


int main(int argc, char **argv)
{
#define CHECK_RET(ret, err) do { \
        if (ret) { \
            printf(err " Retcode=%d\n", ret); \
            return ret; \
        } \
    } while (0)

    (void)argc;
    (void)argv;

    int ret;

    stbi_set_flip_vertically_on_load(true);

    int subsystems = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    ret = SDL_Init(subsystems);
    CHECK_RET(ret, "SDL Init failed!");
    DEFER({ SDL_Quit(); });

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    int win_w = 1600, win_h = 900;
    int win_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    const char *win_title = "3yee | Parametric Equation Viewer";
    SDL_Window *window = SDL_CreateWindow(win_title, 0, 0, win_w, win_h, win_flags);
    CHECK_RET(!window, "SDL create Window failed!");
    DEFER({ SDL_DestroyWindow(window); });

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    CHECK_RET(!glcontext, "Failed to init GL context!");
    DEFER({ SDL_GL_DeleteContext(glcontext); });

    ret = SDL_GL_MakeCurrent(window, glcontext);
    CHECK_RET(ret, "Failed to make the GL context current!");

    glewExperimental = GL_TRUE;
    ret = glewInit();
    CHECK_RET(ret != GLEW_OK, "Failed to initialize GLEW!");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    DEFER({ ImGui::DestroyContext(); });

    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
    DEFER({ ImGui_ImplSDL2_Shutdown(); });

    ImGui_ImplOpenGL3_Init("#version 300 es");
    DEFER({ ImGui_ImplOpenGL3_Shutdown(); });

    RenderCtx render_ctx;
    AddObject(&render_ctx, CreateSurface());
    ProvideCamera(&render_ctx);

    glEnable(GL_DEPTH_TEST);

    render_ctx.running = true;

    render_ctx.time = SDL_GetTicks() / 1000.f;
    render_ctx.dt = 0.f;

    render_ctx.window = window;
    render_ctx.imgui_io = &io;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg((void(*)(void *))MainLoop, &render_ctx, 0, 1);
#else
    while (render_ctx.running) {
        MainLoop(&render_ctx);
    }
#endif
}
