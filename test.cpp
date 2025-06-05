#include <cinttypes>
#include <array>
#include <bitset>
#include <iostream>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

constexpr uint8_t pbox[] = { 16, 13, 14, 17, 12, 15, 9, 7, 11, 6, 10, 8, 19, 22, 23, 20, 21, 18, 1, 3, 0, 4, 2, 5 };
constexpr uint8_t invpbox[] = { 20, 18, 22, 19, 21, 23, 9, 7, 11, 6, 10, 8, 4, 1, 2, 5, 0, 3, 17, 12, 15, 16, 13, 14 };

inline std::array<uint8_t, 3> &L(std::array<uint8_t, 3> &block, const uint8_t (&Pbox)[24]) noexcept {
    uint32_t input =
        (static_cast<uint32_t>(block[0]) << 16) |
        (static_cast<uint32_t>(block[1]) << 8) |
        block[2];
    uint32_t output = 0;
    for (uint8_t i = 0; i < 24; ++i)
        output |= ((input >> (23 - Pbox[i])) & 1) << (23 - i);
    block[0] = static_cast<uint8_t>((output >> 16) & 0xFF);
    block[1] = static_cast<uint8_t>((output >> 8) & 0xFF);
    block[2] = static_cast<uint8_t>(output & 0xFF);
    return block;
}

inline std::bitset<6> Pbox(const std::bitset<6> &subblock, const uint8_t num_subblock) noexcept {
    std::bitset<6> result;
    switch (num_subblock) {
        case 0:
            result[0] = subblock[1];
            result[1] = subblock[3];
            result[2] = subblock[0];
            result[3] = subblock[4];
            result[4] = subblock[2];
            result[5] = subblock[5];
            break;
        case 1:
            result[0] = subblock[3];
            result[1] = subblock[1];
            result[2] = subblock[5];
            result[3] = subblock[0];
            result[4] = subblock[4];
            result[5] = subblock[2];
            break;
        case 2:
            result[0] = subblock[4];
            result[1] = subblock[1];
            result[2] = subblock[2];
            result[3] = subblock[5];
            result[4] = subblock[0];
            result[5] = subblock[3];
            break;
        case 3:
            result[0] = subblock[1];
            result[1] = subblock[4];
            result[2] = subblock[5];
            result[3] = subblock[2];
            result[4] = subblock[3];
            result[5] = subblock[0];
            break;
        default:
            break;
    }
    return result;
}

inline std::bitset<6> invPbox(const std::bitset<6> &subblock, const uint8_t num_subblock) noexcept {
    std::bitset<6> result;
    switch (num_subblock) {
        case 0:
            result[0] = subblock[4];
            result[1] = subblock[1];
            result[2] = subblock[2];
            result[3] = subblock[5];
            result[4] = subblock[0];
            result[5] = subblock[3];
            break;
        case 1:
            result[0] = subblock[3];
            result[1] = subblock[1];
            result[2] = subblock[5];
            result[3] = subblock[0];
            result[4] = subblock[4];
            result[5] = subblock[2];
            break;
        case 2:
            result[0] = subblock[5];
            result[1] = subblock[0];
            result[2] = subblock[3];
            result[3] = subblock[4];
            result[4] = subblock[1];
            result[5] = subblock[2];
            break;
        case 3:
            result[0] = subblock[2];
            result[1] = subblock[0];
            result[2] = subblock[4];
            result[3] = subblock[1];
            result[4] = subblock[3];
            result[5] = subblock[5];
            break;
        default:
            break;
    }
    return result;
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
                getSubblocks(result1, L(block, invpbox).data(), 0);

                std::bitset<6> result2[4];
                result2[2] = invPbox(subblocks[0], 0);
                result2[1] = invPbox(subblocks[1], 1);
                result2[3] = invPbox(subblocks[2], 2);
                result2[0] = invPbox(subblocks[3], 3);

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