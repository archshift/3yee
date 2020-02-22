#pragma once

#include <optional>
#include <string>

#include "object.h"

struct Equations {
    std::string x = "u";
    std::string y = "-5.0 * sin(t) * exp(-abs(u) - abs(v))";
    std::string z = "v";
};

struct ModelParams {
    unsigned res_x = 1000, res_y = 1000;
    float x_min = -3, x_max = 3, y_min = -3, y_max = 3;
};

struct ShaderProgram;
struct Mesh;

struct SurfaceEditor : Component {
    size_t eq_num;

    Equations eqs;
    ModelParams model_params;
    float recompile_timeout;

    SurfaceEditor(size_t eq_num):
        eq_num(eq_num)
    {
    }

    void update(RenderCtx *ctx, Object *obj, float dt);

    Mesh create_mesh();
    std::optional<ShaderProgram> create_shader();
};

Object CreateSurface();