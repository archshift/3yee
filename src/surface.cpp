#include "surface.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <GL/glew.h>

#include "renderer.h"

void SurfaceEditor::update(RenderCtx *ctx, Object *obj, float dt)
{
    (void)ctx;
    bool model_diff = false;
    bool eq_diff = false;

    bool mparam_open = true;
    ImGui::Begin("Model Params", &mparam_open, ImGuiWindowFlags_AlwaysAutoResize);
    {
        eq_diff |= ImGui::InputText("x=", &eqs.x);
        eq_diff |= ImGui::InputText("y=", &eqs.y);
        eq_diff |= ImGui::InputText("z=", &eqs.z);

        int res[2] = { (int)model_params.res_x, (int)model_params.res_y };
        model_diff |= ImGui::InputInt2("Num Triangles (u, v)", res);
        model_params.res_x = res[0];
        model_params.res_y = res[1];

        float range_u[2] = { model_params.x_min, model_params.x_max };
        model_diff |= ImGui::InputFloat2("Range (u)", range_u);
        model_params.x_min = range_u[0];
        model_params.x_max = range_u[1];

        float range_v[2] = { model_params.y_min, model_params.y_max };
        model_diff |= ImGui::InputFloat2("Range (v)", range_v);
        model_params.y_min = range_v[0];
        model_params.y_max = range_v[1];
    }
    ImGui::End();

    if (eq_diff) {
        recompile_timeout = 0.5;
    }
    if (recompile_timeout) {
        recompile_timeout -= dt;

        if (recompile_timeout <= 0) {
            recompile_timeout = 0;

            printf("Refreshing equation...\n");

            Renderer &renderer = obj->component<Renderer>().value();
            auto new_shader = create_shader();
            if (new_shader) {
                renderer.shader = std::move(*new_shader);
            }
        }
    }

    if (model_diff) {
        printf("Refreshing model_params...\n");

        Mesh &mesh = obj->component<Mesh>().value();
        auto new_mesh = create_mesh();
        mesh = std::move(new_mesh);
    }
}



std::optional<ShaderProgram> SurfaceEditor::create_shader()
{
    auto vertex_xform = [=](std::string *src){
        const char *needle = "__INCLUDE_XYZ__";
        size_t findpos = src->find(needle);

        if (findpos != std::string::npos) {
            char *glsl_xyz;
            asprintf(&glsl_xyz,
                "float x = %s;\n"
                "float y = %s;\n"
                "float z = %s;\n",
                eqs.x.c_str(),
                eqs.y.c_str(),
                eqs.z.c_str()
            );
            src->replace(findpos, strlen(needle), glsl_xyz);
            free(glsl_xyz);
        }
    };
    auto sh_vertex = LoadShaderFile("vertex.glsl", GL_VERTEX_SHADER, vertex_xform);
    if (!sh_vertex)
        return {};

    auto sh_fragment = LoadShaderFile("fragment.glsl", GL_FRAGMENT_SHADER);
    if (!sh_fragment)
        return {};

    auto program = ShaderProgram::link({ *sh_vertex, *sh_fragment });
    if (!program)
        return {};

    return program;
}



Mesh SurfaceEditor::create_mesh()
{
    unsigned res_x = model_params.res_x;
    unsigned res_y = model_params.res_y;

    float x_min = model_params.x_min, x_max = model_params.x_max;
    float y_min = model_params.y_min, y_max = model_params.y_max;

    unsigned verts_x = res_x + 1;
    unsigned verts_y = res_y + 1;
    float width = x_max - x_min;
    float height = y_max - y_min;

    // NxN grid: (N+1)^2 vertices, NxNx2 triangles
    std::vector<Vertex> vertices(verts_x * verts_y);
    std::vector<VIndices> indices(res_x * res_y * 2);

    for (unsigned i = 0; i < verts_x; i++) {
        for (unsigned j = 0; j < verts_y; j++) {
            float fract_x = (float)i / (float)verts_x;
            float fract_y = (float)j / (float)verts_y;

            float pos_x = x_min + fract_x * width;
            float pos_y = y_min + fract_y * height;
            vertices[i*verts_y+j] = { pos_x, -1, pos_y, 0, 0 };
        }
    }

    for (unsigned i = 0; i < res_x; i++) {
        for (unsigned j = 0; j < res_y; j++) {
            unsigned offs = 2 * (i*res_y + j);
            indices[offs]   = { (i+0)*verts_y+(j+0), (i+1)*verts_y+(j+0), (i+0)*verts_y+(j+1) };
            indices[offs+1] = { (i+1)*verts_y+(j+1), (i+1)*verts_y+(j+0), (i+0)*verts_y+(j+1) };
        }
    }

    return { vertices, indices };
}




Object CreateSurface()
{
    Object obj;

    SurfaceEditor surface_editor;
    Mesh mesh = surface_editor.create_mesh();
    ShaderProgram shader = surface_editor.create_shader().value();
    Renderer renderer(std::move(shader));

    obj.add_component(std::move(surface_editor));
    obj.add_component(std::move(renderer));
    obj.add_component(std::move(mesh));

    return obj;
}