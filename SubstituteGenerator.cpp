#include <unordered_set>
#include <iostream>
#include "CTR_DRBG.hpp"
#include "Kuznechik.hpp"
#include "SubstituteGenerator.hpp"

Substitute genSubstitute() {
    CTR_DRBG<Kuznechik, true> rng;
    Substitute substitute;
    substitute[0] = static_cast<uint8_t>(rng.uint64() >> 58);
    for (uint8_t i = 1; i < 64; ++i) {
        uint8_t variant;
        do variant = static_cast<uint8_t>(rng.uint64() >> 58);
        while (std::find(substitute.begin(), substitute.begin() + i, variant) != substitute.begin() + i);
        substitute[i] = variant;
    }
    std::cout << toString(substitute) << std::endl;
    return substitute;
}

CoordinateFunctions toCoordinateFunctions(const Substitute &substitute) noexcept {
    CoordinateFunctions coordinate_functions;
    for (uint8_t i = 0; i < 64; ++i)
        for (uint8_t j = 0; j < 6; ++j)
            coordinate_functions[j][i] = (substitute[i] >> (5 - j)) & 1;
    std::cout << toString(coordinate_functions) << std::endl;
    return coordinate_functions;
}