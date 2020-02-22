#pragma once

#include <vector>

#include "glm.h"
#include "camera.h"
#include "object.h"
#include "shader.h"


#pragma pack(push, 1)
struct Vertex {
    float x, y, z;
    float tex_u, tex_v;
};
struct VIndices {
    unsigned first, second, third;
};
#pragma pack(pop)

struct VertArrayObj {
    unsigned vbo, ebo, vao;
    bool dirty = true;

    RESOURCE_IMPL(VertArrayObj);

    VertArrayObj();
    ~VertArrayObj();
};


struct Mesh: public Component {
    VertArrayObj vao;

    glm::mat4 xform = glm::identity<glm::mat4>();

    std::vector<Vertex> vertices;
    std::vector<VIndices> indices;

    Mesh(std::vector<Vertex> vertices, std::vector<VIndices> indices):
        vertices(vertices), indices(indices)
    {
    }

    void edit(std::vector<Vertex> vertices, std::vector<VIndices> indices)
    {
        this->vertices = vertices;
        this->indices = indices;
        vao.dirty = true;
    }
};

struct Renderer: Component {
    ShaderProgram shader;

    Renderer(ShaderProgram shader):
        shader(std::move(shader))
    {
    }

    void draw(Object *obj, Camera *camera, float time);
};