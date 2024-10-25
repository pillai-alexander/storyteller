#pragma once

#include <vector>

namespace constants {
    extern unsigned int ONE;
}

template<typename T> using vector2d = std::vector<std::vector<T>>;
template<typename T> using vector3d = std::vector<std::vector<std::vector<T>>>;

namespace util {
    extern double gamma_scale_from_mean(double shape, double mean);
}