#include "smb360/input_mapper.hpp"
namespace smb360 {
InputState map_pad_to_nes(const PadState& p){
  InputState i{};
  i.up=(p.buttons&PadUp)!=0; i.down=(p.buttons&PadDown)!=0;
  i.left=(p.buttons&PadLeft)!=0; i.right=(p.buttons&PadRight)!=0;
  i.a=(p.buttons&PadA)!=0; i.b=(p.buttons&PadB)!=0;
  i.start=(p.buttons&PadStart)!=0; i.select=(p.buttons&PadBack)!=0;
  return i;
}
}
