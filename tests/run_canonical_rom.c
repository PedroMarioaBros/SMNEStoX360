#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "../src/canonical/machine.h"

typedef struct { uint64_t frame; uint8_t buttons; } input_event;
int main(int argc,char **argv){
 FILE *f;uint8_t rom[40976];static smb360_machine m;
 uint64_t limit=1000000;unsigned used=0,i,event_count=0,next=0;
 input_event events[1024];
 const uint8_t header[16]={0x4e,0x45,0x53,0x1a,2,1,1,0,0,0,0,0,0,0,0,0};
 if(argc<2||argc>5){
  fprintf(stderr,"usage: %s ROM [instructions [frame.bin [input.txt]]]\n",argv[0]);return 2;
 }
 if(argc>=3){
  char *end;errno=0;limit=strtoull(argv[2],&end,10);
  if(errno||*end||argv[2][0]=='-'||!limit){fputs("invalid instruction limit\n",stderr);return 2;}
 }
 f=fopen(argv[1],"rb");if(!f){perror("ROM");return 3;}
 if(fread(rom,1,sizeof(rom),f)!=sizeof(rom)||fgetc(f)!=EOF){
  fclose(f);fputs("canonical ROM size mismatch\n",stderr);return 4;
 }
 fclose(f);
 if(memcmp(rom,header,sizeof(header))){fputs("canonical header mismatch\n",stderr);return 5;}
 if(argc==5){
  char line[128];f=fopen(argv[4],"r");if(!f){perror("input");return 6;}
  while(fgets(line,sizeof(line),f)){
   uint64_t frame;unsigned buttons;char extra;
   if(line[0]=='#'||line[0]=='\n')continue;
   if(event_count==1024||sscanf(line,"%" SCNu64 " %x %c",&frame,&buttons,&extra)!=2||
      buttons>255||(event_count&&frame<=events[event_count-1].frame)){
    fclose(f);fputs("invalid input events (frame buttons_hex, strictly increasing)\n",stderr);return 6;
   }
   events[event_count].frame=frame;events[event_count++].buttons=(uint8_t)buttons;
  }
  fclose(f);
 }
 /* Use tools/run_canonical.py: it verifies full ROM/PRG/CHR hashes before this process. */
 smb360_machine_init(&m,rom+16,rom+32784);
 while(m.instructions<limit){
  while(next<event_count&&m.ppu.frame>=events[next].frame){
   smb360_nrom_set_controller1(&m.bus,events[next].buttons);next++;
  }
  if(!smb360_machine_step(&m))break;
 }
 printf("instructions=%" PRIu64 " pc=$%04X stopped=$%02X cpu_cycles=%" PRIu64
        " ppu_frame=%" PRIu64 " scanline=%u dot=%u\n",
  m.instructions,m.cpu.pc,(unsigned)(m.cpu.stopped&255),m.cpu.cycles,m.ppu.frame,
  (unsigned)m.ppu.scanline,(unsigned)m.ppu.dot);
 for(i=0;i<256;i++)if(m.cpu.opcode_hits[i])used++;
 printf("unique_opcodes=%u\nopcodes:",used);
 for(i=0;i<256;i++)if(m.cpu.opcode_hits[i])printf(" %02X:%" PRIu64,i,m.cpu.opcode_hits[i]);
 printf("\nnmi_count=%" PRIu64 " first_nmi_pc=$%04X first_nmi_cycle=%" PRIu64
        " first_nmi_frame=%" PRIu64 " last_opcode_pc=$%04X sprite_zero_hits=%" PRIu64 "\n",
  m.nmi_count,m.first_nmi_pc,m.first_nmi_cycle,m.first_nmi_frame,m.cpu.opcode_pc,m.bus.sprite_zero_hits);
 if(argc>=4){
  f=fopen(argv[3],"wb");if(!f){perror("frame");return 7;}
  if(fwrite(m.bus.completed_pixels,1,sizeof(m.bus.completed_pixels),f)!=sizeof(m.bus.completed_pixels)){
   fclose(f);return 7;
  }
  if(fclose(f))return 7;
 }
 return m.cpu.stopped?1:0;
}
