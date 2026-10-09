# S011 — noise da APU NTSC 2A03 (09/10/2026)

## Proveniência e limites

- HEAD de entrada `main`: `c7e4cc7c7954d6fad528dd5a23a7a5b0c89fbf74`.
- Branch: `work/s011-apu-noise-20261009`.
- ROM única do proprietário `SMB_v026.nes`, tamanho 40976 bytes,
  cabeçalho iNES `4e45531a020101000000000000000000`:
  - ROM SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
  - PRG SHA-256 `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`
  - CHR SHA-256 `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`
- Os hashes acima foram conferidos no arquivo do proprietário presente na
  sessão; `git ls-remote` local falhou por DNS
  (`Could not resolve host: github.com`). GitHub foi acessado somente
  pela conexão autorizada e os jobs CI verificam os arquivos versionados.
  Nenhum PRG, CHR, ROM ou gameplay foi substituído.

## Implementação de noise

Em `src/canonical/apu.[ch]`, o quarto canal independente possui:

- $400C: controle de envelope, volume constante, loop e length halt;
  $400D sem efeito; $400E seleciona entre 16 períodos NTSC e tap
  de feedback; $400F recarrega length somente habilitado e sinaliza
  reinício de envelope (sem reinicializar LFSR).
- LFSR de 15 bits com seed 1, feedback bit0 XOR bit1 no modo longo,
  XOR bit6 no modo curto, deslocamento para direita e feedback no bit14.
  Timer CPU/2 com períodos exatos em ciclos CPU:
  `4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068`.
- Quarter-frame atualiza envelope/divider/decay/loop; half-frame
  decrementa length se halt=0. $4015 bit3 limpa length ao desabilitar,
  não recarrega apenas por habilitar e indica estado de length nas leituras.
  Bit0 da LFSR ou length zero silenciam a saída DAC 0..15. O LFSR
  continua rodando mesmo com a saída silenciada.
- Pulse1, pulse2, triangle e leitura do controle 2 em $4017
  preservados, sem alteração do barramento `nrom.c` ou do gameplay.
- Adicionados `tests/test_apu_noise.c` (cinco cenários cobrindo 16
  períodos, fase, taps, output, envelope/length/loop e isolamento)
  e asserções de registradores em `tests/test_ppu_registers.c`.
  Runner `tests/run_host_tests.sh` passa de 10 a 11 suítes.

## Critérios de validação

As instruções permanentes exigem:
```sh
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Sucessos de CI devem ser atribuídos ao commit exato que contém o último
código, nunca apenas à branch ou a um commit anterior. O comparador
binjnes cobre pixels de 256x240 e todos os 2048 bytes de CPU RAM, não
waveform de áudio. O frontend SDL usa vídeo/teclado dummy. Se um teste
falhar, registrar o erro e corrigir antes de merge; não alterar hashes
canônicos esperados.

## Limites que permanecem

O core agora possui saídas DAC modeladas de pulse1, pulse2, triangle e
noise; **não é áudio audível**. Faltam DMC, IRQ de APU conectada à CPU,
mixer não linear, resampler/PCM, backend SDL de áudio e comparação
independente do sinal sonoro. CPU/PPU ainda rodam o PRG 6502 original
num runtime interpretado de software, não port PowerPC nativo.
Nenhum novo `default.xex` dessa arquitetura validado no Xbox.
Build histórico #16 apresentou Fatal Crash.

Referências exclusivamente de semântica de hardware, não de gameplay:
- https://www.nesdev.org/wiki/APU_Noise
- https://www.nesdev.org/wiki/APU_Envelope
- https://www.nesdev.org/wiki/APU_registers
