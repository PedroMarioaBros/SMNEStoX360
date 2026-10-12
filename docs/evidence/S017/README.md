# S017 — primeiro PE/XEX PowerPC com ponto de entrada sem Newlib CRT

**Motivação e resultado físico anterior:** S015
`default.xex` gerado por SynthXEX **causou Fatal Crash
imediato no Xbox 360** em 12/10/2026; ver
https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .
O proprietário usa o exploit de software ABadAvatar,
mas exige um executável Xbox 360 real sem depender de
explicações genéricas sobre o método de desbloqueio.
Nenhum novo XEX S017 foi iniciado em console até esta
evidência; não afirmar que um sucesso de CI valida o boot.

## Descoberta concreta: startup Newlib desnecessário

O S016 `boot_kernel_only.c` era uma `main`
ligada com Newlib CRT, e o PE 15360 B continha:
- 10 exports `xboxkrnl.exe` de file/heap/thread/Debug;
- 7 exports `xam.xex` (`CreateFileA`, `ReadFile`,
  `WriteFile`, `XamLoaderLaunchTitle`, etc.).

Isso foi verificado nos PE gerados
pelo artifact anterior da S016, não numa descrição teórica.
O S017 fornece `smb360_entry` diretamente ao linker
com `-nostdlib -Wl,/entry:smb360_entry` e
`-Wl,/nodefaultlib:libc.a`, usando apenas
os stubs oficiais OpenXeChain `xecorelib`
e três chamadas `KeDelayExecutionThread`,
`XamLoaderTerminateTitle` e `ExTerminateThread`.
Arquivos `src/platform/canonical_xex/native_crtless_*.c`;
script `scripts/build-crtless-xex.sh`.
Não reutiliza nathsou nem MMIO de vídeo histórico.

## Dois programas XEX2 criados

**01 (raiz do artifact: `default.xex`):**
sem ROM ou gameplay, espera ~5 segundos, pede
encerramento do título ao Xbox. PE ≈2048 bytes, módulo
Xbox PPC `0x01f2`, entrypoint XEX/PE igual.
Nenhum Newlib CRT ou file I/O no PE.

**02 (`02_CANONICAL_CRTLESS/default.xex`):**
mesma entrada independente de CRT, executa o
`src/canonical/` original, com
`assets/canonical/SMB_v026.nes` inteira de 40976 B
embutida **uma vez**, SHA256
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
C99 freestanding `memcpy/memset` em
`native_crtless_memory.c`; 120 frames com
watchdog; sem vídeo, áudio nem controles Xbox.
É um *diagnóstico de runtime NES no PowerPC*,
**não** o port jogável e **a CPU 6502 ainda é
interpretada**. Sem dispositivo físico,
sucesso de execução é impossível de confirmar.

`tools/verify_crtless_xex.py` inspeciona cabeçalho
PE PowerPC, `IMAGE_SUBSYSTEM_XBOX=14`, imagem
`0x82000000`, entrypoint do XEX2 id `0x00010100`,
import descriptors reais (exatamente as três funções),
ausência de `CreateFileA`, `ReadFile`, `WriteFile`,
`XNotifyQueueUI`, `XamInputGetState`,
`XamLoaderLaunchTitle`; ROM exata na etapa 2,
ausente na etapa 1. Cada etapa possui manifest
de procedência + SHA256SUMS.

## Bug concreto corrigido/contornado no gerador SynthXEX

Primeira tentativa de incluir ROM e núcleo com
ponto de entrada CRT-free CI
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206298625
**failure**: SynthXEX retornou literalmente
`ERROR: Out of memory. Aborting.`. O PE canônico
foi de fato compilado (68096 bytes; .text, .rdata,
.data, .rodata) mas a seção `.data` era puramente
BSS (virtual 0x1fa58, raw size zero).
Inspeção do código público do SynthXEX
https://github.com/OpenXeChain/SynthXEX :
`src/pemapper/pemapper.c` faz `realloc(buffer,
sectionInfo[i].rawSize)`, trata `NULL` como
`ERR_OUT_OF_MEM`; `realloc(ptr,0)` não é
uma real alocação, podendo retornar NULL.
Uma constante inicializada, `volatile`, realmente
usada em `native_crtless_canonical.c`,
produziu `.data` de tamanho bruto não zero,
permitindo o gerador terminar; **sem alterar
ROM ou gameplay**.

O primeiro build S017 mínimo foi
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206131060
`completed/success`, SHA XEX
`d72073ad5e62af47f9deefb86ef0a6e865ffb2f074766fee412a29f344af863c`.
Após correção da seção zero, workflow
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206437946
`completed/success`, ambas as imagens,
PE 2048 e 68608 B, XEX 200704 e 397312 B,
mesmas importações exatas. O runner host e
build canônico da commit `439ab0ce46f497a220f803cb34e3ac805e6a148e`
também foram `success`.

O aviso de linker
`/align specified without /driver; image may not run`
**ainda permanece**. Não foi ocultado com
`/driver`, pois uma imagem de driver pode não
ser correta para o título Xbox. XEX2 válido,
testes compilados e ROM embutida não equivalem
a compatibilidade garantida/boot testado.

## Riscos e protocolo para a próxima sessão

- Nunca executar novamente S015: Fatal Crash real.
- Sempre executar **S017 01** em pasta separada
  antes de cogitar **02**, e nunca substituir
  `BadUpdatePayload/default.xex`,
  Aurora, plugins, firmware ou NAND.
- Novo Fatal Crash = interromper, registrar,
  corrigir com a evidência; não insistir em reboot.
- O próximo alvo real depois do boot é gerar
  saída visual Xbox via API estável, áudio,
  gamepad e tradução estática 6502→PPC,
  sem usar backend legado MMIO.

## Publicação e validação pós-merge

A PR [#14](https://github.com/PedroMarioaBros/SMNEStoX360/pull/14)
foi integrada por squash em
`40825ff3530d426b05d3e23b4ab02b8d82f12ebe`.
CI da branch final:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206678772
`success`, artifact `smb360-S017-NATIVE-CRTLESS-DIAGNOSTIC`
#11728050710. Ambos os arquivos foram extraídos,
re-verificados localmente como `XEX2`, com SHA-256
correspondente aos respectivos `SHA256SUMS.txt`,
e a ROM do proprietário apareceu byte-idêntica
exatamente uma vez no PE da Etapa 02.
- S017 ETAPA 01: 200704 bytes,
  `e1e6eedc3722f442d02b4054247726927956e2681609b673c2f42f67cbcf5404`.
- S017 ETAPA 02: 397312 bytes,
  `cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee`.

CI main pós-merge:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754775
**success** (XEX CRT-free); regressão canônica
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754778
**success**. Ambos S017 continuam **NÃO TESTADOS NO XBOX**.
O usuário já recebeu os executáveis no chat, seguindo
guia de teste escalonado e sem alterar payload do desbloqueio.
