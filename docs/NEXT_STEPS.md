# Próxima ação — após S013, 09/10/2026

Leia `START_HERE.md`, `AGENTS.md`,
`docs/CONTINUITY_PROTOCOL.md`, `docs/WORK_CHECKPOINT.md`
e `docs/SESSION_LOG.md` antes de trabalhar.
O PRG/CHR original do proprietário está em
`assets/canonical/SMB_v026.nes` (ROM SHA-256
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`).
Não usar dados de gameplay de outro Mario ou outra ROM.

## Ponto de partida real

S013 integrada pela PR #9:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/9 ;
commit squash `64a7edaa76c14b069da283023bab38d1b64e1173`.
A S013 introduziu `src/canonical/audio_pcm.[ch]`: mixer
não linear dos cinco canais, phase accumulator determinístico,
PCM mono 16-bit a 48 kHz, filtros simples;
`src/host/main.c` tem fila de áudio SDL opcional e `--wav`.
Validação do último código:
`5666e3437e7073e781862991fa9d0b5fb4bd61ed`,
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892058096 .
Quatro jobs verdes, 13 suítes C99 normal/san, 600/600
pixels e RAM sem comandos e 600/600 com comandos.
Teste de audio de 600 frames, duas capturas byte-idênticas:
479.146 samples, min -5889/max +6328, WAV SHA-256
`3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.
Artefato disponível como `canonical-s013-audio-wav`
no workflow (retenção de 30 dias). Não é comparação com
sinal de áudio de referência.

## S014 — iniciar XEX **canônico**, sem depender de nathsou

Objetivo principal: aproximar o projeto da execução real no Xbox 360,
sem reaproveitar a implementação de gameplay de terceiros.
Tratar como estágio de boot/compilação, não declarar jogo
funcional antes do teste físico.

1. Inspecionar ferramentas, arquivos `src/platform/xex/`,
   `src/canonical/`, `scripts/build-smb-xex.sh`, workflows
   e issue https://github.com/PedroMarioaBros/SMNEStoX360/issues/8 .
   O build histórico usa `nathsou/smb`, gera `default.xex`
   histórico e falha ao checar `boot-test.xex` inexistente.
   **Não misturar esse código com gameplay da ROM canônica.**
2. Criar target de compilação PPC via OpenXeChain para
   o core C99 `src/canonical/` e PRG/CHR autorizados
   embutidos; verificar hashes, símbolos, dependências
   e layout. Criar entrypoint mínimo e backend seguros
   sem endereços de framebuffer hardcoded, com logs,
   timer, controle e interrupções tratados cautelosamente.
3. Construir `default.xex` desse target separado,
   conferir magic XEX2, hash, mapa, assets e
   link no CI. Um XEX2 compilado **não** prova boot;
   documentar diferença entre build e teste físico.
4. Testar em hardware desbloqueado quando houver
   acesso físico do proprietário; preservar resultados
   de boot/crash e evitar afirmar avanço no console
   sem evidência.
5. Não violar o desejo de port direto:
   o runtime atual interpreta CPU 6502.
   Depois do boot, abrir subprojeto de tradução
   **estática** para instruções PPC, reaproveitando
   dados da ROM canônica e separando clearly da
   interpretação atual. Avaliar tamanho/semântica de
   cada instrução antes de afirmar que o interpretador
   foi removido.

Tarefa secundária S014 / S015: validar áudio independentemente:
produzir traço das alterações da APU e amostras do
mesmo roteiro em binjnes, com clocks e controles
iguais, comparar hash/ondas e investigar desvios.
Refinar DMC DMA/IRQ por microciclos e filtros
(90/440/14 kHz estão apenas aproximados),
sem patch específico da ROM.

Comandos baseline (Linux, compilador C99/Python3/SDL2):

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
python3 tests/test_host_audio.py
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  build/host/smb-v026-host --smoke --wav build/host/audio.wav
```

Atenção: a gravação WAV é possível sem dispositivo físico de
áudio. A reprodução por SDL com alto-falantes requer um
dispositivo real que não foi testado nesta sessão.
Não existe novo XEX canônico validado no Xbox;
build histórico #16 teve Fatal Crash.
Não inferir progresso percentual total do port a partir
de funções isoladas. Documentar e preservar
`WORK_CHECKPOINT.md`, `SESSION_LOG.md`, esta
tarefa, evidências e HEAD remoto após cada sessão.
