# Próxima ação — após S014 (09/10/2026)

**Continuidade obrigatória:** `START_HERE.md`, `AGENTS.md`,
`docs/CONTINUITY_PROTOCOL.md`, `docs/WORK_CHECKPOINT.md`,
`docs/SESSION_LOG.md`, esta tarefa. Confirmar `main` HEAD remoto.
O jogo é EXCLUSIVAMENTE `assets/canonical/SMB_v026.nes`
(ROM SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`).
NÃO importar outros PRG/CHR ou gameplay.

## Resultado comprovado da S014

A S014 entrou na main pela PR #10:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/10 ;
commit squash `b3a0aa56d6b45be588cec0ebfd936f1cb070ce07`.
O primeiro target **Xbox canônico** está integrado em:
`src/platform/canonical_xex/main.c`,
`src/platform/canonical_xex/embedded_rom.S`,
`scripts/build-canonical-xex.sh`,
`.github/workflows/canonical-xex.yml`.
Compila os mesmos fontes `src/canonical/*.c` do host,
com `SMB_v026.nes` íntegra embutida. Não depende de
`nathsou/smb`, diferentemente do `scripts/build-smb-xex.sh`
histórico e da issue
https://github.com/PedroMarioaBros/SMNEStoX360/issues/8 .

CI compilação PPC PowerPC `75eb79a35b6c60ff8ed4d4e17a88404af89c2325`:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958120419
`completed/success`. `default.xex` com magic XEX2,
manifest e checksums verificados, artifact de 30 dias
`smb360-CANONICAL-DIAGNOSTIC-xex` (id 11630285368).
**Atenção: esse XEX é DIAGNÓSTICO SEM VÍDEO, SEM ÁUDIO, SEM INPUT,
não foi testado em hardware e não é jogo jogável.**
LLVM emitiu `lld-link: warning: /align specified without /driver;
image may not run`. Observar possível problema de boot.

Host regressão no commit
`0217fc98606e491c4458855e7f413b581474d36e`:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958266320
quatro jobs verdes; 13 suítes host normal/san PASS,
binjnes pixels+RAM 600/600 idle e 600/600 com input,
SDL readback 61.440 pixels e WAV 479146 amostras com
hash `3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.
Isso NÃO testa vídeo/áudio no console.

## S015 — boot, diagnóstico observável e tela segura

1. Reler os fontes XEX novos, CI, a issue #8 e
   `docs/evidence/S014/README.md`. Preservar baseline
   do runtime e hashes de ROM/PRG/CHR.
2. Investigar o warning `/align specified without /driver`
   no OpenXeChain e validar layout/headers XEX e imports
   contra documentação da toolchain; não remover flags
   sem experimento comparativo.
3. Criar um primeiro caminho seguro para VIDEO Xbox,
   baseado em API validada do sistema/toolchain ou
   libxenon documentado. **Jamais** reutilizar
   `src/platform/xex/video_fb.c` e seus endereços
   `0x7fc...`/`0xdffff...` conhecidos por Fatal Crash.
   Priorizar tela de diagnóstico antes de gameplay.
4. Implementar leitura de controle Xbox e mapeamento
   dos 8 botões para `smb360_nrom_set_controller1/2`,
   testando parsing e preservando a ROM. Pacing adequado,
   registro de travamentos, de desempenho e watchdog seguro.
5. A validação de BOOT e tela requer **teste físico pelo
   proprietário** no Xbox 360 desbloqueado. Solicitar
   apenas quando houver pacote diagnóstico e instruções
   objetivas. Distinguir "compilou", "XEX2 válido",
   "iniciou", "renderizou", "jogável".
6. Depois do boot/vídeo/input, adicionar mixer PCM do
   `src/canonical/audio_pcm.c` ao dispositivo áudio
   seguro do Xbox, sem usar a APU `nathsou`.
7. Rota de port **sem interpretador**: documentar
   uma conversão estática de opcodes/fluxo da ROM
   canônica 6502→PowerPC; medir cobertura e
   precisão antes de afirmar tradução total.
   Esse requisito ainda NÃO foi atendido.

Comandos para reproduzir (clone Linux com ferramentas host):

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
```

Com o OpenXeChain disponível no mesmo target do
workflow `canonical-xex.yml`:

```sh
OPENXECHAIN_PREFIX="$PWD/.openxechain/sysroot" \
  bash scripts/build-canonical-xex.sh
test "$(dd if=build-canonical-xex/default.xex bs=1 count=4 2>/dev/null)" = XEX2
(cd build-canonical-xex && sha256sum -c SHA256SUMS.txt)
```

Esperado: C99 normal + ASan/UBSan íntegros,
600+600 pixels/RAM idênticos à referência,
checksum ROM/PRG/CHR intacto e XEX2 compilando.
**Não declarar boot ou jogabilidade sem evidência física.**
Qualquer falha nova deve ficar registrada em
`docs/SESSION_LOG.md` e `docs/evidence/`.
Atualizar checkpoint/NEXT_STEPS e confirmar commit publicado
na main antes da resposta.
