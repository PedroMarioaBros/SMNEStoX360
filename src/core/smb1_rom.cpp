#include "smb360/smb1_rom.hpp"
#include "smb360/ines.hpp"
namespace smb360 {
Smb1RomStatus validate_smb1_rom_layout(const std::uint8_t* data, std::size_t size) {
    const INesInfo i = parse_ines(data, size);
    if (!i.valid) return Smb1RomStatus::InvalidInes;
    if (i.trainer) return Smb1RomStatus::HasTrainer;
    if (i.mapper != 0) return Smb1RomStatus::WrongMapper;
    if (i.prg_banks != 2 || i.prg_size != 0x8000) return Smb1RomStatus::WrongPrgSize;
    if (i.chr_banks != 1 || i.chr_size != 0x2000) return Smb1RomStatus::WrongChrSize;
    return Smb1RomStatus::Ok;
}
const char* smb1_rom_status_string(Smb1RomStatus s) {
    switch (s) {
      case Smb1RomStatus::Ok: return "ok";
      case Smb1RomStatus::InvalidInes: return "invalid iNES image";
      case Smb1RomStatus::WrongMapper: return "SMB1 requires mapper 0";
      case Smb1RomStatus::WrongPrgSize: return "SMB1 requires 32 KiB PRG";
      case Smb1RomStatus::WrongChrSize: return "SMB1 requires 8 KiB CHR";
      case Smb1RomStatus::HasTrainer: return "SMB1 image must not contain a trainer";
    }
    return "unknown";
}
}
