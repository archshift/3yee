#pragma once

#include <optional>
#include <vector>
#include <string>

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

typedef void (*ShaderMod)(std::vector<char> *);
std::optional<Shader> LoadShaderFile(const std::string &filename, int type, ShaderMod mod);
std::optional<Shader> LoadShaderFile(const std::string &filename, int type);
