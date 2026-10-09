# Próxima ação — após S010 (09/10/2026)

Ao iniciar uma sessão: `START_HERE.md` → `AGENTS.md` →
`CONTINUITY_PROTOCOL.md` → `WORK_CHECKPOINT.md` → este documento →
`SESSION_LOG.md`. A ROM `assets/canonical/SMB_v026.nes` é a única
fonte do PRG/CHR e do comportamento do jogo. Proibido trocá-la.

## S010 concluída no host — não é XEX Xbox

Triangle implementado em `src/canonical/apu.[ch]`, junto a pulse1/pulse2.
Códigos e testes da branch `work/s010-apu-triangle-20261009`:
PR https://github.com/PedroMarioaBros/SMNEStoX360/pull/5 .
Commit exato do código validado:
`73ca0151d153448e8d13cca07bdf395fd93618cd`.
GitHub Actions:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889762682
(`success`: normal, sanitizers, SDL e binjnes 600+600).
Os resultados são dos cenários medidos, não certificação de áudio ou console.
Atenção à falha de fixture histórica e resolução descritas em evidence/S010.

## Tarefa técnica ativa: S011 — noise da APU 2A03

1. Inspecionar `src/canonical/apu.[ch]`, `nrom.[ch]`, `machine.c`
   e os testes; conferir HEAD e hashes canônicos no checkout.
2. Implementar o canal noise nos registradores $400C/$400E/$400F:
   envelope, length, período via tabela NTSC de 16 entradas, controle
   de modo/taps do registrador de deslocamento (LFSR de 15 bits) e
   nível 0–15; temporização CPU/2. $400D é ignorado.
   Sem modificar pulse1/pulse2/triangle nem mudar a ROM.
3. Incluir bit3 de enable/status $4015. Testar recarga de envelope,
   length halt/decay, períodos de noise, avanço do LFSR em ambos modos,
   saída silenciada pelo bit0 do LFSR, $4015, reset e independência
   dos demais canais.
4. Comprovar normal `-std=c99 -Wall -Wextra -Werror` e ASan/UBSan.
   Repetir `python3 tools/compare_reference.py --idle-frames 600
   --input-frames 600`: pixels/RAM 600/600 nos dois roteiros,
   investigar cada desvio sem editar hash esperado.
5. Registrar evidência com parâmetros, commits exatos, sucessos/falhas
   e limitações; publicar código e docs, verificar HEAD remoto.

Comandos copiáveis (clone limpo, Linux com C99/Python3/SDL2 dev):

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

Esperado: todas as suítes incluindo triangle passam, pixels/RAM idênticos
em 600/600 idle e 600/600 scripted; SDL readback 61.440 pixels PASS.
Se clone local bloquear por DNS, usar CI e declarar explicitamente que
o teste foi remoto. Não reapresentar baseline como novo avanço.

Depois do noise: implementar DMC, IRQ de APU conectada à CPU, mixer NES
não-linear, resampling PCM e saída sonora real no SDL, com referência
independente de áudio. O objetivo Xbox requer ainda boot mínimo/loop,
vídeo seguro, controle, integração da APU e XEX com assets embutidos,
seguido de teste físico. Build antigo #16 deu Fatal Crash; não reutilizar
framebuffer hardcoded. Nenhum `default.xex` canônico validado até S010.

**A arquitetura atual interpreta CPU 6502 em software**, embora execute
o PRG original sem seletor de ROM. Não é a tradução direta nativa
PowerPC sem interpretador desejada pelo proprietário. Preservar essa
distinção em cada checkpoint.

Antes de responder no final: atualizar `WORK_CHECKPOINT.md`,
`SESSION_LOG.md`, este arquivo, publicar commits e conferir HEAD
remoto. Não inventar percentuais de progresso.
