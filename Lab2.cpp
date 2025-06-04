#include <iostream>
#include <map>
#include <sstream>
#include "Lab2.hpp"

Dominances getDominances(const CoordinateFunctions &coordinate_functions) noexcept {
    Dominances dominances;
    std::cout << "Преобладания координатных функций: ";
    for (uint8_t i = 0; i < 6; ++i) {
        dominances[i] = ssize_t(64) - static_cast<ssize_t>(2 * coordinate_functions[i].count());
        std::cout << "d(f_" << i + 1 << ") = " << dominances[i] << " ";
    }
    std::cout << std::endl;
    return dominances;
}

using ProhibitionTable = std::map<uint64_t, uint64_t>;

static ProhibitionTable initProhibitionTable(const CoordinateFunction &coordinate_function) noexcept {
    ProhibitionTable table;
    for (uint8_t i = 0; i < 64; ++i)
        table[i] = coordinate_function[i];
    return table;
}

static std::string lowBitsToString(uint64_t value, size_t n) noexcept {
    std::ostringstream oss;
    for (size_t i = 0; i < n; ++i)
        oss << ((value >> (n - 1 - i)) & 1 ? "1" : "0");
    return oss.str();
}


static void printProhibitionTable(const ProhibitionTable &table, size_t input_size, size_t output_size) noexcept {
    for (const auto &[input, output] : table) {
        std::cout << lowBitsToString(input, input_size);
        std::cout << " -> ";
        std::cout << lowBitsToString(output, output_size);
        std::cout << "\n";
    }
    std::cout << std::endl;
}

static bool filterProhibitionTable(ProhibitionTable &table) {
    std::map<uint64_t, size_t> output_counts;
    for (const auto &[input, output] : table)
        ++output_counts[output];
    bool equal = true;
    auto least_freq_it = output_counts.begin();
    for (auto it = std::next(output_counts.begin()); it != output_counts.end(); ++it)
        if (it->second < least_freq_it->second) {
            least_freq_it = it;
            equal = false;
        } else if (it->second != least_freq_it->second) equal = false;
    if (equal) return true;
    for (auto it = table.begin(); it != table.end(); ) {
        if (it->second != least_freq_it->first) it = table.erase(it);
        else ++it;
    }
    return false;
}

static ProhibitionTable updateProhibitionTable(const ProhibitionTable &table, const CoordinateFunction &coordinate_function) noexcept {
    ProhibitionTable new_table;
    for (const auto &[input, output] : table)
        for (const uint8_t new_input_bit : {uint8_t(0), uint8_t(1)}) {
            uint64_t new_input = (input << 1) | new_input_bit;
            new_table[new_input] = (output << 1) | coordinate_function[new_input & 0x3F]; 
        }
    return new_table;
}

static int findProhibitionStep(
    ProhibitionTable &new_table,
    ProhibitionTable &table,
    const CoordinateFunction &coordinate_function
) noexcept {
    ProhibitionTable tables[2];
    for (const auto &[input, output] : table)
        for (const uint8_t new_input_bit : {uint8_t(0), uint8_t(1)}) {
            uint64_t new_input = (input << 1) | new_input_bit;
            uint64_t new_output_bit = coordinate_function[new_input & 0x3F];
            tables[new_output_bit][new_input] = (output << 1) | new_output_bit;
        }
    if (tables[0].empty() || tables[1].empty()) return 2;
    if (tables[0].size() < tables[1].size()) {
        new_table = std::move(tables[0]);
        return 0;
    }
    new_table = std::move(tables[1]);
    return 1;
}

static Prohibition strongEquiprobability(const CoordinateFunction &coordinate_function) noexcept {
    constexpr size_t tries = 20;

    ProhibitionTable table = initProhibitionTable(coordinate_function);
    size_t i = 0;
    for (; i < tries; ++i) {
        printProhibitionTable(table, 6 + i, i + 1);
        if (!filterProhibitionTable(table)) {
            std::cout << (i+1) << "-битовые выходы не равновероятны." << std::endl;
            break;
        }
        std::cout << (i+1)<< "-битовые выходы равновероятны." << std::endl;
        table = updateProhibitionTable(table, coordinate_function);
    }
    if (i >= tries) {
        std::cout << "Было сделано " << tries << " попыток. С большой долей вероятности считаем функцию сильно равновероятной" << std::endl;
        return Prohibition(0, 0);;
    }
    std::cout << "Начинаем поиск запретов." << std::endl; 
    printProhibitionTable(table, 6 + i, i + 1);
    ProhibitionTable new_table;
    int prohibition_last_bit = findProhibitionStep(new_table, table, coordinate_function);
    while (prohibition_last_bit != 2) {
        table = new_table;
        ++i;
        printProhibitionTable(table, 6 + i, i + 1);
        prohibition_last_bit = findProhibitionStep(new_table, table, coordinate_function);
    }
    printProhibitionTable(new_table, 6 + i, i + 1);
    uint64_t prohibition = table.begin()->second;
    prohibition = (prohibition << 1) | static_cast<uint64_t>(prohibition_last_bit);
    ++i;
    return Prohibition(prohibition, i);
}

Prohibitions strongEquiprobabilities(const CoordinateFunctions &coordinate_functions) noexcept {
    std::cout << "Проверка на сильную равновероятность:" << std::endl;
    Prohibitions prohibitions;
    for (uint8_t i = 0; i < 6; ++i) {
        prohibitions[i] = strongEquiprobability(coordinate_functions[i]);
        if (prohibitions[i].size != 0)
            std::cout << "Запрет " << funcLabel(i) << ": " << lowBitsToString(prohibitions[i].prohibition, prohibitions[i].size) << "\n";
    }
    std::cout << std::endl;
    return prohibitions;
}