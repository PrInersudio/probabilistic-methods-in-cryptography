#ifndef CIPHER_HPP
#define CIPHER_HPP

#include <array>
#include <cinttypes>

template <std::size_t BlockSize, std::size_t KeySize>
class Cipher {
public:
    virtual void initKeySchedule(const std::array<uint8_t, KeySize> &key) = 0;
    virtual std::array<uint8_t, BlockSize> &encrypt(std::array<uint8_t, BlockSize> &) const = 0;
    virtual std::array<uint8_t, BlockSize> &decrypt(std::array<uint8_t, BlockSize> &) const = 0;
    virtual ~Cipher() = default;
};

template <typename T>
concept IsCipher = requires(const std::array<uint8_t, T::KeySize> &key) {
    { T() };
    { T(key) };
    { T::BlockSize } -> std::same_as<const std::size_t &>;
    { T::KeySize } -> std::same_as<const std::size_t &>;
    requires std::is_base_of_v<Cipher<T::BlockSize, T::KeySize>, T>;
    
};

#endif