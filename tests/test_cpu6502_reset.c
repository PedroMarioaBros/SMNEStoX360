#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/nrom.h"
#include "../src/canonical/cpu6502.h"

int main(void){
 uint8_t prg[SMB360_PRG_SIZE],chr[SMB360_CHR_SIZE];
 smb360_nrom bus; smb360_cpu6502 cpu;
 memset(prg,0xEA,sizeof(prg)); memset(chr,0,sizeof(chr));
 /* reset vector -> $8000; program: SEI, CLD, LDA #$10, STA $0002, LDX #$FF, TXS */
 prg[0]=0x78;prg[1]=0xD8;prg[2]=0xA9;prg[3]=0x10;prg[4]=0x8D;prg[5]=0x02;prg[6]=0x00;
 prg[7]=0xA2;prg[8]=0xFF;prg[9]=0x9A;
 prg[0x7ffc]=0x00;prg[0x7ffd]=0x80;
 smb360_nrom_init(&bus,prg,chr);smb360_cpu6502_init(&cpu,&bus);smb360_cpu6502_reset(&cpu);
 assert(cpu.pc==0x8000);assert(cpu.s==0xFD);
 for(int i=0;i<6;i++) assert(smb360_cpu6502_step(&cpu));
 assert(bus.ram[2]==0x10);assert(cpu.x==0xFF);assert(cpu.s==0xFF);
 puts("canonical 6502 reset/smoke: PASS");return 0;
}
