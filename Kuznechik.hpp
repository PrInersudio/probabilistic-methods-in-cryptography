#ifndef KUZNECHIK_HPP
#define KUZNECHIK_HPP

#include "Cipher.hpp"

class Kuznechik final : public Cipher<16, 32> {
private:
    std::array<uint8_t, 16> key_schedule_[10];
public:
    static constexpr std::size_t BlockSize = 16;
    static constexpr std::size_t KeySize = 32;

    Kuznechik() noexcept = default;
    void initKeySchedule(const std::array<uint8_t, 32> &key) noexcept override;
    inline Kuznechik(const std::array<uint8_t, 32> &key) noexcept
        { initKeySchedule(key); }
    std::array<uint8_t, 16> &encrypt(std::array<uint8_t, 16> &plain_text) const noexcept override;
    std::array<uint8_t, 16> &decrypt(std::array<uint8_t, 16> &encrypted_text) const noexcept override;
    #ifdef UNIT_TESTS
        const std::array<uint8_t, 16> *getKeySchedule() const noexcept;
    #endif
};

#ifdef UNIT_TESTS
    std::array<uint8_t, 16> &testSubstitute(std::array<uint8_t, 16> &vector) noexcept;
    std::array<uint8_t, 16> &testInverseSubstitute(std::array<uint8_t, 16> &vector) noexcept;
    std::array<uint8_t, 16> &testLinear(std::array<uint8_t, 16> &vector) noexcept;
    std::array<uint8_t, 16> &testInverseLinear(std::array<uint8_t, 16> &vector) noexcept;
#endif

#endif