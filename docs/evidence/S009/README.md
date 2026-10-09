# S009 — 09/10/2026 (America/Sao_Paulo): canal pulse 2

Entrada em `main`: `248c8758f5421c7ed1c7bbf230b8e509c79f5d83`.
Branch de trabalho: `work/s009-apu-pulse2-20261009`.

## Identidade da ROM (arquivo do proprietário, sem alterações)

- ROM: 40976 bytes; SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- PRG: 32768 bytes; SHA-256 `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`.
- CHR: 8192 bytes; SHA-256 `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- Header iNES `4e45531a020101000000000000000000`.

## Implementação

- `src/canonical/apu.h` e `apu.c`: pulse 2 independente nos endereços
  $4004–$4007; ambas as unidades recebem o mesmo relógio da APU e
  sequenciador de envelope/length/sweep, mas guardam timer, duty, period,
  length e envelope/sweep individuais.
- Sweep negativo aplica complemento de um no pulse 1 (`period - change - 1`)
  e complemento de dois no pulse 2 (`period - change`), sem substituir dados
  ou lógica da ROM. Cada saída DAC independente de 0 a 15.
- $4015 habilita/desabilita e reporta bits 0 e 1 separadamente; reinicialização
  de comprimento depende da habilitação e da escrita em $4003/$4007.
  Outros bits de $4015 ainda parciais.
- `tests/test_apu_pulse2.c`: quatro testes novos de temporização/duty,
  envelope e length, escrita/enable/status e diferença de sweep entre canais.
- `tests/test_ppu_registers.c`: asserções de write dos dois canais,
  status $4015, leitura exclusiva do controle 2 em $4017 e alinhamento de clock
  CPU/APU. `tests/run_host_tests.sh` executa ambos os suites pulse.

## Testes locais realmente executados

Os arquivos `apu.c`, `apu.h`, `test_apu_pulse.c` e `test_apu_pulse2.c`
foram compilados em ambiente local isolado. Binário gerado com o C99 padrão e
flags `-Wall -Wextra -Werror`; ambos os executáveis executaram também sob
ASan/UBSan, com `ASAN_OPTIONS=detect_leaks=0`.

Comandos de reprodução local (a partir de um checkout completo):

```sh
cc -std=c99 -O2 -Wall -Wextra -Werror src/canonical/apu.c tests/test_apu_pulse.c -o build/test_apu_pulse
cc -std=c99 -O2 -Wall -Wextra -Werror src/canonical/apu.c tests/test_apu_pulse2.c -o build/test_apu_pulse2
build/test_apu_pulse
build/test_apu_pulse2
sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
```

Saída local isolada: `APU pulse 1/frame counter: PASS` e
`APU pulse 2 independence/sweep/envelope/status: PASS` em normal e
ASan/UBSan, quatro execuções bem-sucedidas. Houve duas falhas durante a
construção dos testes: a primeira fixture escreveu index 1 (length=254)
quando pretendia index 3 (length=2), e uma asserção de waveform foi feita
em duty phase 7 (saída zero) em vez da phase 0; fixtures corrigidas e
retestadas, sem necessidade de alterar o core para satisfazer a fixture.

O clone GitHub direto na máquina local está bloqueado por DNS
(`Could not resolve host: github.com`); a conexão autorizada do GitHub
foi utilizada para publicar a implementação. Não anunciar que a suíte host
completa ou o comparador binjnes rodaram localmente; conferir jobs GitHub
Actions e seus commits exatos. Nenhum hardware Xbox usado.

## Limites

Pulse1 e pulse2 produzem apenas níveis digitais, não áudio PCM audível.
Sem triangle, noise, DMC, mixer, saída SDL e sem IRQ APU ligada à CPU.
Relógio do frame sequencer e escritas CPU ainda agregados por instrução,
não equivalentes a microciclos do hardware. Validar áudio contra uma
referência independente antes de anunciar fidelidade sonora.
Sem novo default.xex canônico validado no console.

Referências sem substituir gameplay:
- https://www.nesdev.org/wiki/APU_Pulse
- https://www.nesdev.org/wiki/APU_Sweep
- https://www.nesdev.org/wiki/APU_Envelope

Próxima etapa: verificar jobs GitHub Actions do último commit de código,
comparar pixels/RAM dos roteiros existentes e, após aprovação, integrar e
seguir com o canal triangle.
