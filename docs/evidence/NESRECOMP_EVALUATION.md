# NESRecomp feasibility evaluation — SMB_v026

Status: **generation and seven sampled differential host scenarios (24,600 frames) pass in CI; full selector coverage and Xbox/XEX port remain pending**.

## Decision being tested

Pause expansion of our own static translator while evaluating the upstream
[NESRecomp framework](https://github.com/mstan/nesrecomp) against the exact
canonical ROM in this repository. This is a tool feasibility study, not a
decision to replace the project's architecture.

Pinned upstream revision:
`a39d9cdbec0ac98b1430ddb53747b5ed8f8cac87`

Upstream documentation describes a static 6502-to-C recompiler, a default
cycle-oriented backend, and a NES hardware runner. Mapper 0/NROM is listed as
supported, and an SMB project is listed as playable. Those facts make it a
strong candidate, but do not prove it can recompile our modified ROM or target
Xbox 360.

## Canonical input

- File: `assets/canonical/SMB_v026.nes`
- Size: 40,976 bytes
- SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
- Expected format: iNES, 32 KiB PRG, 8 KiB CHR, mapper 0 / NROM-256.

The harness verifies the full ROM hash before downloading/building the tool.
It never patches or rewrites the canonical ROM.

## What is added to this repository

- `tools/evaluate_nesrecomp.sh`: repeatable host-side evaluation. It fetches
  only the pinned upstream commit, applies temporary diagnostic/experiment
  patches, builds the standalone recompiler, and generates baseline/configured
  cycle-backend C in temporary directories. It validates tail diagnostics
  against NESRecomp's counts; no generated C is committed or uploaded.
- `tools/nesrecomp-tail-diagnostic.patch`: prints only entries seen by analysis
  but not compiled, without printing or publishing generated source.
- `tools/nesrecomp-data-region-vector.patch` and
  `tools/nesrecomp_smb_v026_data_region_experiment.toml`: a controlled test of
  honoring an explicit data-region annotation while seeding vectors. These are
  experiments, not approved product code/config.
- `.github/workflows/nesrecomp-evaluation.yml`: runs the harness on demand and
  for relevant changes.
- This evidence/decision document and updated project continuity docs.

Generated C is intentionally **not committed or uploaded as an artifact** by
this first-stage harness. It is derived from the ROM and should not be
redistributed by default. The harness also does not vendor upstream source.

## License and provenance

The upstream NESRecomp repository's current `LICENSE` is PolyForm
Noncommercial 1.0.0. It explicitly allows personal noncommercial research,
experimentation, testing, and hobby use, but commercial use is not granted by
that license. The framework's dependencies have separate licenses. Review the
exact pinned revision and all dependency notices before copying or modifying
upstream source inside this repository.

The existing project rule remains in force: do not adopt, copy, or migrate the
third-party Super Mario Bros. port as the product base. This evaluation invokes
the generic recompiler against our own ROM; it does not use the third-party
game project's generated code, game hooks, patches, or renderer.

## Actual first-stage result — CI success

Run [39186469481](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186469481)
reported:

- Canonical ROM hash and size passed: 40,976 bytes, SHA-256 unchanged.
- NESRecomp pinned revision fetched and built successfully with GCC 13.3/CMake/Ninja.
- Parsed as 2 PRG banks × 16 KiB, mapper 0; vectors NMI `$8082`, RESET `$8000`, IRQ `$FFF0`.
- Default cycle backend reported **1,305 instructions compiled and 1 left to the interpreter** because it crosses a 4 KiB bank boundary or `$FFFF`.
- Generated **5 C files**, totaling **545,655 bytes / 14,397 lines**. They were kept in a temporary directory and were not committed or uploaded.
- Upstream compiler emitted several C warnings (const qualifier and possible `snprintf` truncation); they did not fail its build.

**Interpretation:** this is a real positive result for the feasibility question: NESRecomp can parse our exact ROM and generate substantial cycle-backend C. It is not yet acceptable as the final architecture because the log explicitly reports one interpreter fallback. The next step must identify the exact fallback target and determine whether it can be compiled natively without retaining an interpreter. No generated runner was built or executed, and no Xbox/XEX test occurred.

## Second-cycle finding — single interpreter tail (19/10/2026)

The latest evaluation run [39186568820](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186568820) completed successfully on the current PR head. Inspection of the pinned cycle generator shows that its `tails` count is computed from positions visited by discovery (`seen`) but not marked compilable (`!is_insn`); the current summary reports only the count, not individual addresses.

The temporary CI diagnostic now confirms the sole tail precisely:

```
[TAIL_DIAGNOSTIC] bank=7 slot=7 addr=$FFFF opcode=$FF len=3
```

Run [39207665461](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39207665461) completed **success** with the patch applied only to the temporary upstream checkout. The reported tail count (1) matches the single diagnostic record; generated C metrics remain unchanged, and generated C was not committed or uploaded.

The canonical PRG bytes at CPU `$FFFA-$FFFF` are `82 80 00 80 F0 FF`; the IRQ vector at `$FFFE-$FFFF` therefore points to `$FFF0`. NESRecomp's cycle backend seeds all three interrupt vectors and follows the decoded control flow. It then walks through the byte sequence beginning at `$FFF0` and eventually marks `$FFFF` as a seen instruction start, but cannot compile the three-byte `$FF` opcode because its bytes would wrap past `$FFFF`. The chain starts from the IRQ-vector seed; given the vector/table byte layout, this is strong evidence of a false code-discovery path into data/vector bytes, not evidence that gameplay code needs an interpreted instruction. The earlier `$9FFF` hypothesis was wrong and is superseded.

## Controlled data-region experiment — CI success

Run [39208412755](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39208412755) passed with two generation runs against the same verified canonical ROM:

- **Baseline (pinned upstream behavior, plus telemetry only):** 1,305 instructions compiled; one tail at bank 7 / slot 7 / `$FFFF` (`FF`, length 3).
- **Configured experiment:** 1,298 instructions compiled; **zero interpreter-tail candidates**. The log shows `[VECTOR_DATA_REGION] vector=$FFFE target=$FFF0 bank=7 skipped`.

The second result used a temporary patch, `tools/nesrecomp-data-region-vector.patch`, that makes cycle-backend vector seeding respect the already parsed `[[data_region]]` annotation, plus `tools/nesrecomp_smb_v026_data_region_experiment.toml` declaring bank 7, `$FFF0-$FFFE` as data. This exposes a gap in the pinned upstream cycle backend: its current vector seeding does not consult `data_regions`; the configuration alone would not currently stop the walk. The annotation suppresses the IRQ-vector path, whose bytes have the form of a data table and lead to the false tail. Both source patches are applied only to the temporary checkout by CI; no upstream source or generated C is vendored or published.

**Interpretation:** the reported single tail is not necessary for the configured static code walk when the IRQ target is explicitly classified as data. This is progress, not architecture approval. It does not prove the IRQ path can never occur, and intentionally omits the code from `$FFF0`; if a real IRQ arrives there, this generated build would no longer contain a handler for it. Before treating the annotation as correct, verify from the canonical ROM and runtime behavior that the title does not rely on IRQ service, and build/test a runtime with the interpreter entirely removed (not merely unused). No runner execution, no differential gameplay test, no XEX and no Xbox test occurred.

## Follow-up telemetry validation — CI success (23/10/2026)

Commit `32c22083f0e936ff5284134c624b4aeefdc96c24` passed both [push run 39923036457](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39923036457) and [PR run 39923040038](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39923040038). The temporary state profiler now avoids a line for every decrement of `$0747`: it records discrete-state changes, one periodic snapshot per 60 frames, and low-value changes when the counter reaches `$05–$00`.

- The CI log shrank from 2,315,428 to 431,490 bytes (about 81% smaller), while the harness still ran all seven deterministic scenarios.
- All **24,600** sampled frames matched the reference state/trace hashes; PCM/WAV matched byte-for-byte in all seven; zero `NATIVE_COVERAGE_MISS`; the interpreter source unit remains out of `build.ninja` and `cpu_interp_step` remains absent from runner symbols.
- In death/reset states, `$0747=$30` appears alongside `game_task=$06`, then the next observed reset state has `$0747=$00` and `game_task=$00`. This confirms the low-value sampling is active, but does not prove the `$0747==0` condition was met on the object dispatcher path at `$C987`.
- The summary line in `tools/evaluate_nesrecomp.sh` previously said “all six deterministic scenarios” despite running seven. This commit corrects the displayed count to seven; it does not alter the test route.

Dynamic selector coverage remains unchanged at **62 pairs / 12 callsites / 50 targets**, with all 16 priority selectors still unobserved. The weighted project estimate remains **22%**; no XEX/Xenon output or physical Xbox test.

## Latest directed gameplay/dispatch profile — CI success (23/10/2026)

The test commit `3c64fcad12e36d380b6972365c3a77ad129fe3dc` passed both the push run [39921081582](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39921081582) and PR run [39921086708](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39921086708). The run added a 3,600-frame `obstacle` sequence that alternates 28 frames of `RIGHT+A+B` with 32 frames of `RIGHT+B`; the six previous scenarios were retained as regressions.

- **24,600 total frames** were compared across seven scenarios (idle 600, route 600, jump 1,800, long 3,600, extended 9,000, run-only 5,400, obstacle 3,600).
- Every per-frame state/trace hash matched the interpreter-enabled reference. PCM/WAV matched byte-for-byte in all seven scenarios; no `NATIVE_COVERAGE_MISS` occurred.
- Runtime structure was verified: `cpu6502_interp.c` absent from `build.ninja`, and `cpu_interp_step` absent from the executable symbols.
- The new input timing did **not** resolve the gameplay obstacle. In its reference profile, the player reached page 2/`x=$C6` at frame 775, then fell/reset at frames 985, 1763 and 3087. Lives remained `$08`; no level completion or Game Over was demonstrated.
- The dispatch profile remained **62 unique (callsite, selector) pairs, 12 callsites, 50 destinations**. The 16 priority selectors remain unobserved: `$8215:2/3`, `$9664:1`, `$C27F:18/20/21/22/23`, and `$C88F:16/17/18/19/20/21/22/29`.
- `$0746` remained `$00` in observed snapshots. `$0747` changed as a frame-countdown during `game_task=$0B`, flooding change-triggered snapshots. The follow-up instrumentation adjustment in the current commit removes the countdown byte from generic change detection while preserving samples every 60 frames and when `$0747` changes through `$00–$05`. This adjustment itself remains pending CI validation.

This is evidence of deterministic parity for these seven sampled host routes only—not full-ROM coverage, game completion, Xenon/PowerPC output, an XEX or physical Xbox compatibility. The weighted project estimate remains **22%**.

## First-stage acceptance criteria

1. The canonical SHA-256 check passes.
2. The pinned upstream commit is fetched exactly.
3. The standalone recompiler builds on the Linux CI host.
4. It parses the canonical ROM and reports mapper/PRG/vectors.
5. Cycle-backend C source files are generated and non-empty.
6. Generated-source metrics are recorded by CI without publishing the code.

Passing these criteria establishes only that generation is technically
possible. It does **not** prove complete function discovery, successful
compilation of the generated game with its runner, full native coverage,
removal of interpreter fallback, or Xbox 360/XEX compatibility.

## Follow-up gates — do not skip

1. Inspect the generation report for mapper, reset/NMI/IRQ vectors, discovered
   functions, and warnings. Compare the generated source inventory with the
   canonical ROM's PRG layout.
2. Determine whether dynamically dispatched code, indirect jumps, data/code
   overlap, and modified routines require project-specific `game.toml`
   annotations. Do not borrow the SMB port's code/config as the product base.
3. Build the generated output and runner on the host, then run deterministic
   scenarios against our existing canonical reference. Record any divergence.
4. Measure native-versus-fallback execution. NESRecomp documents a generated
   interpreter fallback for code outside compiled ROM views. The final product
   requirement forbids that fallback at runtime; do not claim success unless
   coverage is proven and fallback execution is absent or disabled safely.
5. Assess runner dependencies and replace the desktop platform layer only after
   the above works. SDL2, desktop entry points, filesystem assumptions, and
   platform APIs must be audited before a Xenon/PowerPC build is attempted.
6. Only then decide whether to adapt NESRecomp, combine permitted generic
   components, or resume our own translator.

## Reproduce

From the repository root on a Linux host with Git, CMake, Ninja, a C compiler,
and network access:

```sh
bash tools/evaluate_nesrecomp.sh
```

The local container could not resolve GitHub from its shell. The GitHub Actions
run [39186504796](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186504796)
completed successfully on the evaluation branch, confirming the upstream build
and generation test. The earlier run [39186469481](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186469481)
contains the full generation metrics below. The latest run at the time of this
update also passed after removing unnecessary apt installation from the workflow.

## Ciclo de trabalho — remoção de interpretador e teste de cobertura (20/10/2026)

O experimento avançou além da geração C:

- A análise configurada passou a gerar **4.849 instruções** no relatório do backend cycle, sem candidato de cauda interpretada. A configuração experimental anota destinos de despacho inline encontrados no PRG e a região de vetor/tabela IRQ como dados; esses seeds são hipóteses de análise, não uma configuração final aprovada.
- O harness adaptou somente a cópia temporária do upstream para separar a tabela de metadados de comprimento de opcode do arquivo do interpretador e excluí-lo do runtime host experimental.
- A inspeção de build confirmou `cpu6502_interp.c` ausente do `build.ninja` e `cpu_interp_step` ausente dos símbolos do executável. Portanto, a remoção do arquivo/símbolo foi demonstrada para esse build experimental.
- **O teste fail-fast não passou:** execução do runner parou em `[NATIVE_COVERAGE_MISS] pc=$9B0E bank=1`. Isso significa que a análise estática ainda não cobriu pelo menos um endereço realmente alcançado no cenário de inicialização/teste; não se pode declarar o jogo executável sem interpretador. As tentativas seguintes encontraram outros destinos descobertos de despacho inline, mostrando que simplesmente adicionar seeds manualmente ainda não fecha a cobertura.
- Run mais recente consultado: [39210393377](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39210393377), falha esperada no gate de cobertura, com o interpretador ausente do binário. O log registra a falha de cobertura em `$9B0E`. Nenhum C gerado foi publicado.

### Porcentagens de progresso — estimativas por gate (20/10/2026)

| Frente | Progresso | Evidência/critério |
|---|---:|---|
| ROM canônica e reprodutibilidade da entrada | 100% | SHA-256/40.976 bytes verificados pelo harness |
| Construção do recompilador genérico no host CI | 100% | upstream pinado compila |
| Geração estática C a partir da ROM | 100% da prova de geração | milhares de instruções e arquivos C temporários; não é jogo validado |
| Remoção do interpretador no runtime experimental | 100% da prova estrutural do build atual | arquivo/símbolo ausentes; cobertura ainda falha |
| Cobertura de fluxo sem interpretador | 35% | já há diagnóstico e trap fail-fast, mas o cenário alcança `$9B0E` sem código nativo |
| Runner host integrado + execução reproduzível do jogo | 10% | build/instrumentação inicial; execução interrompida pela falha de cobertura |
| Equivalência funcional (estado/quadros/áudio vs. referência) | 0% | nenhum teste diferencial completo |
| Port para Xbox 360/Xenon/PowerPC e XEX final | 0% | não iniciado/aprovado |
| Validação física no Xbox 360 | 0% nesta etapa | nenhum XEX novo; não solicitar teste físico ainda |
| **Projeto completo: port funcional, sem interpretador e validado** | **10%** | ainda não existe jogo nativo funcional comprovado |

Esses percentuais são estimativas de marcos, não uma medição de linhas de código. O número geral permanece baixo porque a parte difícil é a cobertura e fidelidade de runtime, não gerar C. Não somar as frentes como se tivessem o mesmo peso.

### Próxima ação única

Parar de adicionar destinos manualmente em sequência. Instrumentar o trap para registrar PC, banco, opcode e origem da última transferência de controle; depois rastrear por que `$9B0E` é alcançado e melhorar a descoberta genérica de fluxo/despacho inline. Reexecutar a cobertura host até não existir `NATIVE_COVERAGE_MISS` em cenários determinísticos. Só então avançar para comparação de quadros/estado. Sem XEX e sem hardware.


## Novo ciclo — diagnóstico de despacho indireto e cobertura orientada por execução (20/10/2026)

Run [39417601965](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39417601965) terminou **success** para o harness experimental do PR #20. Esta é a primeira execução bem-sucedida do runner sem interpretador em dois roteiros determinísticos, sem PC nativo ausente nesses roteiros.

O trap mostrou o miss inicial em $9061, opcode $A9, alcançado pela transferência [NATIVE_LAST_TRANSFER] from=$8E16 opcode=$6C to=$9061. O opcode $6C é JMP indireto. O trecho $8E04–$8E16 é o dispatcher inline; o destino é calculado em runtime, razão pela qual a análise estática inicial não encontrou todos os alvos.

O harness agora aplica cobertura guiada por execução apenas à cópia temporária do TOML, adicionando o PC ROM que causou cada miss como seed, regenerando e recompilando o runner sem interpretador. Há limite de 24 adições para impedir loop infinito. O TOML versionado e a ROM canônica não são modificados. Oito endereços foram adicionados à cópia temporária: $9061, $8245, $AEDC, $AEEA, $B069, $B376, $C328 e $C9D8.

Resultado do run 39417601965:
- Idle/título por 600 quadros: 17.865.920 ciclos; 17.865.913 ciclos nativos (100,0%); zero instruções ROM/RAM interpretadas.
- Entrada roteirizada por 600 quadros: 17.865.925 ciclos; 17.865.918 ciclos nativos (100,0%); zero instruções ROM/RAM interpretadas.
- Inspeção do executável: cpu6502_interp.c ausente do build.ninja e cpu_interp_step ausente dos símbolos.
- Geração C: 5 arquivos, 545.655 bytes, 14.397 linhas; código gerado permaneceu temporário e não foi publicado.

**Interpretação limitada:** isso prova execução nativa sem fallback nos dois roteiros testados, não a cobertura integral da ROM, jogabilidade correta em geral ou equivalência com a referência. Nenhum teste diferencial de quadros/estado/áudio foi executado; compatibilidade Xenon/XEX não começou.

Estimativa geral: **12%**, contra 10% anteriormente, somente pelo resultado de execução amostrada sem interpretador. Próxima tarefa é comparar os mesmos inputs e a mesma ROM/configuração entre runner normal com interpretador e build sem interpretador, usando --hash-out por quadro; depois ampliar gameplay e transições. Não gerar XEX nem pedir teste físico.


## Novo ciclo — equivalência por quadro entre runtime nativo e referência (20/10/2026)

Run [39418368074](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39418368074) terminou **success** e realizou o próximo gate planejado: comparação do runner experimental sem interpretador contra o runner normal com interpretador habilitado.

As duas variantes usaram a ROM canônica. O runner nativo recebeu os oito destinos derivados da cobertura; o runner de referência usou a configuração experimental original, sem esses destinos adicionais, de forma que o caminho de fallback foi exercitado.

**Resultados de --hash-out:**
- Idle/título, 600 quadros: os hashes por quadro coincidiram em todas as linhas. A referência utilizou 17.152 ciclos ROM interpretados (0,1%); o build nativo executou 17.865.913 dos 17.865.920 ciclos em código nativo (100,0%).
- Entrada roteirizada, 600 quadros: os hashes por quadro também coincidiram em todas as linhas. A referência utilizou 1.145.069 ciclos ROM interpretados (6,4%); o build nativo executou 17.865.918 dos 17.865.925 ciclos em código nativo (100,0%).
- Os hashes comparados incluem trace observável, memória/pixels, estado dos registradores CPU e estado interno de hardware. A saída PCM/WAV não teve uma comparação waveform independente nesta etapa.
- O build sem interpretador permaneceu sem o arquivo cpu6502_interp.c no build.ninja e sem o símbolo cpu_interp_step no executável.
- A experiência completa exigiu oito seeds temporários em endereços alcançados por JMP indireto compartilhado: $9061, $8245, $AEDC, $AEEA, $B069, $B376, $C328, $C9D8. Esses endereços não foram persistidos no config versionado e o código C não foi publicado.

**Conclusão:** paridade observável comprovada em 1.200 quadros distribuídos em dois roteiros determinísticos; não significa equivalência em todo o gameplay, todas as rotas de despacho ou áudio PCM. Progresso global estimado: **15%**. Próximo passo: ampliar cenários e duração, depois comparar PCM/áudio. Sem XEX/teste físico.


## Novo ciclo — matriz ampliada (20/10/2026)

Run [39418927424](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39418927424) terminou **success**. O harness executou uma matriz de quatro cenários com o interpretador removido do runner e comparou as capturas --hash-out contra a variante de referência com interpretador habilitado.

| Cenário | Quadros | Ciclos totais | Ciclos nativos no runner sem interpretador | Ciclos ROM interpretados na referência | Hashes por quadro |
|---|---:|---:|---:|---:|---|
| Idle/título | 600 | 17.865.920 | 17.865.913 (100,0%) | 17.152 (0,1%) | Iguais em todos os quadros |
| Rota de input curta | 600 | 17.865.925 | 17.865.918 (100,0%) | 1.145.069 (6,4%) | Iguais em todos os quadros |
| Movimento/pulo | 1.800 | 53.602.527 | 53.602.520 (100,0%) | 4.788.327 (8,9%) | Iguais em todos os quadros |
| Input estendido | 3.600 | 107.207.435 | 107.207.428 (100,0%) | 8.937.607 (8,3%) | Iguais em todos os quadros |

A matriz final soma **6.600 quadros**. O build nativo confirmou ausência de cpu6502_interp.c na lista do Ninja e ausência de cpu_interp_step nos símbolos do executável. A referência usou o interpretador, sobretudo durante rotas mais longas; ainda assim, os hashes por quadro coincidiram com o build nativo. Os hashes cobrem trace observável, memória/pixels, CPU e estado de hardware.

Foi necessário adicionar **15 seeds temporários** à cópia de configuração durante as tentativas de cobertura: $9061, $8245, $AEDC, $AEEA, $B069, $B376, $C328, $C9D8, $BC85, $B245, $B269, $91CD, $C2F1, $B36D e $860D. A configuração de referência versionada não foi alterada. C gerado ficou no diretório temporário.

**Limites:** os cenários são determinísticos e amostrados, não a totalidade do jogo. Não foi comparado o waveform PCM/WAV de forma independente. Nenhuma compatibilidade Xbox/XEX foi testada. Estimativa de progresso do port: **18%**.

Próximo passo: caracterizar genericamente o dispatcher indireto compartilhado que produz os alvos, expandir a diversidade de estados e comparar PCM/WAV. Sem XEX ou teste físico.


## Ciclo S024 — verificação do dispatcher e PCM, 20/10/2026

**Run CI:** [39432801245](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39432801245) — **success**.

A abordagem anterior de transformar todos os PCs do miss-log do interpretador em seeds foi rejeitada. O run [39429365074](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39429365074) encontrou 2.072 PCs porque o log incluía instruções sequenciais interpretadas, não apenas novas entradas de fluxo.

O novo profiler observa a entrada do dispatcher inline em `$8E04`, registra o endereço de retorno empilhado pelo JSR, o seletor em A, a posição da palavra inline e o destino decodificado. `tools/verify_nesrecomp_dispatch_profile.py` verifica cada registro diretamente contra a ROM canônica.

### Resultado do perfil

- ROM: hash `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`, mapper 0, PRG 32.768 bytes.
- Varredura do PRG: **18 candidatos brutos** `JSR $8E04`; os roteiros testados exercitaram **12 callsites**.
- Perfil e validação direta: **57 pares únicos callsite/seletor**, **47 destinos ROM únicos**; bytes de chamada e palavras de tabela verificadas — PASS.
- **17 seeds novos** foram aplicados somente ao TOML temporário: `$8245`, `$860D`, `$9061`, `$9071`, `$91CD`, `$AEDC`, `$AEEA`, `$B069`, `$B0E9`, `$B245`, `$B269`, `$B36D`, `$B376`, `$BC85`, `$C2F1`, `$C328`, `$C9D8`. A ROM canônica e a configuração versionada ficaram intactas.
- Inventário dos 57 registros verificados: [INLINE_DISPATCH_PROFILE_20261020.md](INLINE_DISPATCH_PROFILE_20261020.md).

### Recompilação e comparação

O executável experimental não inclui `cpu6502_interp.c` no `build.ninja` e não contém o símbolo `cpu_interp_step`. Os quatro roteiros passaram sem `NATIVE_COVERAGE_MISS` e executaram 100,0% dos ciclos nativamente:

| Cenário | Quadros | Ciclos totais | Ciclos nativos |
|---|---:|---:|---:|
| Idle/título | 600 | 17.865.920 | 17.865.913 (100,0%) |
| Rota curta | 600 | 17.865.925 | 17.865.918 (100,0%) |
| Movimento/pulo | 1.800 | 53.602.527 | 53.602.520 (100,0%) |
| Rota longa | 3.600 | 107.207.435 | 107.207.428 (100,0%) |

Os hashes `--hash-out` coincidiram por quadro entre o build sem interpretador e a referência em **6.600 quadros**. O PCM dos WAVs coincidiu byte a byte, com formato idêntico de 48 kHz, mono, 16-bit:

| Cenário | Amostras PCM | SHA-256 |
|---|---:|---|
| Idle/título | 479.146 | `2cef570772cd9740096484f400d2119e9f1e04e6ff84f0bedc6b85f1377e108c` |
| Rota curta | 479.147 | `74abe61c0278cb3368c448cbaace9138cdbb8e595b05324459ee0a500784d342` |
| Movimento/pulo | 1.437.568 | `b03e2bd423f2456d5b5872d6a6d3263ae4ef16f6f7845e0aa5792158c4629ee9` |
| Rota longa | 2.875.201 | `bf45771e7c88f372c0e19e73fb5468c9c0f92eda9d1b9d258881ce3ff216e67a` |

### Conclusão e limites

A descoberta de fluxo agora é guiada por chamadas reais ao dispatcher e cada alvo observado é validado contra a ROM; porém, ainda não é uma análise estática exaustiva. Seis dos 18 candidatos brutos a `JSR $8E04` não apareceram nos cenários testados, e seletores/estados não percorridos podem existir. Não declarar cobertura integral nem jogabilidade completa.

A evidência de equivalência em 6.600 quadros mais PCM byte a byte aumenta a estimativa funcional de 18% para **20%**. Port Xenon/PowerPC, XEX e teste físico continuam em 0%. C gerado continua temporário, PR #20 segue aberto e PR #19 continua pausado. Nenhum XEX foi criado.

### Próxima etapa

Enumerar os 18 callsites candidatos e suas tabelas inline, identificar os seis ainda não observados e criar cenários determinísticos para alcançá-los quando a análise do PRG indicar caminhos válidos. Repetir os quatro roteiros atuais como regressão, com gates de cobertura fail-fast, hash por quadro e PCM. Sem XEX/teste físico.


## Follow-up — static dispatcher path verification and expanded scenarios (22/10/2026)

A ROM-specific checker, `tools/analyze_unobserved_dispatchers.py`, now verifies the six previously unprofiled dispatcher callsites against the exact canonical ROM, including the destination words in their parent inline tables and the intermediate bytes/conditions. It is invoked by `tools/evaluate_nesrecomp.sh` and included in the workflow path filters.

CI run [39775826447](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39775826447) completed **success** with the checker integrated. The checker validated all six static paths. The then-current four host scenarios also passed in the no-interpreter runner, with 6,600 frames of per-frame hashes matching and PCM/WAV identical byte-for-byte. The report explicitly labels the six target paths as *not yet dynamically observed*.

The evaluator was subsequently expanded with two deterministic exploration scenarios:
- `extended`: 9,000 frames, periodic running/jump input, intended to explore more of the level and object-state paths;
- `run-only`: 5,400 frames of running input, intended to exercise a different gameplay/death route.

These scenarios are exploratory rather than guaranteed triggers for the missing selectors. Their acceptance still depends on CI results: the interpreter-enabled profile must record any newly reached parent callsite/selector; then the no-interpreter runner must complete without a coverage miss and pass frame-hash and PCM/WAV parity. No results are claimed for this expanded matrix until its workflow run completes.

The static checker and additional inputs do not change the canonical ROM, do not modify permanent seed/configuration files, and do not produce an Xbox executable. The weighted project estimate remains 22% pending new dynamic evidence.

## Six-scenario host differential result — 23/10/2026

CI [39778074358](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39778074358) passed on commit `6e94f561f7274f0368929e1c73b0e1c22e0863d0`. Six deterministic scripts ran: idle (600 frames), route (600), jump (1,800), long (3,600), extended (9,000), and run-only (5,400), totalling **21,000 frames**.

For every script, the experimental runner was rebuilt without `cpu6502_interp.c`; the build manifest and executable symbol table confirmed no `cpu6502_interp.c` translation unit and no `cpu_interp_step` symbol. Each no-interpreter scenario reported 100.0% native cycles and completed without a coverage miss. Per-frame state/trace hashes matched the interpreter-enabled reference for all 21,000 tested frames; 48 kHz mono 16-bit PCM/WAV metadata and payload were byte-identical for each scenario. The five generated C files (545,655 bytes / 14,397 lines) remained in a temporary CI directory and were not uploaded or committed.

The expanded dispatcher profile recorded **62 unique (callsite, selector) pairs across 12 callsites and 50 destinations**, compared with 57 pairs / 47 destinations in the earlier four-scenario report. The 16 statically prioritized pairs still did not appear: `$8215:2`, `$8215:3`, `$9664:1`, `$C27F:18/20/21/22/23`, and `$C88F:16/17/18/19/20/21/22/29`. The static graph remains a hypothesis for those dynamic paths, not coverage evidence.

**Interpretation:** sampled host-route confidence improved, but the weighted estimate remains 22% because no new target path was dynamically confirmed. These tests do not prove full-ROM reachability/correctness, Xenon/PowerPC compatibility, Xbox API integration, XEX boot, physical playability, or acceptance of the upstream noncommercial license.

## Next instrumentation step — pending CI

`tools/prepare_nesrecomp_dispatch_profile.py` is being extended to emit temporary `[SMB_STATE]` snapshots at frame boundaries whenever relevant state changes and every 60 frames. Fields include mode/subtask, world/area, lives, player coordinates, and enemy IDs. The purpose is to build directed routes for victory/game-over, area style, and enemy/object-specific dispatchers rather than simply lengthening the same input sequence. Treat this as pending until the CI triggered by that script change passes and its output is inspected. No XEX and no physical test are authorized at this stage.

## State telemetry result and jump-timing test — 23/10/2026

CI [39919520366](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39919520366) passed after introducing temporary `[SMB_STATE]` snapshots. The six previous scenarios still ran 21,000 frames without the interpreter and matched per-frame hashes and PCM/WAV byte-for-byte against the reference.

Snapshots show that existing scripted inputs enter gameplay but repeatedly reach around player page 2 / X=$D2, stall or fall, enter game subtask `$06`, and reset the stage. Lives stayed at `$08`; neither level completion nor Game Over was demonstrated. Values in object-ID slots `$0016-$001B` included `$02, $05, $06, $0E, $12, $18, $2E`. Although `$12` appeared in those RAM slots, the profiler did not observe `$C27F` selector 18. The contents of an object-ID slot therefore do not by themselves prove a dispatch branch executed.

The next evaluation update adds `$0746` and `$0747` to telemetry to inspect the conditions for `$D2E5` and `$C987`. It also adds a separate `obstacle` route alternating 28-frame RIGHT+A+B phases and 32-frame RIGHT+B phases starting after gameplay entry. Existing six routes remain unchanged as regression. Treat the experiment as pending until CI and per-frame/PCM differential checks pass. No XEX or console test.
