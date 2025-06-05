#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Lab4.hpp"
#include "SubstituteGenerator.hpp"

inline uint8_t dotProduct(uint8_t a, uint8_t b) noexcept {
    return static_cast<uint8_t>(__builtin_parity(a & b));
}

inline bool good(const std::array<std::array<uint8_t, 64>, 64>& linear_characteristic, uint8_t border) noexcept {
    return std::abs(static_cast<int>(linear_characteristic[63][63] - border) >= 32);
}

std::pair<Substitute, std::array<std::array<uint8_t, 64>, 64>> linearCharacteristic() {
    std::array<std::array<uint8_t, 64>, 64> linear_characteristic;
    Substitute substitute;
    do {
        substitute = genSubstitute();
        for (uint8_t a = 0; a < 64; ++a) {
            linear_characteristic[a].fill(0);
            for (uint8_t c = 0; c < 64; ++c) {
                for (uint8_t x = 0; x < 64; ++x)
                    if ((dotProduct(x, a) ^ dotProduct(substitute[x], c)) == 0)
                        ++linear_characteristic[a][c];
            }
        }
    } while (!good(linear_characteristic, 16));
    return std::make_pair(substitute, linear_characteristic);
}