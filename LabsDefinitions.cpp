#include <sstream>
#include "LabsDefinitions.hpp"

std::string toString(const Substitute &substitute) noexcept {
    std::ostringstream oss;
    oss << "{ ";
    for (size_t i = 0; i < 63; ++i) {
        oss << static_cast<int>(substitute[i]) << ", ";
    }
    oss << static_cast<int>(substitute[63]);
    oss << " }";
    return oss.str();
}

std::string toString(const CoordinateFunctions &coordinate_functions) noexcept {
    std::ostringstream oss;
    for (uint8_t i = 0; i < 6; ++i) {
        oss << funcLabel(i) << " = ";
        for (size_t j = 0; j < 64; ++j)
            oss << (coordinate_functions[i][j] ? '1' : '0');
        oss << '\n';
    }
    return oss.str();
}

std::string toString(const CoordinateFunction &f, size_t n) noexcept {
    std::ostringstream oss;
    for (size_t i = 0; i < 64; ++i) {
        oss << (f[i] ? '1' : '0');
        if ((i + 1) % n == 0 && i != 63)
            oss << ' ';
    }
    return oss.str();
}

std::string toZhegalkinString(const ZhegalkinPolynomial &poly) noexcept {
    std::ostringstream oss;
    bool first = true;
    if (poly[0]) {
        oss << "1";
        first = false;
    }
    for (size_t i = 1; i < 64; ++i) {
        if (!poly[i]) continue;
        if (!first) oss << " + ";
        first = false;
        for (uint8_t bit = 0; bit < 6; ++bit)
            if (i & (1 << (5 - bit)))
                oss << "x_" << (bit + 1);
    }
    if (first) oss << "0";
    return oss.str();
}