#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <atomic>
#include <memory>
#include <string>
#include "resource.h"
#include "optref.h"

static std::atomic_size_t uuid_gen = 0x1000;

// Odd by construction so coprime to 1<<32
constexpr size_t uuid_inc = ((size_t)(-1) / 3 + (size_t)(-1) / 7) | 1;

typedef uint64_t UuidRef;

struct Uuid {
    RESOURCE_IMPL(Uuid);
    UuidRef val;

    Uuid():
        val(uuid_gen.fetch_add(uuid_inc))
    {
    }

    operator UuidRef()
    {
        return val;
    }
};

struct Object;
struct GameState;

struct Component {
    virtual void update(GameState *ctx, Object *obj, float dt)
    {
        (void)ctx;
        (void)obj;
        (void)dt;
    }

    virtual ~Component()
    {
    }
};

struct Object final {
    Uuid uuid;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    bool updating;
    bool deleted = false;

    template <typename T>
    optref<T> component()
    {
        auto search = components.find(typeid(T));
        if (search == components.end()) {
            return {};
        }
        T *t = static_cast<T*>(search->second.get());
        T &ref_t = *t;
        return ref_t;
    }

    template <typename T>
    bool add_component(T comp)
    {
        std::type_index key(typeid(comp));
        T *t = new T(std::move(comp));
        Component *comp_ptr = static_cast<Component *>(t);
        std::unique_ptr<Component> comp_unique(comp_ptr);
        auto insert_ret = components.insert({ key, std::move(comp_unique) });
        return insert_ret.second;
    }

    void update(GameState *ctx, float dt)
    {
        updating = true;
        for (auto it = components.begin(); it != components.end(); it++) {
            Component &c = *it->second;
            c.update(ctx, this, dt);
        }
        updating = false;
    }
};
