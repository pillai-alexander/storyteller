#pragma once

#include <vector>

namespace constants {
    extern unsigned int ONE;
}

template<typename T> using vector2d = std::vector<std::vector<T>>;
template<typename T> using vector3d = std::vector<std::vector<std::vector<T>>>;