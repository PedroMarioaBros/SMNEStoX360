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
 c->opcode_pc=c->pc; o=rd(c,c->pc++); c->opcode_hits[o]++; c->last_cycles=2;
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
  case 0xE6:{uint8_t z=imm(c),v=rd(c,z);wr(c,z,v);v++;wr(c,z,v);nz(c,v);c->last_cycles=5;}break; /* INC zp */
  case 0xC6:{uint8_t z=imm(c),v=rd(c,z);wr(c,z,v);v--;wr(c,z,v);nz(c,v);c->last_cycles=5;}break; /* DEC zp */
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
  case 0x01:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=6;}break;
  case 0x05:{uint16_t a=imm(c);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=3;}break;
  case 0x0D:{uint16_t a=absop(c);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x11:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x15:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x19:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x1D:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);c->a|=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x21:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=6;}break;
  case 0x25:{uint16_t a=imm(c);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=3;}break;
  case 0x2D:{uint16_t a=absop(c);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x31:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x35:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x39:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x3D:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);c->a&=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x41:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=6;}break;
  case 0x45:{uint16_t a=imm(c);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=3;}break;
  case 0x4D:{uint16_t a=absop(c);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x51:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x55:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=4;}break;
  case 0x59:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x5D:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);c->a^=v;nz(c,c->a);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x61:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);adc(c,v);c->last_cycles=6;}break;
  case 0x65:{uint16_t a=imm(c);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=3;}break;
  case 0x6D:{uint16_t a=absop(c);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=4;}break;
  case 0x71:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x75:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=4;}break;
  case 0x79:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x7D:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);adc(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xA1:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);c->a=v;nz(c,v);c->last_cycles=6;}break;
  case 0xB1:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a=v;nz(c,v);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xB9:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->a=v;nz(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xC1:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=6;}break;
  case 0xC5:{uint16_t a=imm(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=3;}break;
  case 0xCD:{uint16_t a=absop(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=4;}break;
  case 0xD1:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xD5:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=4;}break;
  case 0xD9:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xDD:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->a-v);c->p=(uint8_t)((c->p&~F_C)|(c->a>=v?F_C:0));nz(c,r);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xE1:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=6;}break;
  case 0xE5:{uint16_t a=imm(c);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=3;}break;
  case 0xED:{uint16_t a=absop(c);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=4;}break;
  case 0xF1:{uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=5+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xF5:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=4;}break;
  case 0xF9:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xFD:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);sbc(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0x81:{uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));wr(c,a,c->a);c->last_cycles=6;}break;
  case 0x99:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);wr(c,a,c->a);c->last_cycles=5;}break;
  case 0x9D:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);wr(c,a,c->a);c->last_cycles=5;}break;
  case 0x94:{uint16_t a=(uint8_t)(imm(c)+c->x);wr(c,a,c->y);c->last_cycles=4;}break;
  case 0x8C:{uint16_t a=absop(c);wr(c,a,c->y);c->last_cycles=4;}break;
  case 0x96:{uint16_t a=(uint8_t)(imm(c)+c->y);wr(c,a,c->x);c->last_cycles=4;}break;
  case 0xA6:{uint16_t a=imm(c);uint8_t v=rd(c,a);c->x=v;nz(c,v);c->last_cycles=3;}break;
  case 0xB6:{uint16_t a=(uint8_t)(imm(c)+c->y);uint8_t v=rd(c,a);c->x=v;nz(c,v);c->last_cycles=4;}break;
  case 0xAE:{uint16_t a=absop(c);uint8_t v=rd(c,a);c->x=v;nz(c,v);c->last_cycles=4;}break;
  case 0xBE:{uint16_t b=absop(c),a=(uint16_t)(b+c->y);uint8_t v=rd(c,a);c->x=v;nz(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xA4:{uint16_t a=imm(c);uint8_t v=rd(c,a);c->y=v;nz(c,v);c->last_cycles=3;}break;
  case 0xB4:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);c->y=v;nz(c,v);c->last_cycles=4;}break;
  case 0xAC:{uint16_t a=absop(c);uint8_t v=rd(c,a);c->y=v;nz(c,v);c->last_cycles=4;}break;
  case 0xBC:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);c->y=v;nz(c,v);c->last_cycles=4+((b&0xff00u)!=(a&0xff00u));}break;
  case 0xE4:{uint16_t a=imm(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->x-v);c->p=(uint8_t)((c->p&~F_C)|(c->x>=v?F_C:0));nz(c,r);c->last_cycles=3;}break;
  case 0xEC:{uint16_t a=absop(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->x-v);c->p=(uint8_t)((c->p&~F_C)|(c->x>=v?F_C:0));nz(c,r);c->last_cycles=4;}break;
  case 0xC4:{uint16_t a=imm(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->y-v);c->p=(uint8_t)((c->p&~F_C)|(c->y>=v?F_C:0));nz(c,r);c->last_cycles=3;}break;
  case 0xCC:{uint16_t a=absop(c);uint8_t v=rd(c,a);uint8_t r=(uint8_t)(c->y-v);c->p=(uint8_t)((c->p&~F_C)|(c->y>=v?F_C:0));nz(c,r);c->last_cycles=4;}break;
  case 0x06:{uint16_t a=imm(c);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1));wr(c,a,v);nz(c,v);c->last_cycles=5;}break;
  case 0x16:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x0E:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x1E:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1));wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0x0A:{uint8_t v=c->a;c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1));c->a=v;nz(c,v);}break;
  case 0x26:{uint16_t a=imm(c);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)|carry);wr(c,a,v);nz(c,v);c->last_cycles=5;}break;
  case 0x36:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)|carry);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x2E:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)|carry);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x3E:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)|carry);wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0x2A:{uint8_t v=c->a;uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)|carry);c->a=v;nz(c,v);}break;
  case 0x46:{uint16_t a=imm(c);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1));wr(c,a,v);nz(c,v);c->last_cycles=5;}break;
  case 0x56:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x4E:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x5E:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1));wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0x4A:{uint8_t v=c->a;c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1));c->a=v;nz(c,v);}break;
  case 0x66:{uint16_t a=imm(c);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)|(carry<<7));wr(c,a,v);nz(c,v);c->last_cycles=5;}break;
  case 0x76:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)|(carry<<7));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x6E:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)|(carry<<7));wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0x7E:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)|(carry<<7));wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0x6A:{uint8_t v=c->a;uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)|(carry<<7));c->a=v;nz(c,v);}break;
  case 0xD6:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v-1);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0xCE:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v-1);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0xDE:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v-1);wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0xF6:{uint16_t a=(uint8_t)(imm(c)+c->x);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v+1);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0xEE:{uint16_t a=absop(c);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v+1);wr(c,a,v);nz(c,v);c->last_cycles=6;}break;
  case 0xFE:{uint16_t b=absop(c),a=(uint16_t)(b+c->x);uint8_t v=rd(c,a);wr(c,a,v);v=(uint8_t)(v+1);wr(c,a,v);nz(c,v);c->last_cycles=7;}break;
  case 0x50:{branch(c,!(c->p&F_V));}break;
  case 0x70:{branch(c,c->p&F_V);}break;
  case 0xF8:{c->p|=F_D;}break;
  case 0x48:{push(c,c->a);c->last_cycles=3;}break;
  case 0x08:{push(c,c->p|F_B|F_U);c->last_cycles=3;}break;
  case 0x68:{c->a=pop(c);nz(c,c->a);c->last_cycles=4;}break;
  case 0x28:{c->p=(uint8_t)((pop(c)&~F_B)|F_U);c->last_cycles=4;}break;
  case 0x6C:{uint16_t a=absop(c);uint8_t l=rd(c,a);c->pc=(uint16_t)(l|((uint16_t)rd(c,(uint16_t)((a&0xff00u)|((a+1)&0xffu)))<<8));c->last_cycles=5;}break;
  case 0x00:{c->pc++;push(c,(uint8_t)(c->pc>>8));push(c,(uint8_t)c->pc);push(c,c->p|F_B|F_U);c->p|=F_I;c->pc=rd16(c,0xfffe);c->last_cycles=7;}break;
  case 0x40:{uint8_t l,h;c->p=(uint8_t)((pop(c)&~F_B)|F_U);l=pop(c);h=pop(c);c->pc=(uint16_t)(l|((uint16_t)h<<8));c->last_cycles=6;}break;
  default:c->stopped=o;return 0;                    /* deliberate: expose next missing opcode */
 }
 c->cycles += c->last_cycles;
 return 1;
}

/* Instruction-boundary interrupt service, not cycle-exact pin polling. */
static void interrupt_entry(smb360_cpu6502 *c, uint16_t vector) {
 push(c,(uint8_t)(c->pc>>8));push(c,(uint8_t)c->pc);
 push(c,(uint8_t)((c->p&~F_B)|F_U));c->p|=F_I;
 c->pc=rd16(c,vector);c->last_cycles=7;c->cycles+=7;
}
void smb360_cpu6502_nmi(smb360_cpu6502 *c) {
 if(!c->stopped) interrupt_entry(c,0xfffa);
}
int smb360_cpu6502_irq(smb360_cpu6502 *c) {
 if(c->stopped || (c->p&F_I)) return 0;
 interrupt_entry(c,0xfffe);return 1;
}
