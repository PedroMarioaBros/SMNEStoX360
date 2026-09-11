#pragma once
#include <cstddef>
#include <cstdint>
namespace smb360 {
struct INesInfo {
  bool valid=false;
  std::uint8_t prg_banks=0, chr_banks=0;
  std::uint16_t mapper=0;
  bool trainer=false;
  bool vertical_mirroring=false;
  std::size_t prg_offset=0, prg_size=0, chr_offset=0, chr_size=0;
};
INesInfo parse_ines(const std::uint8_t* data, std::size_t size);
}
