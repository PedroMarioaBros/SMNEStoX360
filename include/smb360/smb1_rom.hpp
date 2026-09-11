#pragma once
#include <cstddef>
#include <cstdint>
namespace smb360 {
enum class Smb1RomStatus { Ok, InvalidInes, WrongMapper, WrongPrgSize, WrongChrSize, HasTrainer };
Smb1RomStatus validate_smb1_rom_layout(const std::uint8_t* data, std::size_t size);
const char* smb1_rom_status_string(Smb1RomStatus status);
}
