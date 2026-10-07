#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/machine.h"
static uint8_t prg[SMB360_PRG_SIZE], chr[SMB360_CHR_SIZE];
static smb360_machine m;
static void setup(void) {
 memset(prg,0xea,sizeof(prg)); memset(chr,0,sizeof(chr));
 prg[0x7ffc]=0;prg[0x7ffd]=0x80;
 prg[0x7ffa]=0x82;prg[0x7ffb]=0x80;
 prg[0x7ffe]=0;prg[0x7fff]=0x90;
 smb360_machine_init(&m,prg,chr);
}
static void step(void){assert(smb360_cpu6502_step(&m.cpu));}
static void arithmetic(void) {
 unsigned op,a,b,carry,decimal; unsigned long cases=0;
 setup();
 for(op=0;op<2;op++)for(a=0;a<256;a++)for(b=0;b<256;b++)
 for(carry=0;carry<2;carry++)for(decimal=0;decimal<2;decimal++){
  int result=op ? (int)a-(int)b-(1-(int)carry) : (int)a+(int)b+(int)carry;
  int signed_a=a<128 ? (int)a : (int)a-256;
  int signed_b=b<128 ? (int)b : (int)b-256;
  int signed_result=op ? signed_a-signed_b-(1-(int)carry) : signed_a+signed_b+(int)carry;
  unsigned v=(unsigned)result&255;
  unsigned flags=(v==0?F_Z:0)|(v&F_N)|
   ((op ? result>=0 : result>255)?F_C:0)|
   ((signed_result < -128 || signed_result > 127)?F_V:0);
  prg[0]=op?0xe9:0x69;prg[1]=(uint8_t)b;m.cpu.pc=0x8000;
  m.cpu.a=(uint8_t)a;m.cpu.p=(uint8_t)(F_U|F_I|carry|(decimal?F_D:0));
  step();
  assert(m.cpu.a==v);assert((m.cpu.p&(F_C|F_V|F_N|F_Z))==flags);
  assert((m.cpu.p&F_D)==(decimal?F_D:0));assert(m.cpu.last_cycles==2);cases++;
 }
 printf("ADC/SBC exhaustive binary+decimal-flag cases: %lu PASS\n",cases);
}
static void addressing(void){
 setup(); prg[0]=0xb5;prg[1]=0xff;m.cpu.x=2;m.bus.ram[1]=0x80;step();
 assert(m.cpu.a==0x80&&m.cpu.last_cycles==4);
 setup();prg[0]=0xa1;prg[1]=0xfe;m.cpu.x=1;
 m.bus.ram[0xff]=0x34;m.bus.ram[0]=0x02;m.bus.ram[0x234]=0x42;step();
 assert(m.cpu.a==0x42&&m.cpu.last_cycles==6);
 setup();prg[0]=0xb1;prg[1]=0xff;m.cpu.y=1;
 m.bus.ram[0xff]=0xff;m.bus.ram[0]=0x02;m.bus.ram[0x300]=0x55;step();
 assert(m.cpu.a==0x55&&m.cpu.last_cycles==6);
 setup();prg[0]=0xbd;prg[1]=0xff;prg[2]=2;m.cpu.x=1;m.bus.ram[0x300]=0xab;step();
 assert(m.cpu.a==0xab&&m.cpu.last_cycles==5);
 setup();prg[0]=0x99;prg[1]=0xff;prg[2]=2;m.cpu.y=1;m.cpu.a=0x77;step();
 assert(m.bus.ram[0x300]==0x77&&m.cpu.last_cycles==5);
 setup();prg[0]=0x6c;prg[1]=0xff;prg[2]=2;
 m.bus.ram[0x2ff]=0x34;m.bus.ram[0x200]=0x81;m.bus.ram[0x300]=0x99;step();
 assert(m.cpu.pc==0x8134&&m.cpu.last_cycles==5);
 setup();m.cpu.pc=0x80fd;prg[0xfd]=0xd0;prg[0xfe]=1;step();
 assert(m.cpu.pc==0x8100&&m.cpu.last_cycles==4);
 setup();prg[0]=0xd0;prg[1]=0xfe;m.cpu.p|=F_Z;step();
 assert(m.cpu.pc==0x8002&&m.cpu.last_cycles==2);
}
static void stack_interrupts(void){
 setup();prg[0]=0x20;prg[1]=0x10;prg[2]=0x80;prg[0x10]=0x60;step();
 assert(m.cpu.pc==0x8010&&m.cpu.s==0xfb);
 assert(m.bus.ram[0x1fd]==0x80&&m.bus.ram[0x1fc]==2);step();
 assert(m.cpu.pc==0x8003&&m.cpu.s==0xfd);
 setup();prg[0]=0x00;prg[0x1000]=0x40;m.cpu.p=F_U|F_D|F_C;step();
 assert(m.cpu.pc==0x9000&&m.cpu.last_cycles==7);
 assert(m.bus.ram[0x1fc]==2);
 assert(m.bus.ram[0x1fb]==(F_U|F_B|F_D|F_C));step();
 assert(m.cpu.pc==0x8002&&m.cpu.p==(F_U|F_D|F_C)&&m.cpu.last_cycles==6);
 setup();m.cpu.pc=0x8123;m.cpu.p=F_U|F_I|F_C;prg[0x82]=0x40;
 smb360_cpu6502_nmi(&m.cpu);
 assert(m.cpu.pc==0x8082&&m.cpu.cycles==14&&m.cpu.s==0xfa);
 assert(m.bus.ram[0x1fd]==0x81&&m.bus.ram[0x1fc]==0x23);
 assert(m.bus.ram[0x1fb]==(F_U|F_I|F_C));step();assert(m.cpu.pc==0x8123);
 assert(!smb360_cpu6502_irq(&m.cpu));
 m.cpu.p&=(uint8_t)~F_I;assert(smb360_cpu6502_irq(&m.cpu));assert(m.cpu.pc==0x9000);
 setup();m.cpu.s=0;prg[0]=0x48;prg[1]=0x68;m.cpu.a=0x80;step();
 assert(m.cpu.s==255&&m.bus.ram[0x100]==0x80);m.cpu.a=0;step();
 assert(m.cpu.a==0x80&&m.cpu.s==0&&(m.cpu.p&F_N));
}
static void nmi_edges(void){
 setup();assert(m.ppu.dot==21);prg[0x82]=0x40;
 smb360_nrom_set_vblank(&m.bus,1);assert(!m.bus.nmi_pending);
 smb360_nrom_write(&m.bus,0x2000,0x80);assert(m.bus.nmi_pending);
 assert(smb360_machine_step(&m));assert(m.nmi_count==1);
 assert(m.first_nmi_pc==0x8082&&m.cpu.pc==0x8000&&m.instructions==1);
 assert(!m.bus.nmi_pending);
 smb360_nrom_write(&m.bus,0x2000,0x80);assert(!m.bus.nmi_pending);
 smb360_nrom_write(&m.bus,0x2000,0);smb360_nrom_write(&m.bus,0x2000,0x80);
 assert(m.bus.nmi_pending);
 m.bus.nmi_pending=0;(void)smb360_nrom_read(&m.bus,0x2002);
 assert(!m.bus.nmi_line);
 smb360_nrom_set_vblank(&m.bus,1);assert(m.bus.nmi_pending);
}
static void illegal(void){
 setup();prg[0]=2;assert(!smb360_cpu6502_step(&m.cpu));
 assert(m.cpu.stopped==2&&m.cpu.opcode_pc==0x8000&&m.cpu.cycles==7);
 assert(!smb360_cpu6502_step(&m.cpu));assert(m.cpu.opcode_hits[2]==1);
}
int main(void){arithmetic();addressing();stack_interrupts();nmi_edges();illegal();puts("CPU addressing/stack/interrupts/stop: PASS");return 0;}
