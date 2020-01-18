#include <optional>
#include <array>
#include <cstdio>
#include <cstdint>
#include <deque>
#include <unordered_set>

#include <glad/glad.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SDL.h>

#include "defer.h"
#include "resource.h"
#include "shader.h"

enum class InputType {
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

struct Texture {
    GLuint texture;

    RESOURCE_IMPL(Texture);

    static std::optional<Texture> from_path(const std::string &path)
    {
        int w, h, chan;
        uint8_t *buf = stbi_load(path.c_str(), &w, &h, &chan, 4);
        if (!buf) {
            printf("Could not load image at `%s`!\n", path.c_str());
            return {};
        }

        Texture tex = Texture()
            .with_data(buf, w, h, GL_RGBA);

        stbi_image_free(buf);
        return tex;
    }

    Texture()
    {
        glGenTextures(1, &texture);
    }


    ~Texture()
    {
        if (!valid)
            return;
        glDeleteTextures(1, &texture);
    }

    Texture with_data(const uint8_t *buf, int w, int h, GLint format)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, buf);
        glGenerateMipmap(GL_TEXTURE_2D);
        return std::move(*this);
    }
};

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
    GLuint vbo, ebo, vao;
    bool dirty = true;

    RESOURCE_IMPL(VertArrayObj);

    VertArrayObj()
    {
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glGenVertexArrays(1, &vao);
    }

    ~VertArrayObj()
    {
        if (!valid)
            return;
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
};

struct Model {
    VertArrayObj vao;

    glm::mat4 xform = glm::identity<glm::mat4>();

    std::vector<Vertex> vertices;
    std::vector<VIndices> indices;

    Model(std::vector<Vertex> vertices, std::vector<VIndices> indices):
        vertices(vertices), indices(indices)
    {
    }
};

struct Camera {
    glm::mat4 projection = glm::perspective((float)M_PI / 4.f, 16.f/9.f, 0.1f, 100.f);
    glm::mat4 cached_xform;
    bool xform_dirty = true;

    glm::vec3 pos = glm::zero<glm::vec3>();
    glm::vec2 look = glm::zero<glm::vec2>();

    const glm::mat4 &xform() {
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
};

struct Controller {
    glm::vec3 movement = glm::zero<glm::vec3>();
    glm::vec2 look = glm::zero<glm::vec2>();
};

struct RenderCtx {
    std::optional<ShaderProgram> sh_program;
    std::optional<Model> verts;
    std::optional<Texture> texture;
    std::optional<Camera> camera;

    InputBuffer input_buf;
    Controller controller;
};

#define VA_OFFSETOF(type, mem) ( &((type *)NULL)->mem )

void DrawFrame(RenderCtx *ctx)
{
#define VTX_POS_ARG 0
#define VTX_TEXPOS_ARG 1

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const ShaderProgram &sh_program = ctx->sh_program.value();
    glUseProgram(sh_program.id);

    GLint u_time = glGetUniformLocation(sh_program.id, "u_time");
    // GLint u_texture = glGetUniformLocation(sh_program, "u_texture");
    GLint u_model = glGetUniformLocation(sh_program.id, "u_model");
    GLint u_view = glGetUniformLocation(sh_program.id, "u_view");
    GLint u_proj = glGetUniformLocation(sh_program.id, "u_proj");
    glUniform1f(u_time, (float)SDL_GetTicks() / 1000.f);

    // const Texture &texture = ctx->texture.value();
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, texture.texture);
    // glUniform1i(u_texture, 0);

    Model &model = ctx->verts.value();
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model.xform));
    auto &vertices = model.vertices;
    auto &indices = model.indices;
    auto &vao = model.vao;

    Camera &camera = ctx->camera.value();
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(camera.xform()));
    glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(camera.projection));

    glBindVertexArray(vao.vao);
    if (vao.dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, vao.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(VIndices), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(VTX_POS_ARG, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), VA_OFFSETOF(Vertex, x));
        glEnableVertexAttribArray(VTX_POS_ARG);

        glVertexAttribPointer(VTX_TEXPOS_ARG, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), VA_OFFSETOF(Vertex, tex_u));
        glEnableVertexAttribArray(VTX_TEXPOS_ARG);

        vao.dirty = false;
    }

    glDrawElements(GL_TRIANGLES, indices.size() * sizeof(VIndices) / sizeof(float), GL_UNSIGNED_INT, 0);
}

void Update(RenderCtx *ctx, float dt)
{
    glm::vec3 &movement = ctx->controller.movement;
    glm::vec2 &look = ctx->controller.look;

    std::optional<Input> input;
    while ((input = ctx->input_buf.pop())) {
        switch (input->type) {
        case InputType::Key:
            switch (input->digital.source) {
            case SDL_SCANCODE_W:
                movement.z = -input->digital.press;
                break;
            case SDL_SCANCODE_S:
                movement.z = input->digital.press;
                break;
            case SDL_SCANCODE_A:
                movement.x = -input->digital.press;
                break;
            case SDL_SCANCODE_D:
                movement.x = input->digital.press;
                break;
            case SDL_SCANCODE_LSHIFT:
                movement.y = input->digital.press;
                break;
            case SDL_SCANCODE_LCTRL:
                movement.y = -input->digital.press;
                break;
            case SDL_SCANCODE_UP:
                look.y = input->digital.press;
                break;
            case SDL_SCANCODE_DOWN:
                look.y = -input->digital.press;
                break;
            case SDL_SCANCODE_LEFT:
                look.x = -input->digital.press;
                break;
            case SDL_SCANCODE_RIGHT:
                look.x = input->digital.press;
                break;
            }
            break;
        default:
            break;
        }
    }

    //Model &model = ctx->verts.value();
    // // want to rotate pi rad/sec
    //model.xform = glm::rotate(model.xform, dt * (float)M_PI, glm::vec3(0.f, 1.f, 0.f));

    Camera &camera = ctx->camera.value();
    static const float mv_speed = 10.f;
    static const float look_speed = 3.f;

    camera.look += dt * look_speed * look;
    glm::vec3 look_movement = glm::rotate(movement, -camera.look.x, glm::vec3(0, 1, 0));
    camera.pos += dt * mv_speed * look_movement;
    if (camera.look.y > M_PI/2)
        camera.look.y = M_PI/2;
    if (camera.look.y < -M_PI/2)
        camera.look.y = -M_PI/2;
    camera.xform_dirty = true;
}

int ProvideShaders(RenderCtx *ctx)
{
    auto sh_vertex = LoadShaderFile("../vertex.glsl", GL_VERTEX_SHADER);
    if (!sh_vertex)
        return -1;

    auto sh_fragment = LoadShaderFile("../fragment.glsl", GL_FRAGMENT_SHADER);
    if (!sh_fragment)
        return -1;

    ShaderProgram program;
    glAttachShader(program.id, sh_vertex->id);
    glAttachShader(program.id, sh_fragment->id);
    glLinkProgram(program.id);

    int success;
    glGetProgramiv(program.id, GL_LINK_STATUS, &success);
    if (!success) {
        printf("Failed to link shaders!\n");

        std::array<char, 512> log;
        glGetProgramInfoLog(program.id, 512, nullptr, &log[0]);
        printf("%s", &log[0]);
        return -1;
    }

    ctx->sh_program = std::move(program);
    return 0;
}

void ProvideModel(RenderCtx *ctx)
{
    unsigned res_x = 1000;
    unsigned res_y = 1000;

    float x_min = 0, x_max = 2*M_PI;
    float y_min = 0, y_max = 2*M_PI;

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

    ctx->verts.emplace(vertices, indices);
}

void ProvideTexture(RenderCtx *ctx)
{
    (void)ctx;
    //ctx->texture = Texture::from_path("../space.jpg");
}

void ProvideCamera(RenderCtx *ctx)
{
    Camera camera;
    camera.pos = glm::vec3(0.f, 0.f, 2.f);
    ctx->camera = camera;
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    int win_w = 480, win_h = 320;
    int win_flags = SDL_WINDOW_OPENGL;
    SDL_Window *window = SDL_CreateWindow("...", 0, 0, win_w, win_h, win_flags);
    CHECK_RET(!window, "SDL create Window failed!");
    DEFER({ SDL_DestroyWindow(window); });

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    CHECK_RET(!glcontext, "Failed to init GL context!");
    DEFER({ SDL_GL_DeleteContext(glcontext); });

    ret = !gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    CHECK_RET(ret, "Failed to initialized GLAD!");

    RenderCtx render_ctx;
    ProvideShaders(&render_ctx);
    ProvideModel(&render_ctx);
    ProvideTexture(&render_ctx);
    ProvideCamera(&render_ctx);

    glEnable(GL_DEPTH_TEST);

    SDL_Event ev;
    bool running = true;

    float time = SDL_GetTicks() / 1000.f;
    float dt = 0.f;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                Input input;
                input.type = InputType::Key;
                input.digital = {
                    ev.key.keysym.scancode,
                    ev.key.state == SDL_PRESSED,
                };
                render_ctx.input_buf.push(input);
                break;
            }
            default: break;
            }
        }

        Update(&render_ctx, dt);
        DrawFrame(&render_ctx);
        SDL_GL_SwapWindow(window);

        float now = SDL_GetTicks() / 1000.f;
        dt = now - time;
        time = now;
    }
}
