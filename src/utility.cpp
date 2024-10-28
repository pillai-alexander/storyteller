#include <storyteller/utility.hpp>

namespace constants {
    unsigned int ONE = 1;
}

namespace util {
    double gamma_scale_from_mean(double shape, double mean) {
        return mean / shape;
    }
}