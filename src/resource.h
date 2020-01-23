#pragma once

#include <cstring>

#define RESOURCE_IMPL(Resource) \
    bool valid = true; \
    Resource& operator=(Resource&& other) \
    { \
        if (this != &other) { \
            memcpy(this, &other, sizeof(*this)); \
            other.valid = false; \
        } \
        return *this; \
    } \
    Resource(Resource &&other) \
    { \
        *this = std::move(other); \
    } \
    Resource(Resource &other) = delete;

