# Próxima ação — após S009, 09/10/2026

Leia START_HERE.md, AGENTS.md e CONTINUITY_PROTOCOL.md. A fonte exclusiva
de gameplay continua sendo assets/canonical/SMB_v026.nes. Não substituir
PRG/CHR por dados de outra ROM ou por recriação de terceiros.

## Ponto de partida confirmado

S009 pulse 2 integrado à main via PR #4:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/4 .
Commit merge: `652f4e2d7f71d26e6a4eb6f65d438aadd6f1c957`.
Último commit de código medido em CI:
`3f877fd7d3802df35897d303105b7980d56ecd7a`.
Workflow verde (quatro jobs):
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889071037
Pixels/RAM binjnes: 600/600 idle e 600/600 com entrada, ambos por tipo.
A S009 é concluída; repetir este baseline apenas como regressão.

## Tarefa ativa S010 — canal triangle APU 2A03

1. Inspecionar `src/canonical/apu.[ch]`, `nrom.[ch]`,
   `tests/test_apu_pulse2.c` e o clock de `machine.c` sem alterar
   os dois canais pulse concluídos.
2. Implementar registradores $4008, $400A, $400B: linear counter,
   timer/period 11-bit e length counter; os relógios do triangle e
   dos pulse são distintos (triangle avança no CPU clock, pulse no
   clock APU / 2). Sequência triangle de 32 níveis, volume 0–15,
   gate por length/linear e period, bit 2 de $4015.
3. Testes unitários focados em sequência, timer, linear reload,
   control flag, comprimento/halt, enable/disable e isolamento
   de $4015 dos dois pulse. Verificar semântica NESdev, inclusive
   casos em que a unidade retém a saída, não é necessariamente zero.
4. Rodar os testes de regressão host normais e sanitizados e comparar
   a ROM canônica com binjnes (600+600 frames e RAM). Investigar
   cada divergência sem alterar a ROM ou hashes esperados.
5. Publicar evidências e documentação; depois priorizar noise, DMC,
   IRQ de frame conectada à CPU e áudio PCM/mixer no SDL, com
   comparação de áudio independente antes de prometer reprodução.

Comandos copiáveis a partir de um clone limpo:

```sh
git clone https://github.com/PedroMarioaBros/SMNEStoX360.git
cd SMNEStoX360
git log -1 --oneline
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Resultados esperados dos testes que já existem: novos testes/anteriores
passam; referência 600/600 pixels e RAM em ambos os cenários e SDL smoke
passa. Esses cenários não demonstram fidelidade de áudio. Dependências:
C99, Python 3, SDL2 dev; se clone DNS falhar, usar CI do GitHub e
registrar a distinção entre validação local e remota.

## Meta final Xbox 360

Nenhum novo `default.xex` canônico validado no console; o build antigo
#16 resultou em Fatal Crash. Para Xbox: boot mínimo → loop → vídeo seguro
→ controle → runtime canônico → áudio → XEX com dados embutidos → teste
físico. Não usar endereços de framebuffer hardcoded do backend histórico.
O runtime ainda interpreta 6502 por software e **não é tradução direta
nativa PowerPC**; avaliar essa meta separadamente, sem chamá-lo de port
sem emulador.

Em cada sessão: registrar tentativas e resultados, atualizar
WORK_CHECKPOINT.md, SESSION_LOG.md, NEXT_STEPS.md, fazer commits no
GitHub e confirmar HEAD remoto.
