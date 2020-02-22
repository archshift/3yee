#pragma once

#include <string>
#include <optional>

#include "resource.h"

struct Texture {
    unsigned texture;

    RESOURCE_IMPL(Texture);

    static std::optional<Texture> from_path(const std::string &path);
    Texture with_data(const uint8_t *buf, int w, int h, int format);
    Texture();
    ~Texture();
};