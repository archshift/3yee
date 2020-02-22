#pragma once

#include <optional>
#include <unordered_map>

#include "input.h"
#include "object.h"

struct SDL_Window;
struct ImGuiIO;

struct GameState {
    std::optional<UuidRef> main_camera;

    std::unordered_map<UuidRef, Object> objects;

    InputBuffer input_buf;
    Controller controller;

    bool running;
    float time;
    float dt;

    SDL_Window *window;
    ImGuiIO *imgui_io;
};
