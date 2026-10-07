#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/cpu6502.h"
int main(void){
 static uint8_t prg[32768],chr[8192];
 static smb360_nrom bus;smb360_cpu6502 c;
 unsigned kind,mode,value,carry,count=0;
 const unsigned low[]={0x0a,6,0x16,0x0e,0x1e};
 const unsigned cycles[]={2,5,6,6,7};
 smb360_nrom_init(&bus,prg,chr);smb360_cpu6502_init(&c,&bus);
 for(kind=0;kind<4;kind++)for(mode=0;mode<5;mode++)
 for(value=0;value<256;value++)for(carry=0;carry<2;carry++){
  unsigned result,new_c,flags,got;
  c.pc=0x8000;c.p=(uint8_t)(F_U|F_I|F_D|F_V|carry);
  c.x=0;c.a=(uint8_t)value;c.stopped=0;
  prg[0]=(uint8_t)(kind*0x20+low[mode]);prg[1]=0x20;prg[2]=0;
  bus.ram[0x20]=(uint8_t)value;
  if(kind<2){result=(value*2+(kind==1?carry:0))&255;new_c=value/128;}
  else{result=value/2+(kind==3?carry*128:0);new_c=value%2;}
  flags=F_U|F_I|F_D|F_V|(result&128)|(result?0:F_Z)|new_c;
  assert(smb360_cpu6502_step(&c));got=mode?bus.ram[0x20]:c.a;
  assert(got==result&&c.p==flags&&c.last_cycles==cycles[mode]);count++;
 }
 printf("ASL/ROL/LSR/ROR all values/carry/address forms: %u PASS\n",count);
 return 0;
}
