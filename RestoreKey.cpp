#include <bitset>
#include <fstream>
#include <cinttypes>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "BlockCipherX.hpp"

#define NUM_BLOCKS 32
#define TEXT_LEN NUM_BLOCKS * 3

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
    uint32_t block = (text[pos] << 16) | (text[pos + 1] << 8) | text[pos + 2];
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

inline uint8_t dotProduct(const std::bitset<6>& a, const std::bitset<6>& b) noexcept {
    return (a & b).count() & 1;
}

struct BestKeys {
    std::bitset<6> round0;
    std::bitset<6> round2;
    size_t T = 0;
};

constexpr uint8_t Pbox_transitions[4] = { 3, 1, 0, 2 };

static std::string toHex(const std::array<uint8_t, 9> &data) noexcept {
    std::ostringstream oss;
    for (uint8_t byte : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return oss.str();
}

int main() {

    uint8_t plain_text[TEXT_LEN];
    std::ifstream plain_text_file("plain_text.bin", std::ios::binary);
    plain_text_file.read(reinterpret_cast<char *>(plain_text), TEXT_LEN);

    uint8_t cipher_text[TEXT_LEN];
    std::ifstream cipher_text_file("cipher_text.bin", std::ios::binary);
    cipher_text_file.read(reinterpret_cast<char *>(cipher_text), TEXT_LEN);

    std::bitset<6> plain_subblocks[4][NUM_BLOCKS];
    std::bitset<6> cipher_subblocks[4][NUM_BLOCKS];
    for (size_t i = 0; i < TEXT_LEN; i += 3) {
        std::bitset<6> local_plain_subblocks[4];
        getSubblocks(local_plain_subblocks, plain_text, i);
        std::bitset<6> local_cipher_subblocks[4];
        getSubblocks(local_cipher_subblocks, cipher_text, i);
        for (uint8_t j = 0; j < 4; ++j) {
            plain_subblocks[j][i / 3] = local_plain_subblocks[j];
            cipher_subblocks[j][i / 3] = local_cipher_subblocks[Pbox_transitions[j]];
        }
    }

    constexpr std::bitset<6> a(63);
    constexpr std::bitset<6> c(63);
    BestKeys max_best_keys[4];
    BestKeys min_best_keys[4];
    for (auto &bk : min_best_keys) bk.T = NUM_BLOCKS + 1;
    for (uint8_t i = 0; i < 4; ++i) {
        std::cout << static_cast<int>(i) << ":\n";
        for (uint8_t key_round0_num = 0; key_round0_num < 64; ++key_round0_num)
            for (uint8_t key_round2_num = 0; key_round2_num < 64; ++key_round2_num) {
                size_t T = 0;
                std::bitset<6> key_round0(key_round0_num);
                std::bitset<6> key_round2(key_round2_num);
                for (size_t j = 0; j < NUM_BLOCKS; ++j) {
                    std::bitset<6> C1 = Pbox(S(plain_subblocks[i][j] ^ key_round0), i);
                    std::bitset<6> C2 = invPbox(cipher_subblocks[i][j] ^ key_round2, Pbox_transitions[i]);
                    if ((dotProduct(a, C1) ^ dotProduct(c, C2)) == 0) ++T;
                }
                if (T > max_best_keys[i].T) {
                    max_best_keys[i].round0 = key_round0;
                    max_best_keys[i].round2 = key_round2;
                    max_best_keys[i].T = T;
                }
                if (T < min_best_keys[i].T) {
                    min_best_keys[i].round0 = key_round0;
                    min_best_keys[i].round2 = key_round2;
                    min_best_keys[i].T = T;
                }
                std::cout << max_best_keys[i].round0 << " " << max_best_keys[i].round2 << " " << max_best_keys[i].T << "\n";
                std::cout << min_best_keys[i].round0 << " " << min_best_keys[i].round2 << " " << min_best_keys[i].T << "\n";
                std::cout << "\n";
            }
    }
    BestKeys best_keys[4];
    uint8_t right_part_sum[4];
    for (uint8_t i = 0; i < 4; ++i) {
        if (
            std::abs(static_cast<int>(max_best_keys[i].T - NUM_BLOCKS / 2)) >
            std::abs(static_cast<int>(min_best_keys[i].T - NUM_BLOCKS / 2))
        ) {
            best_keys[i] = max_best_keys[i];
            right_part_sum[i] = 0;
        } else {
            best_keys[i] = min_best_keys[i];
            right_part_sum[i] = 1;
        }
    }

    BlockCipherX cipher;
    for (uint8_t round1_0 = 0; round1_0 < 32; ++round1_0)
        for (uint8_t round1_1 = 0; round1_1 < 32; ++round1_1)
            for (uint8_t round1_2 = 0; round1_2 < 32; ++round1_2)
                for (uint8_t round1_3 = 0; round1_3 < 32; ++round1_3) {
                    std::bitset<6> round1[4] = {round1_0, round1_1, round1_2, round1_3};
                    for (uint8_t i = 0; i <  4; ++i) round1[i][5] = (round1[i].count() & 1) ^ right_part_sum[i];
                    std::bitset<72> key_bits;
                    for (uint8_t i = 0; i < 4; ++i)
                        for (uint8_t j = 0; j < 6; ++j) {
                            key_bits[static_cast<size_t>(i * 4 + j)] = best_keys[i].round0[j];
                            key_bits[static_cast<size_t>((i+6) * 4 + j)] = round1[i][j];
                            key_bits[static_cast<size_t>((i+12) * 4 + j)] = best_keys[i].round2[j];
                        }
                    std::array<uint8_t, 9> key{};
                    for (uint8_t i = 0; i < 9; ++i)
                        for (uint8_t j = 0; j < 8; ++j)
                            key[i] |= static_cast<uint8_t>(static_cast<uint8_t>(key_bits[static_cast<size_t>(i * 8 + j)]) << (7 - j));
                    cipher.initKeySchedule(key);
                    bool correct = true;
                    for (size_t i = 0; i < NUM_BLOCKS; ++i) {
                        std::array<uint8_t, 3> plain_block{ plain_text[i], plain_text[i+1], plain_text[i+2] };
                        std::array<uint8_t, 3> cipher_block{ cipher_text[i], cipher_text[i+1], cipher_text[i+2] };
                        if (cipher.encrypt(plain_block) != cipher_block) {
                            correct = false;
                            break;
                        }
                    }
                    if (correct) {
                        std::cout << "Найден корректный ключ: " << toHex(key) << std::endl;
                        return 0;
                    }
                }
    std::cout << "Не найден корректный ключ." << std::endl;
    return -1;
}