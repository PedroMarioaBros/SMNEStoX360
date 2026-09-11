#include "smb360/platform/xenon/input_adapter.hpp"
#include <input/input.h>
#include <usb/usbmain.h>
namespace smb360 {
PadState xenon_poll_pad(unsigned port){
    usb_do_poll();
    controller_data_s d{};
    PadState p{};
    if(!get_controller_data(&d,static_cast<int>(port))) return p;
    if(d.up) p.buttons|=PadUp;
    if(d.down) p.buttons|=PadDown;
    if(d.left) p.buttons|=PadLeft;
    if(d.right) p.buttons|=PadRight;
    if(d.a) p.buttons|=PadA;
    if(d.b) p.buttons|=PadB;
    if(d.start) p.buttons|=PadStart;
    if(d.back) p.buttons|=PadBack;
    p.lx=d.s1_x; p.ly=d.s1_y; p.rx=d.s2_x; p.ry=d.s2_y; p.lt=d.lt; p.rt=d.rt;
    return p;
}
}
