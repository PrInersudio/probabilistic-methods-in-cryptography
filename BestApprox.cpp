#include <sstream>
#include <iostream>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include "SubstituteGenerator.hpp"

static CoordinateFunction expand(const std::bitset<32> &f5, const uint8_t new_var) noexcept {
    const uint8_t block_size = 1U << (5 - new_var);
    CoordinateFunction expanded;
    for (uint8_t block_index = 0; block_index < 32 / block_size; ++block_index)
        for (uint8_t repetition : {uint8_t(0), uint8_t(1)})
            for (uint8_t block_pos = 0; block_pos < block_size; ++block_pos)
                expanded[static_cast<size_t>(block_size * (block_index * 2 + repetition) + block_pos)] =
                    f5[static_cast<size_t>(block_index * block_size + block_pos)];
    return expanded;
}

inline uint8_t countMatches(const std::bitset<64>& a, const std::bitset<64>& b) noexcept {
    return 64 - static_cast<uint8_t>((a^b).count());
}

static std::string getZhegalkin(const std::bitset<32> &f5, const std::array<uint8_t, 5> &var_indexes) noexcept {
    std::bitset<32> zhegalkin = f5;
    for (uint8_t i = 0; i < 5; ++i) {
        const uint8_t block_size = uint8_t(1) << i;
        for (uint8_t j = 0; j < 32 / block_size; j+=2)
            for (uint8_t k = 0; k < block_size; ++k)
                zhegalkin[static_cast<size_t>((j+1) * block_size + k)] =
                zhegalkin[static_cast<size_t>((j+1) * block_size + k)] ^
                zhegalkin[static_cast<size_t>(j * block_size + k)];
    }
    std::ostringstream oss;
    bool first = true;
    if (zhegalkin[0]) {
        oss << "1";
        first = false;
    }
    for (size_t i = 1; i < 32; ++i) {
        if (!zhegalkin[i]) continue;
        if (!first) oss << " + ";
        first = false;
        for (uint8_t bit = 0; bit < 5; ++bit)
            if (i & (1 << (4 - bit)))
                oss << "x_" << var_indexes[bit];
    }
    if (first) oss << "0";
    return oss.str();
}

struct BestApprox {
    std::mutex mutex;
    std::bitset<32> approx;
    uint8_t var;
    uint8_t matches;
};


int main() {
    Substitute substitute = genSubstitute();
    CoordinateFunctions coordinate_functions = toCoordinateFunctions(substitute);

    BestApprox best_approx[6];
    for (auto &appr : best_approx) {
        appr.approx.reset();
        appr.var = 0;
        appr.matches = 0;
    }
    for (uint8_t i = 0; i < 6; ++i)
        tbb::parallel_for(tbb::blocked_range<uint64_t>(0, 1UL << 32, 65536),
        [&](const tbb::blocked_range<size_t>& r) {
            BestApprox local_best_approx; local_best_approx.approx.reset(); local_best_approx.var = 0; local_best_approx.matches = 0;
            for (uint64_t possible_approx = r.begin(); possible_approx != r.end(); ++possible_approx) {
                std::bitset<32> approx(possible_approx);
                for (uint8_t var = 0; var < 6; ++var) {
                    std::bitset<64> expanded = expand(approx, var);
                    uint8_t matches = countMatches(expanded, coordinate_functions[i]);
                    if (matches > local_best_approx.matches) {
                        local_best_approx.approx = approx;
                        local_best_approx.var = var;
                        local_best_approx.matches = matches;
                    }
                }
            }
            std::lock_guard lock(best_approx[i].mutex);
            if (local_best_approx.matches > best_approx[i].matches) {
                best_approx[i].approx = local_best_approx.approx;
                best_approx[i].var = local_best_approx.var;
                best_approx[i].matches = local_best_approx.matches;
            }
        });
    for (uint8_t i = 0; i < 6; ++i) {
        std::array<uint8_t, 5> var_indexes;
        auto it = var_indexes.begin();
        for (uint8_t j = 0; j < 6 ; ++j) {
            if (j == best_approx[i].var) continue;
            *it = j + 1;
            ++it;
        }
        std::cout << "Наилучшее приближение функции " << funcLabel(i) << ": "
                  << getZhegalkin(best_approx[i].approx, var_indexes)
                  << " Вероятность совпадения: " << static_cast<double>(best_approx[i].matches) / 64
                  << "\n"; 
    }
    std::cout << std::endl;
    return 0;
}