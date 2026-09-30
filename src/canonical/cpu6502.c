#include "cpu6502.h"
#include <string.h>

static uint8_t rd(smb360_cpu6502*c,uint16_t a){return smb360_nrom_read(c->bus,a);}
static void wr(smb360_cpu6502*c,uint16_t a,uint8_t v){smb360_nrom_write(c->bus,a,v);}
static uint16_t rd16(smb360_cpu6502*c,uint16_t a){uint8_t l=rd(c,a);return (uint16_t)(l|((uint16_t)rd(c,a+1)<<8));}
static void nz(smb360_cpu6502*c,uint8_t v){c->p=(uint8_t)((c->p&~(F_N|F_Z))|(v?0:F_Z)|(v&F_N));}
static void push(smb360_cpu6502*c,uint8_t v){wr(c,(uint16_t)(0x100u+c->s),v);c->s--;}
static uint8_t pop(smb360_cpu6502*c){c->s++;return rd(c,(uint16_t)(0x100u+c->s));}
static uint16_t absop(smb360_cpu6502*c){uint16_t a=rd16(c,c->pc);c->pc+=2;return a;}
static uint8_t imm(smb360_cpu6502*c){return rd(c,c->pc++);}
static void branch(smb360_cpu6502*c,int take){int8_t d=(int8_t)imm(c);if(take)c->pc=(uint16_t)(c->pc+d);}

void smb360_cpu6502_init(smb360_cpu6502*c,smb360_nrom*b){memset(c,0,sizeof(*c));c->bus=b;c->s=0xFD;c->p=F_U|F_I;}
void smb360_cpu6502_reset(smb360_cpu6502*c){c->s=0xFD;c->p=F_U|F_I;c->pc=rd16(c,0xFFFC);c->cycles=7;c->stopped=0;}

int smb360_cpu6502_step(smb360_cpu6502*c){
 uint8_t o;
 if(c->stopped)return 0;
 o=rd(c,c->pc++);
 switch(o){
  case 0x78:c->p|=F_I;break;                         /* SEI */
  case 0xD8:c->p&=(uint8_t)~F_D;break;              /* CLD */
  case 0xA2:c->x=imm(c);nz(c,c->x);break;           /* LDX # */
  case 0xA0:c->y=imm(c);nz(c,c->y);break;           /* LDY # */
  case 0xA9:c->a=imm(c);nz(c,c->a);break;           /* LDA # */
  case 0xAD:c->a=rd(c,absop(c));nz(c,c->a);break;   /* LDA abs */
  case 0x8D:wr(c,absop(c),c->a);break;              /* STA abs */
  case 0x8E:wr(c,absop(c),c->x);break;              /* STX abs */
  case 0x9A:c->s=c->x;break;                        /* TXS */
  case 0xE8:c->x++;nz(c,c->x);break;                /* INX */
  case 0xCA:c->x--;nz(c,c->x);break;                /* DEX */
  case 0x88:c->y--;nz(c,c->y);break;                /* DEY */
  case 0xC8:c->y++;nz(c,c->y);break;                /* INY */
  case 0x29:c->a&=imm(c);nz(c,c->a);break;          /* AND # */
  case 0xC9:{uint8_t v=imm(c),r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);}break;
  case 0xD0:branch(c,!(c->p&F_Z));break;             /* BNE */
  case 0xF0:branch(c,(c->p&F_Z));break;              /* BEQ */
  case 0x10:branch(c,!(c->p&F_N));break;             /* BPL */
  case 0x30:branch(c,(c->p&F_N));break;              /* BMI */
  case 0x4C:c->pc=absop(c);break;                    /* JMP abs */
  case 0x20:{uint16_t t=absop(c),r=(uint16_t)(c->pc-1);push(c,(uint8_t)(r>>8));push(c,(uint8_t)r);c->pc=t;}break;
  case 0x60:{uint8_t l=pop(c),h=pop(c);c->pc=(uint16_t)(((uint16_t)h<<8)|l);c->pc++;}break;
  case 0xEA:break;                                   /* NOP */
  default:c->stopped=o;return 0;                    /* deliberate: expose next missing opcode */
 }
 c->cycles++;
 return 1;
}
