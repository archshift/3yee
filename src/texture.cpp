#include "texture.h"

#include <GL/glew.h>

#include <stb_image.h>

std::optional<Texture> Texture::from_path(const std::string &path)
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



Texture Texture::with_data(const uint8_t *buf, int w, int h, GLint format)
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



Texture::Texture()
{
    glGenTextures(1, &texture);
}

Texture::~Texture()
{
    if (!valid)
        return;
    glDeleteTextures(1, &texture);
}

