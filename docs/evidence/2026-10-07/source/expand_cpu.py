from pathlib import Path
import re
p=Path('src/canonical/cpu6502.c'); s=p.read_text()
existing={int(x,16) for x in re.findall(r'case 0x([0-9A-Fa-f]+):',s)}
cases=[]
def add(op,body):
 if op not in existing: cases.append(f'  case 0x{op:02X}:{{{body}}}break;')
# Address calculation and read cycle costs; writes/RMW have fixed costs.
modes={
 'zp':('uint16_t a=imm(c);',3),
 'zx':('uint16_t a=(uint8_t)(imm(c)+c->x);',4),
 'zy':('uint16_t a=(uint8_t)(imm(c)+c->y);',4),
 'ab':('uint16_t a=absop(c);',4),
 'ax':('uint16_t b=absop(c),a=(uint16_t)(b+c->x);',4),
 'ay':('uint16_t b=absop(c),a=(uint16_t)(b+c->y);',4),
 'ix':('uint8_t z=(uint8_t)(imm(c)+c->x);uint16_t a=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8));',6),
 'iy':('uint8_t z=imm(c);uint16_t b=(uint16_t)(rd(c,z)|((uint16_t)rd(c,(uint8_t)(z+1))<<8)),a=(uint16_t)(b+c->y);',5),
}
def readcase(op,mode,action):
 addr,cy=modes[mode];cross='+((b&0xff00u)!=(a&0xff00u))' if mode in ('ax','ay','iy') else ''
 add(op,addr+'uint8_t v=rd(c,a);'+action+f'c->last_cycles={cy}{cross};')
def compare(reg): return f'uint8_t r=(uint8_t)(c->{reg}-v);c->p=(uint8_t)((c->p&~F_C)|(c->{reg}>=v?F_C:0));nz(c,r);'
# Group 1: all eight standard addressing forms, preserving existing implementations.
for base,action in [(0x00,'c->a|=v;nz(c,c->a);'),(0x20,'c->a&=v;nz(c,c->a);'),(0x40,'c->a^=v;nz(c,c->a);'),(0x60,'adc(c,v);'),(0xa0,'c->a=v;nz(c,v);'),(0xc0,compare('a')),(0xe0,'sbc(c,v);')]:
 for low,mode in [(1,'ix'),(5,'zp'),(13,'ab'),(17,'iy'),(21,'zx'),(25,'ay'),(29,'ax')]:readcase(base+low,mode,action)
for op,mode in [(0x81,'ix'),(0x85,'zp'),(0x8d,'ab'),(0x91,'iy'),(0x95,'zx'),(0x99,'ay'),(0x9d,'ax'),(0x84,'zp'),(0x94,'zx'),(0x8c,'ab'),(0x86,'zp'),(0x96,'zy'),(0x8e,'ab')]:
 addr,cy=modes[mode];cy={'ax':5,'ay':5,'iy':6}.get(mode,cy)
 reg='y' if op in (0x84,0x94,0x8c) else 'x' if op in (0x86,0x96,0x8e) else 'a'
 add(op,addr+f'wr(c,a,c->{reg});c->last_cycles={cy};')
for reg,ops in [('x',[(0xa6,'zp'),(0xb6,'zy'),(0xae,'ab'),(0xbe,'ay')]),('y',[(0xa4,'zp'),(0xb4,'zx'),(0xac,'ab'),(0xbc,'ax')])]:
 for op,mode in ops:readcase(op,mode,f'c->{reg}=v;nz(c,v);')
for reg,ops in [('x',[(0xe4,'zp'),(0xec,'ab')]),('y',[(0xc4,'zp'),(0xcc,'ab')])]:
 for op,mode in ops:readcase(op,mode,compare(reg))
# RMW: include original-value dummy write. Accumulator shifts do not write memory.
for base,kind in [(0x00,'asl'),(0x20,'rol'),(0x40,'lsr'),(0x60,'ror'),(0xc0,'dec'),(0xe0,'inc')]:
 if kind in ('asl','rol'):
  action='uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|((v>>7)&1));v=(uint8_t)((v<<1)'+('|carry' if kind=='rol' else '')+');'
 elif kind in ('lsr','ror'):
  action='uint8_t carry=(uint8_t)((c->p&F_C)!=0);c->p=(uint8_t)((c->p&~F_C)|(v&1));v=(uint8_t)((v>>1)'+('|(carry<<7)' if kind=='ror' else '')+');'
 else: action='v=(uint8_t)(v'+('-1' if kind=='dec' else '+1')+');'
 action=action.replace('uint8_t carry=(uint8_t)((c->p&F_C)!=0);','') if kind in ('asl','lsr') else action
 for low,mode,cy in [(6,'zp',5),(0x16,'zx',6),(0x0e,'ab',6),(0x1e,'ax',7)]:
  addr,_=modes[mode]
  # avoid unused b for abs,X (b is used to form a).
  add(base+low,addr+'uint8_t v=rd(c,a);wr(c,a,v);'+action+f'wr(c,a,v);nz(c,v);c->last_cycles={cy};')
 if kind not in ('inc','dec'):add(base+0x0a,'uint8_t v=c->a;'+action+'c->a=v;nz(c,v);')
add(0x50,'branch(c,!(c->p&F_V));');add(0x70,'branch(c,c->p&F_V);')
add(0xf8,'c->p|=F_D;')
add(0x48,'push(c,c->a);c->last_cycles=3;')
add(0x08,'push(c,c->p|F_B|F_U);c->last_cycles=3;')
add(0x68,'c->a=pop(c);nz(c,c->a);c->last_cycles=4;')
add(0x28,'c->p=(uint8_t)((pop(c)&~F_B)|F_U);c->last_cycles=4;')
add(0x6c,'uint16_t a=absop(c);uint8_t l=rd(c,a);c->pc=(uint16_t)(l|((uint16_t)rd(c,(uint16_t)((a&0xff00u)|((a+1)&0xffu)))<<8));c->last_cycles=5;')
add(0x00,'c->pc++;push(c,(uint8_t)(c->pc>>8));push(c,(uint8_t)c->pc);push(c,c->p|F_B|F_U);c->p|=F_I;c->pc=rd16(c,0xfffe);c->last_cycles=7;')
add(0x40,'uint8_t l,h;c->p=(uint8_t)((pop(c)&~F_B)|F_U);l=pop(c);h=pop(c);c->pc=(uint16_t)(l|((uint16_t)h<<8));c->last_cycles=6;')
s=s.replace('  default:', '\n'.join(cases)+'\n  default:')
# Stop metadata must not overload opcode zero; BRK is now implemented.
s=s.replace('o=rd(c,c->pc++);','c->opcode_pc=c->pc; o=rd(c,c->pc++);')
# Existing INC/DEC also need RMW dummy writes.
s=s.replace('v=(uint8_t)(rd(c,z)+1u);wr(c,z,v);','v=rd(c,z);wr(c,z,v);v++;wr(c,z,v);').replace('v=(uint8_t)(rd(c,z)-1u);wr(c,z,v);','v=rd(c,z);wr(c,z,v);v--;wr(c,z,v);')
s += '''
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
'''
p.write_text(s)
p=Path('src/canonical/cpu6502.h');s=p.read_text().replace('uint16_t pc;','uint16_t pc;\n    uint16_t opcode_pc;')
s=s.replace('\n#endif','\nvoid smb360_cpu6502_nmi(smb360_cpu6502 *c);\nint smb360_cpu6502_irq(smb360_cpu6502 *c);\n\n#endif');p.write_text(s)
print('Added',len(cases),'opcodes; total',len(existing)+len(cases))
