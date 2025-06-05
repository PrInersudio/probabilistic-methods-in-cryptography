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

constexpr uint8_t pbox[] = { 16, 13, 14, 17, 12, 15, 9, 7, 11, 6, 10, 8, 19, 22, 23, 20, 21, 18, 1, 3, 0, 4, 2, 5 };
constexpr uint8_t invpbox[] = { 20, 18, 22, 19, 21, 23, 9, 7, 11, 6, 10, 8, 4, 1, 2, 5, 0, 3, 17, 12, 15, 16, 13, 14 };

inline std::array<uint8_t, 3> &S(std::array<uint8_t, 3> &block, const uint8_t (&Sbox)[64]) noexcept {
    uint8_t v0 = block[0] >> 2, v1 = ((block[0] & 0b11) << 4) | (block[1] >> 4),
        v2 = ((block[1] & 0b1111) << 2) | (block[2] >> 6), v3 = block[2] & 0b111111;
    v0 = Sbox[v0]; v1 = Sbox[v1]; v2 = Sbox[v2]; v3 = Sbox[v3];
    block[0] = (v0 << 2) | (v1 >> 4); block[1] = (v1 << 4) | (v2 >> 2);
    block[2] = (v2 << 6) | v3;
    return block;
}

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

class BlockCipherX : public Cipher<3, 9> {
private:
    uint8_t key_[9];

    inline std::array<uint8_t, 3> &X(std::array<uint8_t, 3> &block, uint8_t round) const noexcept {
        for (uint8_t i = 0; i < 3; ++i)
            block[i] ^= key_[round * 3 + i];
        return block;
    }
public:
    static constexpr std::size_t BlockSize = 3;
    static constexpr std::size_t KeySize = 9;

    BlockCipherX() = default;
    inline void initKeySchedule(const std::array<uint8_t, 9> &key) noexcept
        { std::memcpy(key_, key.data(), 9); }
    inline BlockCipherX(const std::array<uint8_t, 9> &key) noexcept
        { initKeySchedule(key); }
    inline std::array<uint8_t, 3> &encrypt(std::array<uint8_t, 3> &plain_text) const noexcept override {
        for (const uint8_t round : {uint8_t(0), uint8_t(1)})
            L(S(X(plain_text, round), sbox), pbox);
        X(plain_text, uint8_t(2));
        return plain_text;
    }
    inline std::array<uint8_t, 3> &decrypt(std::array<uint8_t, 3> &cipher_text) const noexcept override {
        for (const uint8_t round : {uint8_t(2), uint8_t(1)})
            S(L(X(cipher_text, round), invpbox), invsbox);
        X(cipher_text, uint8_t(0));
        return cipher_text;
    }
};