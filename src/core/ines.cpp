#include "smb360/ines.hpp"
namespace smb360 {
INesInfo parse_ines(const std::uint8_t* d,std::size_t n){
  INesInfo r{}; if(!d||n<16||d[0]!='N'||d[1]!='E'||d[2]!='S'||d[3]!=0x1a) return r;
  r.prg_banks=d[4]; r.chr_banks=d[5]; r.trainer=(d[6]&0x04)!=0; r.vertical_mirroring=(d[6]&1)!=0;
  r.mapper=static_cast<std::uint16_t>((d[6]>>4)|(d[7]&0xf0));
  r.prg_offset=16+(r.trainer?512:0); r.prg_size=static_cast<std::size_t>(r.prg_banks)*16384;
  r.chr_offset=r.prg_offset+r.prg_size; r.chr_size=static_cast<std::size_t>(r.chr_banks)*8192;
  if(r.prg_offset>n||r.prg_size>n-r.prg_offset) return INesInfo{};
  if(r.chr_size && (r.chr_offset>n||r.chr_size>n-r.chr_offset)) return INesInfo{};
  r.valid=true; return r;
}
}
