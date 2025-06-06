#include <cinttypes>
#include <cstring>
#include "Cipher.hpp"

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

inline uint8_t *S(uint8_t *block, const uint8_t (&Sbox)[64]) noexcept {
    uint8_t v0 = block[0] >> 2, v1 = ((block[0] & 0b11) << 4) | (block[1] >> 4),
        v2 = ((block[1] & 0b1111) << 2) | (block[2] >> 6), v3 = block[2] & 0b111111;
    v0 = Sbox[v0]; v1 = Sbox[v1]; v2 = Sbox[v2]; v3 = Sbox[v3];
    block[0] = (v0 << 2) | (v1 >> 4); block[1] = (v1 << 4) | (v2 >> 2);
    block[2] = (v2 << 6) | v3;
    return block;
}

constexpr uint8_t pbox[] = { 0, 6, 12, 18, 1, 7, 13, 19, 2, 8, 14, 20, 3, 9, 15, 21, 4, 10, 16, 22, 5, 11, 17, 23 };
constexpr uint8_t invpbox[] = { 0, 4, 8, 12, 16, 20, 1, 5, 9, 13, 17, 21, 2, 6, 10, 14, 18, 22, 3, 7, 11, 15, 19, 23 };

inline uint8_t *L(uint8_t *block, const uint8_t (&Pbox)[24]) noexcept {
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

inline uint8_t *X(uint8_t *block, const uint8_t *key) noexcept {
    for (uint8_t i = 0; i < 3; ++i)
        block[i] ^= key[i];
    return block;
}

class BlockCipherX : public Cipher<3, 9> {
private:
    uint8_t key_[9];
public:
    static constexpr std::size_t BlockSize = 3;
    static constexpr std::size_t KeySize = 9;

    BlockCipherX() = default;
    inline void initKeySchedule(const std::array<uint8_t, 9> &key) noexcept
        { std::memcpy(key_, key.data(), 9); }
    inline BlockCipherX(const std::array<uint8_t, 9> &key) noexcept
        { initKeySchedule(key); }
    inline std::array<uint8_t, 3> &encrypt(std::array<uint8_t, 3> &plain_text) const noexcept override {
        X(S(X(L(S(X(plain_text.data(), key_), sbox), pbox), key_ + 3), sbox), key_ + 6);
        return plain_text;
    }
    inline std::array<uint8_t, 3> &decrypt(std::array<uint8_t, 3> &cipher_text) const noexcept override {
        X(S(L(X(S(X(cipher_text.data(), key_ + 6), invsbox), key_ + 3), invpbox), invsbox), key_);
        return cipher_text;
    }
};