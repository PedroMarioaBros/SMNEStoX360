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
static void adc(smb360_cpu6502*c,uint8_t v){uint16_t q=(uint16_t)c->a+v+((c->p&F_C)?1u:0u);uint8_t r=(uint8_t)q;c->p=(uint8_t)((c->p&~(F_C|F_V))|((q>0xffu)?F_C:0)|((~(c->a^v)&(c->a^r)&0x80u)?F_V:0));c->a=r;nz(c,r);}
static void sbc(smb360_cpu6502*c,uint8_t v){adc(c,(uint8_t)~v);}
static void branch(smb360_cpu6502*c,int take){
 int8_t d=(int8_t)imm(c); c->last_cycles=2;
 if(take){uint16_t old=c->pc;c->pc=(uint16_t)(c->pc+d);c->last_cycles+=(uint32_t)(1+((old&0xff00u)!=(c->pc&0xff00u)));}
}

void smb360_cpu6502_init(smb360_cpu6502*c,smb360_nrom*b){memset(c,0,sizeof(*c));c->bus=b;c->s=0xFD;c->p=F_U|F_I;}
void smb360_cpu6502_reset(smb360_cpu6502*c){c->s=0xFD;c->p=F_U|F_I;c->pc=rd16(c,0xFFFC);c->cycles=7;c->last_cycles=7;c->stopped=0;}

int smb360_cpu6502_step(smb360_cpu6502*c){
 uint8_t o;
 if(c->stopped)return 0;
 o=rd(c,c->pc++); c->last_cycles=2;
 switch(o){
  case 0x78:c->p|=F_I;c->last_cycles=2;break;                         /* SEI */
  case 0xD8:c->p&=(uint8_t)~F_D;c->last_cycles=2;break;              /* CLD */
  case 0xA2:c->x=imm(c);nz(c,c->x);c->last_cycles=2;break;           /* LDX # */
  case 0xA0:c->y=imm(c);nz(c,c->y);c->last_cycles=2;break;           /* LDY # */
  case 0xA9:c->a=imm(c);nz(c,c->a);c->last_cycles=2;break;
  case 0xA5:c->a=rd(c,imm(c));nz(c,c->a);c->last_cycles=3;break;       /* LDA zp */
  case 0xB5:{uint8_t z=imm(c);c->a=rd(c,(uint8_t)(z+c->x));nz(c,c->a);c->last_cycles=4;}break; /* LDA zp,X */           /* LDA # */
  case 0xAD:c->a=rd(c,absop(c));nz(c,c->a);c->last_cycles=4;break;
  case 0xBD:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);c->a=rd(c,a);nz(c,c->a);c->last_cycles=(uint32_t)(4+((b&0xff00u)!=(a&0xff00u)));}break; /* LDA abs,X */   /* LDA abs */
  case 0x85:wr(c,imm(c),c->a);c->last_cycles=3;break;                  /* STA zp */
  case 0x95:{uint8_t z=imm(c);wr(c,(uint8_t)(z+c->x),c->a);c->last_cycles=4;}break; /* STA zp,X */
  case 0x86:wr(c,imm(c),c->x);c->last_cycles=3;break;                  /* STX zp */
  case 0x84:wr(c,imm(c),c->y);c->last_cycles=3;break;                  /* STY zp */
  case 0x8D:wr(c,absop(c),c->a);c->last_cycles=4;break;              /* STA abs */
  case 0x8E:wr(c,absop(c),c->x);c->last_cycles=4;break;              /* STX abs */
  case 0x9A:c->s=c->x;c->last_cycles=2;break;                        /* TXS */
  case 0xAA:c->x=c->a;nz(c,c->x);c->last_cycles=2;break;               /* TAX */
  case 0xA8:c->y=c->a;nz(c,c->y);c->last_cycles=2;break;               /* TAY */
  case 0x8A:c->a=c->x;nz(c,c->a);c->last_cycles=2;break;               /* TXA */
  case 0x98:c->a=c->y;nz(c,c->a);c->last_cycles=2;break;               /* TYA */
  case 0xBA:c->x=c->s;nz(c,c->x);c->last_cycles=2;break;               /* TSX */
  case 0xE6:{uint8_t z=imm(c),v=(uint8_t)(rd(c,z)+1u);wr(c,z,v);nz(c,v);c->last_cycles=5;}break; /* INC zp */
  case 0xC6:{uint8_t z=imm(c),v=(uint8_t)(rd(c,z)-1u);wr(c,z,v);nz(c,v);c->last_cycles=5;}break; /* DEC zp */
  case 0xE8:c->x++;nz(c,c->x);c->last_cycles=2;break;                /* INX */
  case 0xCA:c->x--;nz(c,c->x);c->last_cycles=2;break;                /* DEX */
  case 0x88:c->y--;nz(c,c->y);c->last_cycles=2;break;                /* DEY */
  case 0xC8:c->y++;nz(c,c->y);c->last_cycles=2;break;                /* INY */
  case 0x29:c->a&=imm(c);nz(c,c->a);c->last_cycles=2;break;
  case 0x09:c->a|=imm(c);nz(c,c->a);c->last_cycles=2;break;           /* ORA # */
  case 0x49:c->a^=imm(c);nz(c,c->a);c->last_cycles=2;break;           /* EOR # */          /* AND # */
  case 0x24:{uint8_t v=rd(c,imm(c));c->p=(uint8_t)((c->p&~(F_N|F_V|F_Z))|(v&(F_N|F_V))|((c->a&v)?0:F_Z));c->last_cycles=3;}break; /* BIT zp */
  case 0x2C:{uint8_t v=rd(c,absop(c));c->p=(uint8_t)((c->p&~(F_N|F_V|F_Z))|(v&(F_N|F_V))|((c->a&v)?0:F_Z));c->last_cycles=4;}break; /* BIT abs */          /* AND # */
  case 0x69:adc(c,imm(c));c->last_cycles=2;break;                         /* ADC #; 2A03 ignores decimal arithmetic */
  case 0xE9:sbc(c,imm(c));c->last_cycles=2;break;                         /* SBC # */
  case 0xC9:{uint8_t v=imm(c),r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=2;}break;
  case 0xE0:{uint8_t v=imm(c),r=(uint8_t)(c->x-v);c->p=(uint8_t)((c->p&~F_C)|(c->x>=v?F_C:0));nz(c,r);c->last_cycles=2;}break; /* CPX # */
  case 0xC0:{uint8_t v=imm(c),r=(uint8_t)(c->y-v);c->p=(uint8_t)((c->p&~F_C)|(c->y>=v?F_C:0));nz(c,r);c->last_cycles=2;}break; /* CPY # */
  case 0xD0:branch(c,!(c->p&F_Z));break;             /* BNE */
  case 0xF0:branch(c,(c->p&F_Z));break;              /* BEQ */
  case 0x10:branch(c,!(c->p&F_N));break;             /* BPL */
  case 0x30:branch(c,(c->p&F_N));break;
  case 0xB0:branch(c,(c->p&F_C));break;              /* BCS */
  case 0x90:branch(c,!(c->p&F_C));break;             /* BCC */              /* BMI */
  case 0x91:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));wr(c,(uint16_t)(b+c->y),c->a);c->last_cycles=6;}break; /* STA (zp),Y */
  case 0x4C:c->pc=absop(c);c->last_cycles=3;break;                    /* JMP abs */
  case 0x20:{uint16_t t=absop(c),r=(uint16_t)(c->pc-1);push(c,(uint8_t)(r>>8));push(c,(uint8_t)r);c->pc=t;c->last_cycles=6;}break;
  case 0x60:{uint8_t l=pop(c),h=pop(c);c->pc=(uint16_t)(((uint16_t)h<<8)|l);c->pc++;c->last_cycles=6;}break;
  case 0x18:c->p&=(uint8_t)~F_C;c->last_cycles=2;break;                 /* CLC */
  case 0x38:c->p|=F_C;c->last_cycles=2;break;                           /* SEC */
  case 0x58:c->p&=(uint8_t)~F_I;c->last_cycles=2;break;                 /* CLI */
  case 0xB8:c->p&=(uint8_t)~F_V;c->last_cycles=2;break;                 /* CLV */
  case 0xEA:c->last_cycles=2;break;                                   /* NOP */
  default:c->stopped=o;return 0;                    /* deliberate: expose next missing opcode */
 }
 c->cycles += c->last_cycles;
 return 1;
}
