#pragma once

#include <optional>
#include <functional>

template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;