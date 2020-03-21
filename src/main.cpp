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

#include "axes.h"
#include "glm.h"
#include "game.h"
#include "camera.h"
#include "defer.h"
#include "resource.h"
#include "surface.h"
#include "object.h"
#include "renderer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


void DrawFrame(GameState *ctx)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Object &cam_obj = ctx->objects.at(ctx->main_camera.value());
    Camera &cam = cam_obj.component<Camera>().value();

    for (auto it = ctx->objects.begin(); it != ctx->objects.end(); it++) {
        Object &object = it->second;
        auto renderer = object.component<Renderer>();
        if (renderer) {
            renderer->get().draw(&object, &cam, ctx->time);
        }
    }
}

void Update(GameState *ctx, float dt)
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

    for (auto it = ctx->objects.begin(); it != ctx->objects.end();) {
        Object &object = it->second;
        if (object.deleted) {
            it = ctx->objects.erase(it);
            continue;
        }

        object.update(ctx, dt);
        it++;
    }
}

void QuitLoop(GameState *ctx)
{
#ifdef __EMSCRIPTEN__
    (void)ctx;
    emscripten_cancel_main_loop();
#else
    ctx->running = false;
#endif
}

void MainLoop(GameState *ctx)
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

                Object &cam_obj = ctx->objects.at(ctx->main_camera.value());
                CameraEditor &cam_editor = cam_obj.component<CameraEditor>().value();
                CameraParams &cparams = cam_editor.camera_params;
                Camera &cam = cam_obj.component<Camera>().value();

                cparams.aspect = (float)window_w / (float)window_h;
                cam.set_params(cparams);
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

    ImGui::Begin("Configuration");
    Update(ctx, ctx->dt);

    if (ImGui::Button("Add Surface")) {
        ctx->add_object(CreateSurface());
    }
    ImGui::End();

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

    GameState game_state;
    game_state.add_main_camera(CreateCamera());
    game_state.add_object(CreateSurface());
    game_state.add_object(CreateAxes());

    glEnable(GL_DEPTH_TEST);

    game_state.running = true;

    game_state.time = SDL_GetTicks() / 1000.f;
    game_state.dt = 0.f;

    game_state.window = window;
    game_state.imgui_io = &io;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg((void(*)(void *))MainLoop, &game_state, 0, 1);
#else
    while (game_state.running) {
        MainLoop(&game_state);
    }
#endif
}
