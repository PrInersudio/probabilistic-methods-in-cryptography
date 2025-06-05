#include <iostream>
#include <iomanip>
#include "BlockCipherX.hpp"
#include "CTR_DRBG.hpp"
#include "Kuznechik.hpp"

#define NUM_BLOCKS 32
#define TEXT_LEN NUM_BLOCKS * 3

static std::string toHex(const std::array<uint8_t, 9> &data) noexcept {
    std::ostringstream oss;
    for (uint8_t byte : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return oss.str();
}

int main() {
    CTR_DRBG<Kuznechik, true> rng;
    std::array<uint8_t, 9> key;
    rng(key.data(), 9);
    std::cout << "Ключ: " << toHex(key) << std::endl;
    BlockCipherX cipher(key);

    std::array<uint8_t, 3> test{0x01, 0x02, 0x03};
    std::array<uint8_t, 3> original_test = test;
    cipher.encrypt(test);
    cipher.decrypt(test);
    std::cout << (test == original_test ? "Работает" : "Не работает") << std::endl;
    
    uint8_t plain_text[TEXT_LEN];
    rng(plain_text, TEXT_LEN);
    std::ofstream plain_text_file("plain_text.bin", std::ios::binary);
    plain_text_file.write(reinterpret_cast<char *>(plain_text), TEXT_LEN);

    uint8_t cipher_text[TEXT_LEN];
    for (size_t i = 0; i < TEXT_LEN; i += 3) {
        std::array<uint8_t, 3> block{plain_text[i], plain_text[i + 1], plain_text[i + 2]};
        cipher.encrypt(block);
        cipher_text[i] = block[0]; cipher_text[i + 1] = block[1]; cipher_text[i + 2] = block[2];
    }
    std::ofstream cipher_text_file("cipher_text.bin", std::ios::binary);
    cipher_text_file.write(reinterpret_cast<char *>(cipher_text), TEXT_LEN);

    return 0;
}