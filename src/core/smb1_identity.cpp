#include "smb360/smb1_identity.hpp"
#include "smb360/sha1.hpp"
namespace smb360 {
Smb1KnownVariant identify_known_smb1_rom(const std::uint8_t* data, std::size_t size) {
    if (!data || size != 40976u) return Smb1KnownVariant::Unknown;
    const std::string digest = sha1_hex(data, size);
    if (digest == kSmb1WorldHeaderedSha1) return Smb1KnownVariant::World;
    if (digest == kSmb1UserBaseSha1) return Smb1KnownVariant::UserBase;
    if (digest == kSmb1PtBrBMatSantosSha1) return Smb1KnownVariant::PtBrBMatSantos;
    return Smb1KnownVariant::Unknown;
}

bool is_known_smb1_world_rom(const std::uint8_t* data, std::size_t size) {
    return identify_known_smb1_rom(data, size) == Smb1KnownVariant::World;
}

bool is_known_smb1_rom(const std::uint8_t* data, std::size_t size) {
    return identify_known_smb1_rom(data, size) != Smb1KnownVariant::Unknown;
}

const char* smb1_variant_name(Smb1KnownVariant variant) {
    switch (variant) {
        case Smb1KnownVariant::World: return "SMB1 World";
        case Smb1KnownVariant::UserBase: return "SMB_v026 user-supplied base";
        case Smb1KnownVariant::PtBrBMatSantos: return "SMB1 PT-BR 1.0 BMatSantos";
        case Smb1KnownVariant::Unknown: break;
    }
    return "unknown";
}
}
