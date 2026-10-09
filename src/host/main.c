/* Host bench only: SDL presents pixels; gameplay is the canonical PRG. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "../canonical/machine.h"
#include "../canonical/audio_pcm.h"
#include "canonical_assets.h"

static smb360_machine machine;

/* SDL queued playback and WAV capture consume the same PCM stream. */
typedef struct {
 SDL_AudioDeviceID device;
 FILE *wav;
 int16_t buffer[2048];
 unsigned buffered;
 uint64_t written;
 int failed;
} host_audio;
static void audio_flush(host_audio *a){
 if(!a->buffered || a->failed)return;
 if(a->wav){
  uint8_t bytes[4096];
  for(unsigned i=0;i<a->buffered;i++){
   uint16_t v=(uint16_t)a->buffer[i];
   bytes[i*2]=(uint8_t)v;
   bytes[i*2+1]=(uint8_t)(v>>8);
  }
  if(fwrite(bytes,2,a->buffered,a->wav)!=a->buffered)a->failed=1;
 }
 if(a->device && !a->failed){
  if(SDL_GetQueuedAudioSize(a->device)>SMB360_PCM_RATE*2u/3u)
   SDL_ClearQueuedAudio(a->device);
  if(SDL_QueueAudio(a->device,a->buffer,a->buffered*sizeof(a->buffer[0])))
   a->failed=1;
 }
 a->written+=a->buffered;
 a->buffered=0;
}
static void audio_sink(int16_t sample,void *opaque){
 host_audio *a=(host_audio*)opaque;
 a->buffer[a->buffered++]=sample;
 if(a->buffered==2048u)audio_flush(a);
}
static void wav_u16(FILE*f,uint16_t v){
 fputc((int)(v&255u),f);fputc((int)(v>>8),f);
}
static void wav_u32(FILE*f,uint32_t v){
 wav_u16(f,(uint16_t)v);wav_u16(f,(uint16_t)(v>>16));
}
static int wav_finish(host_audio *a){
 uint32_t data;
 if(!a->wav)return 0;
 if(a->written>0x7fffff00u)return -1;
 data=(uint32_t)(a->written*2u);
 if(fseek(a->wav,0,SEEK_SET))return -1;
 if(fwrite("RIFF",1,4,a->wav)!=4)return -1;
 wav_u32(a->wav,data+36u);
 if(fwrite("WAVEfmt ",1,8,a->wav)!=8)return -1;
 wav_u32(a->wav,16u);wav_u16(a->wav,1u);wav_u16(a->wav,1u);
 wav_u32(a->wav,SMB360_PCM_RATE);wav_u32(a->wav,SMB360_PCM_RATE*2u);
 wav_u16(a->wav,2u);wav_u16(a->wav,16u);
 if(fwrite("data",1,4,a->wav)!=4)return -1;
 wav_u32(a->wav,data);
 return ferror(a->wav)?-1:0;
}

static uint8_t held[SDL_NUM_SCANCODES];
static SDL_GameController *pads[2];
static const SDL_Scancode keys[2][8]={
 {SDL_SCANCODE_Z,SDL_SCANCODE_X,SDL_SCANCODE_RSHIFT,SDL_SCANCODE_RETURN,
  SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT},
 {SDL_SCANCODE_G,SDL_SCANCODE_F,SDL_SCANCODE_TAB,SDL_SCANCODE_SPACE,
  SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_A,SDL_SCANCODE_D}};
static const SDL_GameControllerButton buttons[8]={
 SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_BACK,
 SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,
 SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT};

static void connect_pad(int index){
 unsigned p;
 if(!SDL_IsGameController(index))return;
 for(p=0;p<2;p++)if(pads[p] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[p]))==
   SDL_JoystickGetDeviceInstanceID(index))return;
 for(p=0;p<2;p++)if(!pads[p]){pads[p]=SDL_GameControllerOpen(index);break;}
}
static uint8_t input(unsigned port,int focused){
 unsigned i;uint8_t value=0;
 if(!focused)return 0;
 for(i=0;i<8;i++)if(held[keys[port][i]] ||
   (pads[port] && SDL_GameControllerGetButton(pads[port],buttons[i])))value|=(uint8_t)(1u<<i);
 /* Cancel opposing directions to keep keyboard chords deterministic. */
 if((value&0x30)==0x30)value&=(uint8_t)~0x30;
 if((value&0xc0)==0xc0)value&=(uint8_t)~0xc0;
 return value;
}
static void push_key(SDL_Scancode key,int down){
 SDL_Event e;memset(&e,0,sizeof(e));e.type=down?SDL_KEYDOWN:SDL_KEYUP;
 e.key.keysym.scancode=key;
 if(SDL_PushEvent(&e)<0){fprintf(stderr,"event injection: %s\n",SDL_GetError());exit(1);}
}
static void smoke_events(unsigned frame){
 if(frame==100)push_key(SDL_SCANCODE_RETURN,1);
 if(frame==102)push_key(SDL_SCANCODE_RETURN,0);
 if(frame==220){push_key(SDL_SCANCODE_Z,1);push_key(SDL_SCANCODE_RIGHT,1);}
 if(frame==270)push_key(SDL_SCANCODE_Z,0);
 if(frame==360)push_key(SDL_SCANCODE_RIGHT,0);
}
int main(int argc,char **argv){
 SDL_Window *window=NULL;SDL_Renderer *renderer=NULL;SDL_Texture *texture=NULL;
 uint32_t rgba[256*240];unsigned frames=0,limit=0,i,unique=0;
 int smoke=0,running=1,focused=1,paused=0,result=1;
 smb360_pcm pcm;host_audio audio={0};
 const char *dump=NULL,*wav_path=NULL;double next,frequency;uint64_t keyboard_transitions=0;
 for(int a=1;a<argc;a++){
  if(!strcmp(argv[a],"--smoke")){smoke=1;limit=600;}
  else if(!strcmp(argv[a],"--dump") && a+1<argc)dump=argv[++a];
  else if(!strcmp(argv[a],"--wav") && a+1<argc)wav_path=argv[++a];
  else {fprintf(stderr,"usage: %s [--smoke] [--dump frame.bin] [--wav audio.wav]\n"
   "P1: arrows, Z jump, X run/fire, Enter Start, right Shift Select\n"
   "P2: WASD, G jump, F run/fire, Space Start, Tab Select\n"
   "Gamepad: d-pad, A jump, X run/fire, Start/Back. P pause; Escape quit.\n",argv[0]);
   return !strcmp(argv[a],"--help")?0:2;}
 }
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_TIMER))goto done;
 SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
 window=SDL_CreateWindow("SMB_v026 — bancada host",SDL_WINDOWPOS_CENTERED,
   SDL_WINDOWPOS_CENTERED,768,720,SDL_WINDOW_RESIZABLE);
 if(!window)goto done;
 renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
 if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
 if(!renderer || SDL_RenderSetLogicalSize(renderer,256,240))goto done;
 texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,256,240);
 if(!texture)goto done;
 for(int j=0;j<SDL_NumJoysticks();j++)connect_pad(j);
 smb360_machine_init(&machine,canonical_prg,canonical_chr);
 smb360_pcm_init(&pcm);
 if(wav_path){
  const uint8_t header[44]={0};
  audio.wav=fopen(wav_path,"wb");
  if(!audio.wav){perror(wav_path);goto done;}
  if(fwrite(header,1,sizeof(header),audio.wav)!=sizeof(header))goto done;
 }
 if(!smoke && SDL_InitSubSystem(SDL_INIT_AUDIO)==0){
  SDL_AudioSpec desired;SDL_zero(desired);
  desired.freq=SMB360_PCM_RATE;desired.format=AUDIO_S16SYS;
  desired.channels=1;desired.samples=2048;
  audio.device=SDL_OpenAudioDevice(NULL,0,&desired,NULL,0);
  if(audio.device)SDL_PauseAudioDevice(audio.device,0);
 }
 frequency=(double)SDL_GetPerformanceFrequency();next=(double)SDL_GetPerformanceCounter();
 while(running && (!limit || frames<limit)){
  SDL_Event e;
  if(smoke)smoke_events(frames);
  while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT)running=0;
   else if(e.type==SDL_KEYDOWN || e.type==SDL_KEYUP){
    SDL_Scancode key=e.key.keysym.scancode;
    if(key>=0 && key<SDL_NUM_SCANCODES){held[key]=(e.type==SDL_KEYDOWN);keyboard_transitions++;}
    if(e.type==SDL_KEYDOWN && !e.key.repeat){
     if(key==SDL_SCANCODE_ESCAPE)running=0;
     if(key==SDL_SCANCODE_P)paused=!paused;
    }
   }else if(e.type==SDL_WINDOWEVENT){
    if(e.window.event==SDL_WINDOWEVENT_FOCUS_LOST){focused=0;memset(held,0,sizeof(held));}
    if(e.window.event==SDL_WINDOWEVENT_FOCUS_GAINED)focused=1;
   }else if(e.type==SDL_CONTROLLERDEVICEADDED)connect_pad(e.cdevice.which);
   else if(e.type==SDL_CONTROLLERDEVICEREMOVED){
    for(i=0;i<2;i++)if(pads[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i]))==e.cdevice.which){
     SDL_GameControllerClose(pads[i]);pads[i]=NULL;
    }
   }
  }
  if(!running)break;
  if(paused || (!focused && !smoke)){SDL_Delay(20);next=(double)SDL_GetPerformanceCounter();continue;}
  smb360_nrom_set_controller1(&machine.bus,input(0,focused));
  smb360_nrom_set_controller2(&machine.bus,input(1,focused));
  do {
   uint64_t previous=machine.bus.apu.cpu_cycles;
   if(!smb360_machine_step(&machine)){
    fprintf(stderr,"unsupported opcode at $%04X: $%02X\n",machine.cpu.opcode_pc,machine.cpu.stopped&255);goto done;
   }
   if(audio.wav || audio.device){
    uint64_t elapsed=machine.bus.apu.cpu_cycles-previous;
    smb360_pcm_advance(&pcm,&machine.bus.apu,(uint32_t)elapsed,audio_sink,&audio);
    if(audio.failed)goto done;
   }
  }while(machine.ppu.frame<frames || machine.ppu.scanline<241 ||
         (machine.ppu.scanline==241 && machine.ppu.dot<1));
  audio_flush(&audio);if(audio.failed)goto done;
  for(i=0;i<256*240;i++)rgba[i]=host_palette[machine.bus.completed_pixels[i]&63];
  if(SDL_UpdateTexture(texture,NULL,rgba,256*(int)sizeof(uint32_t)) ||
     SDL_RenderClear(renderer) || SDL_RenderCopy(renderer,texture,NULL,NULL))goto done;
  if(smoke && frames==599){
   int w,h;SDL_Surface *surface;
   if(SDL_GetRendererOutputSize(renderer,&w,&h) || w!=768 || h!=720)goto done;
   surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888);
   if(!surface)goto done;
   if(SDL_RenderReadPixels(renderer,NULL,SDL_PIXELFORMAT_ARGB8888,surface->pixels,surface->pitch)){
    SDL_FreeSurface(surface);goto done;
   }
   for(i=0;i<61440;i++){
    uint32_t *row=(uint32_t *)((uint8_t *)surface->pixels+((i/256)*3+1)*surface->pitch);
    if(row[(i%256)*3+1]!=rgba[i]){fputs("SDL readback mismatch\n",stderr);SDL_FreeSurface(surface);goto done;}
   }
   SDL_FreeSurface(surface);puts("SDL rendered pixel readback: 61440 PASS");
  }
  SDL_RenderPresent(renderer);frames++;
  if(!smoke){
   double now,remaining;next+=frequency/60.0988;now=(double)SDL_GetPerformanceCounter();
   remaining=(next-now)*1000.0/frequency;
   if(remaining>=1.0)SDL_Delay((Uint32)remaining);
   if(remaining< -250.0)next=now;
  }
 }
 if(dump){
  FILE *f=fopen(dump,"wb");if(!f){perror(dump);goto done;}
  if(fwrite(machine.bus.completed_pixels,1,61440,f)!=61440){fclose(f);goto done;}
  if(fclose(f))goto done;
 }
 for(i=0;i<256;i++)if(machine.cpu.opcode_hits[i])unique++;
 printf("frames=%u instructions=%" PRIu64 " cpu_cycles=%" PRIu64 " nmi=%" PRIu64
        " unique_opcodes=%u keyboard_transitions=%" PRIu64 "\n",
        frames,machine.instructions,machine.cpu.cycles,machine.nmi_count,unique,keyboard_transitions);
 printf("opcodes:");
 for(i=0;i<256;i++)if(machine.cpu.opcode_hits[i])printf(" %02X:%" PRIu64,i,machine.cpu.opcode_hits[i]);
 printf("\npc=$%04X ppu_frame=%" PRIu64 " scanline=%u dot=%u first_nmi_pc=$%04X first_nmi_cycle=%" PRIu64 "\n",
   machine.cpu.pc,machine.ppu.frame,machine.ppu.scanline,machine.ppu.dot,machine.first_nmi_pc,machine.first_nmi_cycle);
 if(wav_finish(&audio))goto done;
 if(audio.wav){
  if(fclose(audio.wav)){audio.wav=NULL;goto done;}
  audio.wav=NULL;
  printf("audio_samples=%" PRIu64 " rate=%u bits=16 channels=1\n",
         pcm.samples,SMB360_PCM_RATE);
 }
 result=(smoke && (frames!=600 || keyboard_transitions!=6))?1:0;
 done:
 if(result)fprintf(stderr,"host failed: %s\n",SDL_GetError());
 if(audio.wav)fclose(audio.wav);
 if(audio.device)SDL_CloseAudioDevice(audio.device);
 for(i=0;i<2;i++)if(pads[i])SDL_GameControllerClose(pads[i]);
 if(texture)SDL_DestroyTexture(texture);
 if(renderer)SDL_DestroyRenderer(renderer);
 if(window)SDL_DestroyWindow(window);
 SDL_Quit();return result;
}
