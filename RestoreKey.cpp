#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include "BlockCipherX.hpp"

#define NUM_BLOCKS 256
#define TEXT_LEN NUM_BLOCKS * 3

static std::string toHex(const uint8_t *data, const size_t size) noexcept {
    std::ostringstream oss;
    for (size_t i = 0; i < size; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    return oss.str();
}

int main() {

    uint8_t plain_text[TEXT_LEN];
    std::ifstream plain_text_file("plain_text.bin", std::ios::binary);
    plain_text_file.read(reinterpret_cast<char *>(plain_text), TEXT_LEN);

    uint8_t cipher_text[TEXT_LEN];
    std::ifstream cipher_text_file("cipher_text.bin", std::ios::binary);
    cipher_text_file.read(reinterpret_cast<char *>(cipher_text), TEXT_LEN);

    std::cout << "Начато восстановление первого раундового ключа (линейный криптоанализ шифрования)." << std::endl;
    uint8_t best_k0[3]; size_t best_T0 = NUM_BLOCKS / 2;
    std::mutex access_best0;
    tbb::parallel_for(tbb::blocked_range<uint16_t>(0, 256),
    [&](const tbb::blocked_range<uint16_t> &r) {
        uint8_t local_best_k0[3];
        size_t local_best_T = NUM_BLOCKS / 2;
        for (uint16_t k00 = r.begin(); k00 != r.end(); ++k00)
            for (uint16_t k01 = 0; k01 < 256; ++k01)
                for (uint16_t k02 = 0; k02 < 256; ++k02) {
                    uint8_t k0[3] = { static_cast<uint8_t>(k00), static_cast<uint8_t>(k01), static_cast<uint8_t>(k02) };
                    size_t T = 0;
                    for (size_t i = 0; i < TEXT_LEN; i+=3) {
                        uint8_t block[3] = { plain_text[i], plain_text[i + 1], plain_text[i + 2] };
                        L(S(X(block, k0), sbox), pbox);
                        if ((__builtin_parity(block[0] >> 2) ^ __builtin_parity(cipher_text[i] >> 2)) == 0) ++T;
                    }
                    if (std::abs(static_cast<int64_t>(T) - NUM_BLOCKS / 2) > std::abs(static_cast<int64_t>(local_best_T) - NUM_BLOCKS / 2)) {
                        memcpy(local_best_k0, k0, 3);
                        local_best_T = T;
                    }
                }
        std::lock_guard lock(access_best0);
        if (std::abs(static_cast<int64_t>(local_best_T) - NUM_BLOCKS / 2) > std::abs(static_cast<int64_t>(best_T0) - NUM_BLOCKS / 2)) {
            memcpy(best_k0, local_best_k0, 3);
            best_T0 = local_best_T;
        }
    });
    std::cout << "Выбран кандидат на первый раундовый ключ: " << toHex(best_k0, 3) << " T: " << best_T0 << std::endl;

    std::cout << "Начато восстановление третьего раундового ключа (линейный криптоанализ расшифрования)." << std::endl;
    uint8_t best_k2[3]; size_t best_T2 = NUM_BLOCKS / 2;
    std::mutex access_best2;
    tbb::parallel_for(tbb::blocked_range<uint16_t>(0, 256),
    [&](const tbb::blocked_range<uint16_t> &r) {
        uint8_t local_best_k2[3];
        size_t local_best_T = NUM_BLOCKS / 2;
        for (uint16_t k20 = r.begin(); k20 != r.end(); ++k20)
            for (uint16_t k21 = 0; k21 < 256; ++k21)
                for (uint16_t k22 = 0; k22 < 256; ++k22) {
                    uint8_t k2[3] = { static_cast<uint8_t>(k20), static_cast<uint8_t>(k21), static_cast<uint8_t>(k22) };
                    size_t T = 0;
                    for (size_t i = 0; i < TEXT_LEN; i+=3) {
                        uint8_t block[3] = { cipher_text[i], cipher_text[i + 1], cipher_text[i + 2] };
                        L(S(X(block, k2), invsbox), invpbox);
                        if ((__builtin_parity(block[0] >> 2) ^ __builtin_parity(plain_text[i] >> 2)) == 0) ++T;
                    }
                    if (std::abs(static_cast<int64_t>(T) - NUM_BLOCKS / 2) > std::abs(static_cast<int64_t>(local_best_T) - NUM_BLOCKS / 2)) {
                        memcpy(local_best_k2, k2, 3);
                        local_best_T = T;
                    }
                }
        std::lock_guard lock(access_best2);
        if (std::abs(static_cast<int64_t>(local_best_T) - NUM_BLOCKS / 2) > std::abs(static_cast<int64_t>(best_T2) - NUM_BLOCKS / 2)) {
            memcpy(best_k2, local_best_k2, 3);
            best_T2 = local_best_T;
        }
    });
    std::cout << "Выбран кандидат на третий раундовый ключ: " << toHex(best_k2, 3) << " T: " << best_T2 << std::endl;

    std::cout << "Начато восстановление второго раундового ключа (брутфорс)." << std::endl;
    std::atomic_bool found = false;
    tbb::parallel_for(tbb::blocked_range<uint16_t>(0, 256),
    [&](const tbb::blocked_range<uint16_t> &r) {
        BlockCipherX cipher;
        for (uint16_t k10 = r.begin(); k10 != r.end()  && !found; ++k10)
            for (uint16_t k11 = 0; k11 < 256  && !found; ++k11)
                for (uint16_t k12 = 0; k12 < 256 && !found; ++k12) {
                    std::array<uint8_t, 9> key = {
                        best_k0[0], best_k0[1], best_k0[2],
                        static_cast<uint8_t>(k10), static_cast<uint8_t>(k11), static_cast<uint8_t>(k12),
                        best_k2[0], best_k2[1], best_k2[2]
                    };
                    cipher.initKeySchedule(key);
                    bool correct = true;
                    for (size_t i = 0; i < TEXT_LEN; i+=3) {
                        std::array<uint8_t, 3> block = { plain_text[i], plain_text[i + 1], plain_text[i + 2] };
                        if(memcmp(cipher.encrypt(block).data(), cipher_text + i, 3)) {
                            correct = false;
                            break;
                        }
                    }
                    if (correct) {
                        found = true;
                        std::cout << "Восстановлен ключ: " << toHex(key.data(), 9) << std::endl;
                        return;
                    }
                }
    });
    if (!found) std::cout << "Не удалось восстановить ключ." << std::endl;
}