#pragma once
#include <cstddef>
#include <cstdint>
namespace smb360 {
constexpr const char* kSmb1WorldHeaderedSha1 = "33d23c2f2cfa4c9efec87f7bc1321ce3ce6c89bd";
constexpr const char* kSmb1UserBaseSha1 = "dceef7c969d7a7bf5bbc21da3e95618765b1fb1a";
constexpr const char* kSmb1PtBrBMatSantosSha1 = "8dc2faf0ae794b3c5192d4d669d07173cc53b626";

enum class Smb1KnownVariant {
    Unknown,
    World,
    UserBase,
    PtBrBMatSantos,
};

Smb1KnownVariant identify_known_smb1_rom(const std::uint8_t* data, std::size_t size);
bool is_known_smb1_world_rom(const std::uint8_t* data, std::size_t size);
bool is_known_smb1_rom(const std::uint8_t* data, std::size_t size);
const char* smb1_variant_name(Smb1KnownVariant variant);
}
