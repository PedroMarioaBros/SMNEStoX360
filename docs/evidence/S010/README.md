# S010 — canal triangle da APU 2A03 (09/10/2026)

## Entrada e proveniência

- Branch inicial `main`: `030813421ed1aff21e5b6289392a48c9f2c17d1c`.
- Branch de trabalho: `work/s010-apu-triangle-20261009`.
- Pull Request: https://github.com/PedroMarioaBros/SMNEStoX360/pull/5
- Arquivo do proprietário, 40.976 bytes, iNES NROM256:
  - ROM SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
  - PRG SHA-256: `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`
  - CHR SHA-256: `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`
- Estes hashes foram conferidos no anexo do proprietário no ambiente local;
  no CI, `verify_repository.py` conferiu os arquivos do checkout remoto.
  Assets canônicos não foram modificados nesta sessão.

## Implementação entregue em src/canonical/apu.[ch]

- `smb360_apu_triangle` tem estado independente (registros, linear
  counter, length counter, período de 11 bits, timer e índice da onda).
- $4008: control/length halt (bit 7), linear reload value (7 bits);
  escrita não aciona o reload flag automaticamente.
- $400A: período inferior; $400B: período superior, recarga de length
  (somente habilitado) e ativação do linear reload flag, **sem** reiniciar fase.
- $4015: bit 2 habilita/limpa length; leitura reporta bit 2 se length>0;
  pulse1 e pulse2 preservados. $4009 é ignorado.
- Triangle clock CPU/1, enquanto pulse1/pulse2 clock CPU/2.
  Timer divide pelo período+1 e avança a sequência cíclica de 32 níveis:
  `15..0, 0..15`. Os dois contadores atuam como gates do avanço,
  mas DAC retém último nível em vez de forçar zero quando para.
- Quarter frame: linear reload/decrement e limpeza condicional do flag.
  Half frame: length decrement somente quando control bit7 está desligado.
- Sem PCM, amplitude analógica, mixer, SDL áudio ou avanço de IRQ ao CPU.

## Testes e regressão

O ambiente local desta sessão não conseguiu clonar GitHub via HTTPS
(`Could not resolve host: github.com`). Para não inventar testes locais, a
compilação completa, comparação e sanitizers são atribuídos **somente** ao
GitHub Actions. O hash do anexo foi conferido localmente.

Código avaliado pela CI: `73ca0151d153448e8d13cca07bdf395fd93618cd`
- https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889762682
- Resultado: `completed/success`; quatro jobs concluídos: `host (0)`,
  `host (1)`, `reference`, `frontend`.
- 10 testes host passaram nos modos normal e ASan/UBSan
  (neste projeto detect_leaks=0).
- `test_apu_triangle: PASS`, `test_apu_pulse: PASS`,
  `test_apu_pulse2: PASS`, `test_ppu_registers: PASS`.
- `Repository assets and historical artifact: PASS`.
- Diferencial binjnes: `idle pixels: 600 / 600 RAM: 600 / 600`;
  `input pixels: 600 / 600 RAM: 600 / 600`.
  Todas as 2.048 posições da RAM por captura, nenhuma exclusão.
- SDL readback: `61440 PASS` pixels; quadro 599 bateu hash da
  referência `fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc`.
  O teste SDL ainda é virtual/dummy, não sessão humana ou Xbox.

## Falhas durante a construção e como foram resolvidas

Dois jobs intermediários falharam:
- https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889678133
- https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889702475

Ambos: `test_gates_hold_last_level` presumiu que o nível da onda
ficaria estático imediatamente após a escrita em $4008.
**Causa na expectativa do teste**, não reprodução de saída da referência:
o linear counter aplica a nova recarga no quarter-frame seguinte e o timer
pode continuar avançando entre a escrita e aquele instante.
Corrigido o teste para capturar o nível **depois** do quarter-frame
(commit `95657fe2732472b72c6c4c6ff08de0392faff49a`).
CI confirmou o sucesso após correção. Não houve patch artificial de
gameplay/PRG/CHR para forçar uma aprovação.

Ajuste adicional de infraestrutura: `.github/workflows/canonical-host.yml`
agora observa `tests/test_apu*` nos push events, para que qualquer edição
de fixture de áudio dispare a bateria sem edição artificial do runner.

## Limitações e próximo comando

Não há um teste de fidelidade de **amostras de áudio** com referência
independente: o comparador existente verifica pixels e RAM apenas.
Triangle/DAC foi validado por testes sintéticos de semântica, não por
waveform capturada de um NES real. A temporalidade interna é agrupada
por instrução; timers APU e PPU podem ficar desalinhados em acessos
intra-instrução. Noise, DMC, mixer/resampler/PCM e IRQ da APU para CPU
permanecem pendentes. Nenhum default.xex funcional da arquitetura canônica
foi validado no Xbox 360; o antigo build #16 teve Fatal Crash.

Próximo trabalho: noise ($400C/$400E/$400F, LFSR, length e envelope),
com testes normal/san e comparação 600+600; depois DMC e geração de PCM.
Ver `docs/NEXT_STEPS.md`.

Referências técnicas (sem extrair/recriar gameplay):
- https://www.nesdev.org/wiki/APU_Triangle
- https://www.nesdev.org/wiki/NES_APU
- https://www.nesdev.org/wiki/APU_registers
