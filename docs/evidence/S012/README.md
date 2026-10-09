# S012 — DMC/IRQ/DMA inicial (09/10/2026)

## Proveniência e escopo

- HEAD de entrada: `9293f007b29060efa2c7349f2ff272eeca04d60f`.
- Branch: `work/s012-apu-dmc-20261009`.
- Fonte **única** do jogo: `assets/canonical/SMB_v026.nes`, 40.976 B, NROM.
- ROM SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- PRG SHA-256: `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`.
- CHR SHA-256: `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- Nenhum asset da ROM original foi alterado. O GitHub continua fonte
  persistente e `tools/verify_repository.py` verifica a ROM no CI.

## Código novo

- `src/canonical/apu.h`: `smb360_apu_dmc`, API de saída DAC
  0–127, pedido de DMA, endereço de leitura e entrega de byte,
  consulta de IRQ.
- `src/canonical/apu.c`: $4010 (rate/loop/IRQ), $4011 (direct DAC),
  $4012 ($C000 + value*64), $4013 (length = value*16+1),
  $4015 bit4 active enable/disable, DMC IRQ bit7 status;
  16 períodos NTSC 428..54 ciclos CPU, shifter LSB-first,
  DAC saturado de 7 bits, buffer de 1 byte, silence,
  repetição e sinalização de interrupção.
  O IRQ é sinalizado ao ler o último **byte** da amostra,
  não quando terminam os últimos bits. Leitura $4015
  não limpa IRQ DMC; escrita $4015 limpa.
  Endereço DMC $FFFF→$8000 é preservado.
- `src/canonical/machine.c`: lê sample pelo barramento NROM,
  em $8000–$FFFF **somente do PRG fornecido pelo proprietário**;
  contabiliza stall de DMA estimado em 4 ciclos por byte e
  atualiza PPU/APU/CPU; conecta IRQ APU à rotina 6502 existente
  em limites de instrução. Os canais anteriores e o controle 2 em
  $4017 permanecem preservados.
- `tests/test_apu_dmc.c`: testes sintéticos para endereços,
  leitura de PRG via barramento, 16 períodos NTSC,
  shift e delta DAC saturado, IRQ $4015 e CPU, loop,
  buffer, reinício e wrap.
- `tests/run_host_tests.sh` executa a nova 12ª suíte em
  normal e ASan/UBSan (LeakSanitizer desabilitado).
- Acesso externo a NESdev somente para documentação de hardware,
  sem copiar outro jogo/PRG/CHR:
  https://www.nesdev.org/wiki/APU_DMC
  https://www.nesdev.org/wiki/DMA
  https://www.nesdev.org/wiki/APU_Status

## Validação e advertências

Workflow do último commit de código (runner incluído):
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890958510
SHA do código `e1a68b1d7ded75cb968556fe425b5fdcb584e78d`.
Atribuir cada resultado ao commit real e distinguir jobs
normal/sanitizado, frontend SDL e binjnes.
Comandos reprodutíveis num checkout completo:

```sh
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Os testes DMC são sintéticos; a comparação binjnes existente cobre
**pixels e RAM**, mas não amostras/IRQ/stalls do DMC ao executar a ROM.
Não anunciar DMC cycle-accurate com base nesse resultado.

## Limite de hardware a atacar a seguir

O DMC ainda usa temporização **aproximada**: o leitor rouba fixamente
4 ciclos depois de uma instrução completa. No NES real o DMA
pode consumir 1–4 ciclos com arbitragem get/put e
colisões com OAM DMA; temporização dos acessos da CPU
intra-instrução, dupla leitura de controles/PPU em stalls
e janela de IRQ/CLI/SEI não estão reproduzidas.
A IRQ APU é tratada em limites de instrução e não por
polling do pino ciclo a ciclo. Portanto DMC implementado
e compilável **não** é DMC eletricamente preciso.

Ainda não há mixer não linear, saída PCM/resampling/SDL audível,
comparação independente de waveform, backend canônico Xbox
validado ou tradução 6502→PowerPC nativa sem interpretação.
Build histórico #16 sofreu Fatal Crash; não reutilizar
framebuffer antigo hardcoded.

Próxima tarefa: ciclo de mixer/PCM + captura determinística de
áudio com a ROM original, ou primeiro refinar DMA/IRQ quando
a comparação mostrar divergência. Ver `docs/NEXT_STEPS.md`.
