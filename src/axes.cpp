#include "axes.h"

#include "shader.h"
#include "renderer.h"

#include <GL/glew.h>

static std::optional<ShaderProgram> create_shader()
{
    auto sh_vertex = LoadShaderFile("shaders/axes.vert", GL_VERTEX_SHADER);
    if (!sh_vertex)
        return {};

    auto sh_fragment = LoadShaderFile("shaders/axes.frag", GL_FRAGMENT_SHADER);
    if (!sh_fragment)
        return {};

    auto program = ShaderProgram::link({ *sh_vertex, *sh_fragment });
    if (!program)
        return {};

    return program;
}

static Mesh create_mesh()
{
    float end = 10;
    float eps = 0.01;
    int axis = 1;

    std::vector<Vertex> vertices;
    std::vector<VIndices> indices;

    for (int axis = 0; axis < 3; axis++) {

        for (int i = -1; i <= 1; i += 2) {
            for (int j = -1; j <= 1; j += 2) {
                for (int k = -1; k <= 1; k += 2) {
                    vertices.push_back({
                        i * (axis == 0 ? end : eps),
                        j * (axis == 1 ? end : eps),
                        k * (axis == 2 ? end : eps)
                    });
                }
            }
        }

        unsigned base[] = { 0b000, 0b011, 0b110, 0b101 };
        for (int i = 0; i < 4; i++) {
            unsigned b = base[i];

            for (int j = 0; j < 2; j++) {
                for (int k = j + 1; k < 3; k++) {
                    indices.push_back({
                        b + axis * 8,
                        (b ^ (1<<j)) + axis * 8,
                        (b ^ (1<<k)) + axis * 8
                    });
                }
            }
        }

    }

    return Mesh(vertices, indices);
}

Object CreateAxes()
{
    Object obj;

    Mesh mesh = create_mesh();
    ShaderProgram shader = create_shader().value();
    Renderer renderer(std::move(shader));

    obj.add_component(std::move(renderer));
    obj.add_component(std::move(mesh));

    return obj;
}