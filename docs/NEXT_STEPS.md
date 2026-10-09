# Próxima ação — após S008, 09/10/2026

Ler START_HERE.md, AGENTS.md e CONTINUITY_PROTOCOL.md. Usar sempre ROM
canônica versionada, sem trocar PRG/CHR. S008 foi integrada à main pelo
PR #3; merge/squash: `f0af8dbf7236571760e03e6e36938ae61c44edab`.
CI do código `af36b563ee19543acd826bd89a768748e6ae0b5e`:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888390752
(success: host normal/san, SDL e referência curta 600+600).

## Próxima tarefa técnica: pulse 2 (e testes dedicados)

1. Verificar HEAD atual, ROM/PRG/CHR; repetir baseline apenas como regressão.
2. Inspecionar `src/canonical/apu.[ch]` e `tests/test_apu_pulse.c`.
   Generalizar canal pulse mantendo sinais e estado separados; canal 2 usa
   complemento de dois no sweep negativo (diferente de pulse 1).
3. Criar testes independentes de duty, envelope, length, timer, sweep,
   reset de fase, enable em $4015 e saída DAC 0..15 de ambos canais.
   Ampliar asserts de tempo APU/CPU/NMI/DMA no bus.
4. Validar, com a ROM original, pixels/RAM contra o binjnes fixado, sem
   permitir regressões no controle 2; registrar diferenças, não alterar hashes.
5. Depois: triangle, noise, DMC, IRQ da APU conectada à CPU, mixer não-linear
   e amostragem de PCM no SDL/host. Não declarar áudio funcional até haver
   saída audível e comparação com referência.

Comandos imediatos copiáveis:

```sh
git clone https://github.com/PedroMarioaBros/SMNEStoX360.git
cd SMNEStoX360
git log -1 --oneline
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Esperado no HEAD de partida: testes host e frontend OK; 600/600 capturas
idle + 600/600 scripted com pixels e RAM iguais. A comparação é limitada
a esses roteiros, não a todas as modificações da ROM. Se o ambiente não
permitir clone/local, registrar bloqueio e verificar execução específica de CI.

## Meta final Xbox 360

Ainda sem default.xex dessa arquitetura validado no console. Continuar
boot mínimo → loop → vídeo seguro → input/core → APU/áudio → XEX com
assets embutidos → teste físico. Não reutilizar endereços hardcoded do
backend que causou Fatal Crash #16.

A arquitetura atual executa o código 6502 original via runtime (CPU/PPU
implementados em software), não é ainda tradução nativa PowerPC livre de
interpretação de opcodes. Esta distinção deve permanecer explícita ao avaliar
a meta de port direto solicitada pelo proprietário.

Em cada próxima sessão: atualizar WORK_CHECKPOINT.md, SESSION_LOG.md,
NEXT_STEPS.md, publicar commits e confirmar HEAD remoto.
