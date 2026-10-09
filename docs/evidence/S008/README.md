# S008 — 09/10/2026 (America/Sao_Paulo): base APU / pulse 1

Entrada: main `f24d735559ca74eee7467224b5023402e64fdf69`.
Branch de desenvolvimento: `work/s008-apu-pulse1-20261009`.
ROM do proprietário verificada sem alterações:
- ROM SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`;
- PRG SHA-256 `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`;
- CHR SHA-256 `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.

Arquivos: `src/canonical/apu.h`, `apu.c`, `nrom.[ch]`, `machine.c`,
`tests/test_apu_pulse.c`, `tests/test_ppu_registers.c`,
`tests/run_host_tests.sh`.

Implementado: relógio de APU por ciclos de CPU, contador de quadros NTSC
4/5 passos (aproximação por instrução), escrita atrasada de $4017,
pulso 1 com duty, timer, envelope, length, sweep, mute por período/overflow,
status parcial $4015, separação explícita de $4017 leitura (controle 2) e
escrita (frame counter). Clock inclui NMI e stall OAM DMA.
Saída pulse1 é nível DAC 0..15; **sem PCM, mixer, SDL, ou som no Xbox**.
IRQ da APU é registrado internamente; ainda não chega à CPU.
Outros quatro canais ainda não implementados; leitura $4015 é parcial.
Não anunciar timing cycle-accurate, áudio jogável ou XEX.

Validação local isolada (sem checkout completo):
```sh
cc -std=c99 -O2 -Wall -Wextra -Werror src/canonical/apu.c tests/test_apu_pulse.c -o test_apu
./test_apu
cc -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined src/canonical/apu.c tests/test_apu_pulse.c -o test_apu_san
ASAN_OPTIONS=detect_leaks=0 ./test_apu_san
```
Ambos: `APU pulse 1/frame counter: PASS`.
Teste unitário cobre duty/temporizador, mute e gate, envelope/length,
4 e 5 passos, IRQ flag, $4017 reset e sweep; ainda não é teste
independente de hardware. LeakSanitizer desativado.

Tentativa de clone local direto falhou por DNS:
`fatal: unable to access ... Could not resolve host: github.com`.
A publicação ocorreu pelo conector GitHub autorizado. Por isso a
comparação completa e todas as suítes devem ser confirmadas em CI; não
alegar que foram executadas localmente.

Referências técnicas:
- https://www.nesdev.org/wiki/NES_APU
- https://www.nesdev.org/wiki/APU_Pulse
- https://www.nesdev.org/wiki/APU_Sweep
- https://www.nesdev.org/wiki/APU_Envelope

## Fechamento (resultado posterior aos testes isolados)

CI GitHub Actions `37888390752` para o commit de integração
`af36b563ee19543acd826bd89a768748e6ae0b5e` terminou **success**:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888390752
Quatro jobs concluídos com sucesso (host normal, host sanitizado, frontend
SDL e comparação 600+600 frames, pixels/RAM). Não extrapolar para fidelidade
de APU ou execução no Xbox.

PR #3 https://github.com/PedroMarioaBros/SMNEStoX360/pull/3
foi integrado à main por squash em
`f0af8dbf7236571760e03e6e36938ae61c44edab`.
A próxima etapa mudou para implementação do segundo canal pulse 2
e seus testes independentes; comandos exatos em `docs/NEXT_STEPS.md`.

