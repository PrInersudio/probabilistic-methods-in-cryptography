#include <iostream>
#include "LabsDefinitions.hpp"

Weights getWeights(const CoordinateFunctions &coordinate_functions) noexcept {
    Weights weights;
    std::cout << "Веса координатных функций: ";
    for (uint8_t i = 0; i < 6; ++i) {
        weights[i] = coordinate_functions[i].count();
        std::cout << "||f_" << i + 1 << "|| = " << weights[i] << " ";
    }
    std::cout << std::endl;
    return weights;
}

ZhegalkinPolynomials toZhegalkinPolynomials(const CoordinateFunctions &coordinate_functions) noexcept {
    ZhegalkinPolynomials zhegalkin_polynomials = coordinate_functions;
    std::cout << "Многочлены Жегалина:\n";
    for (uint8_t m = 0; m < 6; ++m) {
        std::cout << funcLabel(m) << ":\n";
        for (uint8_t i = 0; i < 6; ++i) {
            const uint8_t block_size = uint8_t(1) << i;
            std::cout << toString(zhegalkin_polynomials[m], block_size) << "\n";
            for (uint8_t j = 0; j < 64 / block_size; j+=2)
                for (uint8_t k = 0; k < block_size; ++k)
                    zhegalkin_polynomials[m][static_cast<size_t>((j+1) * block_size + k)] =
                    zhegalkin_polynomials[m][static_cast<size_t>((j+1) * block_size + k)] ^
                    zhegalkin_polynomials[m][static_cast<size_t>(j * block_size + k)];
        }
        std::cout << toString(zhegalkin_polynomials[m]) << "\n";
        std::cout << toZhegalkinString(zhegalkin_polynomials[m]) << "\n";
    }
    std::cout << std::endl;
    return zhegalkin_polynomials;
}

FictitiousVariables getFictitiousVariables(const ZhegalkinPolynomials &zhegalkin_polynomials) noexcept {
    FictitiousVariables fictitious_variables;
    for (uint8_t i = 0; i < 6; ++i) {
        fictitious_variables[i].set();
        for (uint8_t j = 0; j < 6; ++j)
            for (uint8_t k = 0; k < 64; ++k)
                if (zhegalkin_polynomials[i][k] && (k & (uint8_t(1) << (5 - j))))
                    fictitious_variables[i][j] = 0;
    }
    for (uint8_t i = 0; i < 6; ++i) {
        bool has_any = false;
        std::cout << "Фиктивные переменные: " << funcLabel(i) << ": ";
        for (uint8_t j = 0; j < 6; ++j)
            if (fictitious_variables[i][j]) {
                std::cout << "x_" << (j+1) << " ";
                has_any = true;
            }
        if (!has_any) std::cout << "(нет)";
        std::cout << "\n";
    }
    std::cout << std::endl;
    return fictitious_variables;
}