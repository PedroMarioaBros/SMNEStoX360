#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/canonical/machine.h"

int main(int argc,char**argv){
 FILE*f;long n;uint8_t*rom;smb360_machine m;uint64_t ran;
 if(argc!=2){fprintf(stderr,"usage: %s SMB_v026.nes\n",argv[0]);return 2;}
 f=fopen(argv[1],"rb");if(!f)return 3;
 fseek(f,0,SEEK_END);n=ftell(f);rewind(f);
 if(n!=40976){fclose(f);fprintf(stderr,"canonical ROM size mismatch\n");return 4;}
 rom=(uint8_t*)malloc((size_t)n);if(!rom){fclose(f);return 5;}
 if(fread(rom,1,(size_t)n,f)!=(size_t)n){free(rom);fclose(f);return 6;}fclose(f);
 if(rom[0]!='N'||rom[1]!='E'||rom[2]!='S'||rom[3]!=0x1a){free(rom);return 7;}
 smb360_machine_init(&m,rom+16,rom+16+32768);
 ran=smb360_machine_run(&m,1000000);
 printf("instructions=%llu pc=$%04X stopped=$%02X cpu_cycles=%llu ppu_frame=%llu scanline=%u dot=%u\n",
  (unsigned long long)ran,m.cpu.pc,(unsigned)(m.cpu.stopped&255),
  (unsigned long long)m.cpu.cycles,(unsigned long long)m.ppu.frame,
  (unsigned)m.ppu.scanline,(unsigned)m.ppu.dot);
 free(rom);
 return 0;
}
