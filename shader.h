#pragma once

#include <optional>
#include <string>
#include <functional>

#include "resource.h"

struct Shader {
    uint64_t id;

    RESOURCE_IMPL(Shader);

    Shader(int type);
    ~Shader();
};

struct ShaderProgram {
    uint64_t id;

    RESOURCE_IMPL(ShaderProgram);

    ShaderProgram();
    ~ShaderProgram();
};

typedef std::function<void (std::string *)> ShaderMod;
std::optional<Shader> LoadShaderFile(const std::string &filename, int type, ShaderMod mod);
std::optional<Shader> LoadShaderFile(const std::string &filename, int type);
