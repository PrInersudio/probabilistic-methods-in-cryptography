#include "SubstituteGenerator.hpp"
#include "Lab1.hpp"
#include "Lab2.hpp"
#include "Lab3.hpp"

int main() {

    // Генерация подстановки.
    Substitute substitute = genSubstitute();
    CoordinateFunctions coordinate_functions = toCoordinateFunctions(substitute);

    // Лабораторная 1.
    getWeights(coordinate_functions);
    ZhegalkinPolynomials zhegalkin_polynomials = toZhegalkinPolynomials(coordinate_functions);
    getFictitiousVariables(zhegalkin_polynomials);

    // Лабораторная 2.
    getDominances(coordinate_functions);
    //strongEquiprobabilities(coordinate_functions);

    // Лабораторная 3.
    std::array<FourierCoefficients, 6> fourier_coefficients = getFourierCoefficients(coordinate_functions);
    std::array<WalshHadamardCoefficients, 6> walsh_hadamard_coefficients = getWalshHadamardCoefficients(fourier_coefficients);
    std::array<FourierCoefficients, 6UL> spectrum = getSpectrum(walsh_hadamard_coefficients);
    getCorrelationImmunityOrder(walsh_hadamard_coefficients);
    findBestLinearApproximations(spectrum);
    isBent(walsh_hadamard_coefficients);
}