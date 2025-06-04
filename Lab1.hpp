#pragma once

Weights getWeights(const CoordinateFunctions &coordinate_functions) noexcept;
ZhegalkinPolynomials toZhegalkinPolynomials(const CoordinateFunctions &coordinate_functions) noexcept;
FictitiousVariables getFictitiousVariables(const ZhegalkinPolynomials &zhegalkin_polynomials) noexcept;