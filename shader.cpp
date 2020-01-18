#include "shader.h"

#include <array>

#include <glad/glad.h>

Shader::Shader(int type)
{
    this->id = glCreateShader(type);
}

Shader::~Shader()
{
    if (!this->valid)
        return;
    glDeleteShader(this->id);
}



ShaderProgram::ShaderProgram()
{
    this->id = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    if (!this->valid)
        return;
    glDeleteProgram(this->id);
}



std::optional<Shader> LoadShaderFile(const std::string &filename, int type, ShaderMod mod)
{
    Shader shader(type);
    
    FILE *file = fopen(filename.c_str(), "r");
    if (!file) {
        printf("Could not open shader at `%s`!\n", filename.c_str());
        return {};
    }

    std::vector<char> shader_src(1024);
    size_t buf_pos = 0;
    while (fread(&shader_src[buf_pos], 1, 1024, file) != 0) {
        buf_pos += 1024;
        shader_src.resize(buf_pos);
    }

    fclose(file);

    mod(&shader_src);

    const char *src_start = &shader_src[0];
    glShaderSource(shader.id, 1, &src_start, NULL);
    glCompileShader(shader.id);

    int success;
    glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success);
    if (!success) {
        printf("Failed to compile shader at `%s`!\n", filename.c_str());

        std::array<char, 512> log;
        glGetShaderInfoLog(shader.id, 512, nullptr, &log[0]);
        printf("%s", &log[0]);

        return {};
    }

    return shader;
}



std::optional<Shader> LoadShaderFile(const std::string &filename, int type)
{
    auto shader_mod = [](std::vector<char> *){};
    return LoadShaderFile(filename, type, shader_mod);
}