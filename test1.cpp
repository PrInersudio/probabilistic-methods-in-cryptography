#include <cinttypes>
#include <array>
#include <bitset>
#include <iostream>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

constexpr uint8_t sbox[] = {
    58,  1, 55, 53, 49, 35, 36, 11,
    45, 19,  8,  4, 63, 50,  2, 22,
    34,  5, 26, 60, 15, 47, 59, 20,
    54, 28, 29, 17, 24, 12, 16, 13,
    56, 33, 18, 41,  3,  0, 21,  6,
    10, 52, 42, 23, 61, 51, 46, 39,
    40, 62, 37, 27, 43,  9, 57,  7,
    32, 31, 30, 44, 14, 38, 25, 48
};

constexpr uint8_t invsbox[] = {
    37,  1, 14, 36, 11, 17, 39, 55,
    10, 53, 40,  7, 29, 31, 60, 20,
    30, 27, 34,  9, 23, 38, 15, 43,
    28, 62, 18, 51, 25, 26, 58, 57,
    56, 33, 16,  5,  6, 50, 61, 47,
    48, 35, 42, 52, 59,  8, 46, 21,
    63,  4, 13, 45, 41,  3, 24,  2,
    32, 54,  0, 22, 19, 44, 49, 12
};

inline std::array<uint8_t, 3> &S(std::array<uint8_t, 3> &block, const uint8_t (&Sbox)[64]) noexcept {
    uint8_t v0 = block[0] >> 2, v1 = ((block[0] & 0b11) << 4) | (block[1] >> 4),
        v2 = ((block[1] & 0b1111) << 2) | (block[2] >> 6), v3 = block[2] & 0b111111;
    v0 = Sbox[v0]; v1 = Sbox[v1]; v2 = Sbox[v2]; v3 = Sbox[v3];
    block[0] = (v0 << 2) | (v1 >> 4); block[1] = (v1 << 4) | (v2 >> 2);
    block[2] = (v2 << 6) | v3;
    return block;
}

inline void getSubblocks(std::bitset<6> (&subblocks)[4], const uint8_t *text, const size_t pos) noexcept {
    uint32_t block =
        (static_cast<uint32_t>(text[pos]) << 16) |
        (static_cast<uint32_t>(text[pos + 1]) << 8) | 
        static_cast<uint32_t>(text[pos + 2]);
    for (uint8_t i = 0; i < 4; ++i)
        for (uint8_t j = 0; j < 6; ++j)
            subblocks[i][j] = (block >> (23 - (i * 6 + j))) & 1;
}

std::bitset<6> S(const std::bitset<6>& input) noexcept {
    uint8_t index = 0;
    for (uint8_t i = 0; i < 6; ++i) {
        index <<= 1;
        index |= input[i];
    }
    uint8_t result = sbox[index];
    std::bitset<6> output;
    for (int i = 5; i >= 0; --i)
        output[static_cast<size_t>(5 - i)] = (result >> i) & 1;
    return output;
}

int main() {
    std::atomic<bool> mismatch_found = false;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, 256 * 256 * 256),
        [&](const tbb::blocked_range<size_t> &range) {
            for (size_t index = range.begin(); index < range.end() && !mismatch_found; ++index) {
                uint8_t a = (index >> 16) & 0xFF;
                uint8_t b = (index >> 8) & 0xFF;
                uint8_t c = index & 0xFF;
                std::array<uint8_t, 3> block = {a, b, c};
                std::bitset<6> subblocks[4];
                getSubblocks(subblocks, block.data(), 0);

                std::bitset<6> result1[4];
                getSubblocks(result1, S(block, sbox).data(), 0);

                std::bitset<6> result2[4];
                result2[0] = S(subblocks[0]);
                result2[1] = S(subblocks[1]);
                result2[2] = S(subblocks[2]);
                result2[3] = S(subblocks[3]);

                for (uint8_t i = 0; i < 4; ++i)
                    if (result1[i] != result2[i]) {
                        std::cout << int(a) << " " << int(b) << " " << int(c) << " " << int(i) << " " << result1[i] << " " << result2[i] << std::endl;
                        mismatch_found = true;
                        return;
                    }
            }
        });
    if (!mismatch_found)
        std::cout << "Норм" << std::endl;
    return 0;
}