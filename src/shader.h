#pragma once

#include <optional>
#include <string>
#include <functional>
#include <initializer_list>

#include "resource.h"

struct Shader {
    uint64_t id;

    RESOURCE_IMPL(Shader);

    Shader(int type);
    ~Shader();
};

typedef std::initializer_list<const std::reference_wrapper<Shader>> ShaderList;

struct ShaderProgram {
    uint64_t id;

    RESOURCE_IMPL(ShaderProgram);

    static std::optional<ShaderProgram> link(ShaderList shaders);
    ~ShaderProgram();

private:
    ShaderProgram();
};

typedef std::function<void (std::string *)> ShaderMod;
std::optional<Shader> LoadShaderFile(const std::string &filename, int type, ShaderMod mod);
std::optional<Shader> LoadShaderFile(const std::string &filename, int type);
