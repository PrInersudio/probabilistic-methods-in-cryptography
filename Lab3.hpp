#pragma once

#include <list>
#include "LabsDefinitions.hpp"

std::array<FourierCoefficients, 6> getFourierCoefficients(const CoordinateFunctions &coordinate_functions) noexcept;
std::array<WalshHadamardCoefficients, 6> getWalshHadamardCoefficients(const std::array<FourierCoefficients, 6> &fourier_coefficients) noexcept;
std::array<Spectrum, 6> getSpectrum(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept;
std::array<uint8_t, 6> getCorrelationImmunityOrder(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept;
std::array<std::list<uint8_t>, 6> findBestLinearApproximations(const std::array<Spectrum, 6> &spectrum) noexcept;
std::bitset<6> isBent(const std::array<WalshHadamardCoefficients, 6> &walsh_hadamard_coefficients) noexcept;