#include "renderer.h"

#include <GL/glew.h>

VertArrayObj::VertArrayObj()
{
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
}

VertArrayObj::~VertArrayObj()
{
    if (!valid)
        return;
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}



void Renderer::draw(Object *obj, Camera *camera, float time)
{
#define VA_OFFSETOF(type, mem) ( &((type *)NULL)->mem )

#define VTX_POS_ARG 0
#define VTX_TEXPOS_ARG 1

    glUseProgram(shader.id);

    GLint u_time = glGetUniformLocation(shader.id, "u_time");
    // GLint u_texture = glGetUniformLocation(shader, "u_texture");
    GLint u_model = glGetUniformLocation(shader.id, "u_model");
    GLint u_view = glGetUniformLocation(shader.id, "u_view");
    GLint u_proj = glGetUniformLocation(shader.id, "u_proj");
    glUniform1f(u_time, time);

    Mesh &mesh = obj->component<Mesh>().value();
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(mesh.xform));
    auto &vertices = mesh.vertices;
    auto &indices = mesh.indices;
    auto &vao = mesh.vao;

    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(camera->xform()));
    glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(camera->projection));

    glBindVertexArray(vao.vao);
    if (vao.dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, vao.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(VIndices), &indices[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(VTX_POS_ARG, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), VA_OFFSETOF(Vertex, x));
        glEnableVertexAttribArray(VTX_POS_ARG);

        glVertexAttribPointer(VTX_TEXPOS_ARG, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), VA_OFFSETOF(Vertex, tex_u));
        glEnableVertexAttribArray(VTX_TEXPOS_ARG);

        vao.dirty = false;
    }

    glDrawElements(GL_TRIANGLES, indices.size() * sizeof(VIndices) / sizeof(float), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}