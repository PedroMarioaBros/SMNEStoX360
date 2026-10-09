# Próxima ação — após S012, 09/10/2026

Leia `START_HERE.md` e siga `AGENTS.md` e
`docs/CONTINUITY_PROTOCOL.md`, checkpoint e diário antes de trabalhar.
A única ROM autorizada para conteúdo de jogo é
`assets/canonical/SMB_v026.nes`; validar ROM/PRG/CHR SHA-256.
Não utilizar PRG/CHR de outros jogos nem recriar Super Mario manualmente.

## Ponto de retomada verificado

S012 implementou o DMC, a quinta unidade da APU digital NTSC,
mas DMA tem stall fixo de 4 ciclos **aproximado** e IRQ de APU
polling por instrução. Não alegar DMC cycle-perfect.
Último commit de código testado:
`e1a68b1d7ded75cb968556fe425b5fdcb584e78d`.
Workflow https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890958510
**completed/success**: host normal, ASan/UBSan, referência e SDL.
12/12 suítes C99 passaram, inclusive `test_apu_dmc.c`.
Capturas binjnes = 600/600 pixels e RAM idle, e
600/600 pixels e RAM input; SDL readback 61.440 pixels PASS.
Essas métricas **não** verificam o sinal de áudio.
Detalhes e bloqueios em `docs/evidence/S012/README.md`.

## Tarefa ativa S013 — gerar PCM audível no frontend SDL

1. Ler `src/canonical/apu.[ch]`, `machine.c`, `src/host/`,
   `tests/test_host_frontend.py` e verificar se as saídas DAC
   usam a mesma base temporal; inspecionar quantos ciclos CPU são
   processados a cada frame e suas limitações.
2. Implementar mixer não linear do NES para pulse1+2 e
   triangle/noise/DMC, com saída PCM mono de taxa definida
   (ex.: 48 kHz) e amostragem determinística baseada em CPU NTSC.
   Cuidado com DMC 7 bits e offset DC.
3. Testes sintéticos de componentes e mistura:
   níveis 0..15 pulse/triangle/noise, DMC 0..127,
   silêncio, transições, limites, saturação, consistência entre
   builds normais/sanitizados e contagem previsível de amostras.
4. Habilitar dispositivo de áudio SDL quando disponível;
   manter modo headless/dummy em CI sem bloquear gameplay/render.
   Registrar captura PCM/WAV de cenário real com a ROM canônica,
   com SHA-256 e critérios de ausência de clipping. Comparar
   waveform ou estado APU com referência fixada independente;
   não declarar fidelidade completa antes disso.
5. Rodar todas as 12 suítes C99 em normal e ASan/UBSan,
   verificar ROM/PRG/CHR, frontend SDL e comparação visual/RAM
   binjnes 600+600, registrar evidências, merge e HEAD remoto.

Comandos reproduzíveis num clone limpo:

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

Meta de regressão: 12 suítes PASS e pixels/RAM de
600/600 para idle/input, sem desligar testes para obter
um CI verde. Em caso de falha DNS no clone, verificar
resultados do CI de **commit exato** e registrar bloqueio.

## Dependências e estratégia Xbox

DMC DMA em `machine.c` continua com stall fixo e IRQ
amostrada após instruções; demanda refinamento de microciclos,
principalmente colisões com OAM DMA e acesso a controles/PPU.
Depois do mixer/PCM, expandir comparação diferencial
para APU e investigar divergências de som/timing.

Não existem XEX canônicos desta arquitetura testados
em Xbox 360. O build antigo #16 sofreu Fatal Crash,
e endereços hardcoded de framebuffer daquele backend
não podem ser reutilizados. Boot mínimo seguro, loop,
vídeo, controles, runtime com ROM embutida, áudio e
teste físico no console são tarefas não concluídas.

**Arquitetura atual interpreta o código 6502 original**
em software; ainda não é a tradução direta para PowerPC
sem emulador pretendida pelo proprietário. Avaliar
a conversão estática PPC como um esforço separado; não
chamar o runtime atual de port nativo sem emulação.

Em toda sessão: atualizar checkpoint/diário/próximos
comandos, publicar commit na main e confirmar HEAD remoto.
