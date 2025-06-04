#pragma once

#include <bitset>
#include <cinttypes>
#include <array>
#include <vector>

inline std::string funcLabel(uint8_t num) {
    return "f_" + std::to_string(static_cast<unsigned>(num + 1));
}

using Substitute = std::array<uint8_t, 64>;
using CoordinateFunction = std::bitset<64>;
using CoordinateFunctions = std::array<CoordinateFunction, 6>;
using ZhegalkinPolynomial = CoordinateFunction;
using ZhegalkinPolynomials = CoordinateFunctions;

using Weights = std::array<size_t, 6>;
using FictitiousVariables = std::array<std::bitset<6>, 6>;

using Dominances = std::array<ssize_t, 6>;
struct Prohibition { uint64_t prohibition; size_t size; };
using Prohibitions = std::array<Prohibition, 6>;

using FourierCoefficients = std::array<double, 64>;
using WalshHadamardCoefficients = FourierCoefficients;
using Spectrum = WalshHadamardCoefficients;

std::string toString(const Substitute &substitute) noexcept;
std::string toString(const CoordinateFunctions &coordinate_functions) noexcept;
std::string toString(const CoordinateFunction &f, size_t n = 64) noexcept;
std::string toZhegalkinString(const ZhegalkinPolynomial &poly) noexcept;