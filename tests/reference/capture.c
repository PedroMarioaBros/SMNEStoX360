/* Diagnostic adapter only. binjnes is never linked into the product runtime.
 * Invoke via tools/compare_reference.py for canonical fingerprint validation.
 * Output: frames (61440 bytes each), RAM (2048 bytes each), CSV on stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#ifdef REFERENCE_BINJNES
#include "emulator.h"
#else
#include "../../src/canonical/machine.h"
#endif

static uint8_t buttons(unsigned frame, int scripted) {
 if(!scripted)return 0;
 if(frame>=100 && frame<102)return 8;
 if(frame>=220 && frame<270)return 0x81;
 if(frame>=270 && frame<360)return 0x80;
 return 0;
}
#ifdef REFERENCE_BINJNES
static uint8_t input_buttons;
static void input(SystemInput *s,void *data,bool strobe) {
 JoypadInput *j;
 (void)data;(void)strobe;
 memset(s,0,sizeof(*s));j=&s->port[0].joyp;
 j->A=(input_buttons&1)!=0;j->B=(input_buttons&2)!=0;
 j->select=(input_buttons&4)!=0;j->start=(input_buttons&8)!=0;
 j->up=(input_buttons&16)!=0;j->down=(input_buttons&32)!=0;
 j->left=(input_buttons&64)!=0;j->right=(input_buttons&128)!=0;
}
#endif
static int save(FILE *f,const void *p,size_t n){return fwrite(p,1,n,f)==n;}
int main(int argc,char **argv) {
 FILE *f,*frames,*ram;unsigned count,i;int scripted;
 uint8_t rom[40976],pixels[61440];
 if(argc!=6){fprintf(stderr,"usage: capture ROM frames.bin ram.bin count scripted\n");return 2;}
 count=(unsigned)strtoul(argv[4],NULL,10);scripted=atoi(argv[5]);
 if(!count||count>10000)return 2;
 f=fopen(argv[1],"rb");if(!f)return 3;
 if(fread(rom,1,sizeof(rom),f)!=sizeof(rom)||fgetc(f)!=EOF){fclose(f);return 3;}fclose(f);
 frames=fopen(argv[2],"wb");ram=fopen(argv[3],"wb");if(!frames||!ram)return 3;
#ifdef REFERENCE_BINJNES
 EmulatorInit init;Emulator *e;
 memset(&init,0,sizeof(init));init.rom.data=rom;init.rom.size=sizeof(rom);
 init.audio_frequency=44100;init.audio_frames=4410;init.ram_init=RAM_INIT_ZERO;
 e=emulator_new(&init);if(!e)return 4;
 emulator_set_joypad_callback(e,input,NULL);
 puts("capture,ppu_frame,master_ticks,ppu_state");
 for(i=0;i<count;i++) {
  EmulatorEvent event;unsigned k;
  input_buttons=buttons(i,scripted);
  do {
   event=emulator_run_until(e,emulator_get_ticks(e)+e->master_ticks_per_frame*2u);
   if(event&EMULATOR_EVENT_INVALID_OPCODE)return 5;
  } while(!(event&EMULATOR_EVENT_NEW_FRAME));
  for(k=0;k<sizeof(pixels);k++){
   if(e->frame_buffer[k]&~63u){fputs("emphasis present; cannot compare 6-bit frames\n",stderr);return 6;}
   pixels[k]=(uint8_t)e->frame_buffer[k];
  }
  if(!save(frames,pixels,sizeof(pixels))||!save(ram,e->s.c.ram,2048))return 7;
  printf("%u,%u,%" PRIu64 ",%u\n",i,e->s.p.frame,emulator_get_ticks(e),e->s.p.state);
 }
 emulator_delete(e);
#else
 static smb360_machine m;
 smb360_machine_init(&m,rom+16,rom+32784);
 puts("capture,ppu_frame,cpu_cycles,instructions,scanline,dot,nmi_count");
 for(i=0;i<count;i++) {
  smb360_nrom_set_controller1(&m.bus,buttons(i,scripted));
  do {
   if(!smb360_machine_step(&m)){fprintf(stderr,"stopped at $%04X\n",m.cpu.opcode_pc);return 5;}
  } while(m.ppu.frame<i || m.ppu.scanline<241 ||
          (m.ppu.scanline==241 && m.ppu.dot<1));
  memcpy(pixels,m.bus.completed_pixels,sizeof(pixels));
  if(!save(frames,pixels,sizeof(pixels))||!save(ram,m.bus.ram,2048))return 7;
  printf("%u,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%u,%u,%" PRIu64 "\n",
         i,m.ppu.frame,m.cpu.cycles,m.instructions,m.ppu.scanline,m.ppu.dot,m.nmi_count);
 }
#endif
 if(fclose(frames)||fclose(ram))return 7;
 return 0;
}
