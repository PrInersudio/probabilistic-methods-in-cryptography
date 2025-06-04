#include <iostream>
#include <bitset>
#include <cmath>
#include <limits>
#include <algorithm>
#include "Lab3.hpp"

static FourierCoefficients getFourierCoefficients(const CoordinateFunction &coordinate_function) noexcept {
    std::array<int, 64> fourier_coefficients_integer;
    for (uint8_t i = 0; i < 64; ++i) fourier_coefficients_integer[i] = coordinate_function[i];
    for (uint8_t i = 0; i < 6; ++i) {
        for (uint8_t j = 0; j < 64; ++j) std::cout << fourier_coefficients_integer[j] << " ";
        std::cout << "\n";
        const uint8_t block_size = uint8_t(1) << i;
        for (uint8_t j = 0; j < 64 / block_size; j+=2)
            for (uint8_t k = 0; k < block_size; ++k) {
                const uint8_t first = static_cast<uint8_t>(j * block_size + k);
                const uint8_t second = first + block_size;
                int sum = fourier_coefficients_integer[first] + fourier_coefficients_integer[second];
                int diff = fourier_coefficients_integer[first] - fourier_coefficients_integer[second];
                fourier_coefficients_integer[first] = sum;
                fourier_coefficients_integer[second] = diff;
            }
    }
    for (uint8_t j = 0; j < 64; ++j) std::cout << fourier_coefficients_integer[j] << " ";
    std::cout << "\n";
    FourierCoefficients fourier_coefficients;
    for (uint8_t i = 0; i < 64; ++i) fourier_coefficients[i] = static_cast<double>(fourier_coefficients_integer[i]) / 64;
    for (uint8_t i = 0; i < 64; ++i) std::cout << fourier_coefficients[i] << " ";
    std::cout << "\n";
    return fourier_coefficients;
}

std::array<FourierCoefficients, 6> getFourierCoefficients(const CoordinateFunctions &coordinate_functions) noexcept {
    std::array<FourierCoefficients, 6> fourier_coefficients;
    for (uint8_t i = 0; i < 6; ++i) {
        std::cout << "Коэффициенты Фурье функции " << funcLabel(i) << ":\n";
        fourier_coefficients[i] = getFourierCoefficients(coordinate_functions[i]);
    }
    std::cout << std::endl;
    return fourier_coefficients;
}

std::array<WalshHadamardCoefficients, 6> getWalshHadamardCoefficients(const std::array<FourierCoefficients, 6> &fourier_coefficients) noexcept {
    std::array<WalshHadamardCoefficients, 6> walsh_hadamard_coefficients;
    for (uint8_t i = 0; i < 6; ++i) {
        std::cout << "Коэффициенты Уолша-Адамара функции " << funcLabel(i) << ":\n";
        walsh_hadamard_coefficients[i][0] = 1 - 2 * fourier_coefficients[i][0];
        std::cout << walsh_hadamard_coefficients[i][0] << " ";
        for (uint8_t j = 1; j < 64; ++j) {
            walsh_hadamard_coefficients[i][j] = -2 * fourier_coefficients[i][j];
            std::cout << walsh_hadamard_coefficients[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
    return walsh_hadamard_coefficients;
}

std::array<Spectrum, 6> getSpectrum(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept {
    std::array<Spectrum, 6> spectrum;
    for (uint8_t i = 0; i < 6; ++i) {
        std::cout << "Спектр функции " << funcLabel(i) << ":\n";
        for (uint8_t j = 0; j < 64; ++j) {
            spectrum[i][j] = 32 * walsh_hadamard_coefficients[i][j];
            std::cout << spectrum[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
    return spectrum;
}

inline uint8_t getInputWeight(const uint8_t input) {
    uint8_t weight = 0;
    for (uint8_t i = 0; i < 6; ++i)
        if (input & (1 << i)) ++weight;
    return weight;
}

inline bool isZero(const double x) {
    return std::fabs(x) < std::numeric_limits<double>::epsilon();
}

std::array<uint8_t, 6> getCorrelationImmunityOrder(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept {
    std::array<std::bitset<6>, 6> flags;
    for (uint8_t i = 0; i < 6; ++i) {
        flags[i].set();
        for (uint8_t j = 1; j < 64; ++j)
            if (!isZero(walsh_hadamard_coefficients[i][j]))
                flags[i][getInputWeight(j) - 1] = 0;
    }
    std::array<uint8_t, 6> correlation_immunity{};
    for (uint8_t i = 0; i < 6; ++i) {
        while (correlation_immunity[i] < 6 && flags[i][correlation_immunity[i]])
            ++correlation_immunity[i];
        std::cout << "Функция " << funcLabel(i) << " корреляционно иммунна порядка " << static_cast<unsigned>(correlation_immunity[i]) << "\n";
    }
    std::cout << std::endl;
    return correlation_immunity;
}

inline bool isEqual(const double x, const double y) {
    return std::fabs(x - y) < std::numeric_limits<double>::epsilon();
}

std::array<std::list<uint8_t>, 6> findBestLinearApproximations(const std::array<Spectrum, 6> &spectrum) noexcept {
    std::array<std::list<uint8_t>, 6> best_linear_approximations;
    for (uint8_t i = 0; i < 6; ++i) {
        best_linear_approximations[i].push_back(0);
        double largest_spectrum = std::fabs(spectrum[i][0]);
        for (uint8_t j = 1; j < 64; ++j) {
            double abs_spectrum = std::fabs(spectrum[i][j]);
            if (abs_spectrum > largest_spectrum) {
                largest_spectrum = abs_spectrum;
                best_linear_approximations[i].clear();
                best_linear_approximations[i].push_back(j);
            } else if (isEqual(abs_spectrum, largest_spectrum))
                best_linear_approximations[i].push_back(j);
        }
        std::cout << "Наилучшие линейные приближения функции " << funcLabel(i) << ": ";
        for (const uint8_t u : best_linear_approximations[i]) {
            bool first = true;
            for (uint8_t j = 0; j < 6; ++j) {
                if (!(u & (1 << (5 - j)))) continue;
                if (!first) std::cout << " + ";
                first = false;
                std::cout << "x_" << (j+1);
            }
            if (first) std::cout << "0";
            std::cout << " ";
            std::cout << "P = " << 0.5 + spectrum[i][u] / 64 << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
    return best_linear_approximations;
}

std::bitset<6> isBent(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept {
    std::bitset<6> is_bent;
    for (uint8_t i = 0; i < 6; ++i) {
        is_bent[i] = std::all_of(walsh_hadamard_coefficients[i].begin(), walsh_hadamard_coefficients[i].end(), [&](const double wh){ return isEqual(std::fabs(wh), 8); });
        std::cout << "Функция " << funcLabel(i) << (is_bent[i] ? " " : " не ") << "является бент функцией.\n";
    }
    std::cout << std::endl;
    return is_bent;
}