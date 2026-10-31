# CHECKPOINT ATUAL — oitavo cenário aprovado; progressão aumentou (23/10/2026)

Branch: research/nesrecomp-evaluation-20261018  
PR ativa: [#20](https://github.com/PedroMarioaBros/SMNEStoX360/pull/20)  
CIs de código: push [39990420329 — SUCCESS](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39990420329), PR [39990426882 — SUCCESS](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39990426882), commit 7f4d0cae87d6a804c4aa2d8d150fbd1c4ee71ef7.

- Oito cenários passaram: 28.200 quadros, oito conjuntos de hashes por quadro e oito saídas PCM/WAV idênticas byte a byte. Zero NATIVE_COVERAGE_MISS; cpu6502_interp.c e cpu_interp_step ausentes do runner experimental.
- obstacle-phase chegou à página 4/x=$DC, mundo-X 1244, quadro 1080: 534 pixels além do máximo anterior. Houve reinicializações nos quadros 1234 e 2926; vidas $08; não há conclusão de fase provada.
- O perfil subiu de 62 para 64 pares, 12 callsites e 52 destinos. Pares novos: $9664:8 -> $9B41 e $9664:25 -> $9B01.
- $C907:18 foi observado com ID $12 ativo em slots diferentes. Isso comprova o despacho de tipo, mas $C27F:18 na inicialização permanece sem observação. Os 16 seletores prioritários seguem sem confirmação no callsite específico.
- Progresso ponderado: 22,4% (P3 16%; P5 62%). ROM intacta; C gerado temporário; sem build Xenon/PowerPC, XEX ou teste físico.

Próximo gate: instrumentar coordenadas em maior resolução somente quando página >= 3 ou game_task=$0B, localizar o bloqueio em mundo-X 1244 e ajustar os inputs com base nos quadros exatos. Em paralelo, investigar $9664:8/25 e a rota legítima para $9664:1.

---

## Checkpoint anterior — estudo de viabilidade NESRecomp (19/10/2026)

A pedido do proprietário, a expansão do tradutor próprio está temporariamente
pausada para avaliar o framework genérico NESRecomp usando
`assets/canonical/SMB_v026.nes`. O código de POC existente é preservado, não
foi apagado e não se declara que o tradutor atual está completo.

- Plano: [NESRECOMP_EVALUATION.md](evidence/NESRECOMP_EVALUATION.md).
- Script repetível: `bash tools/evaluate_nesrecomp.sh`.
- CI: [NESRecomp canonical ROM evaluation](../.github/workflows/nesrecomp-evaluation.yml).
- Commit upstream fixado: `a39d9cdbec0ac98b1430ddb53747b5ed8f8cac87`.
- A avaliação verifica o SHA-256 da ROM, compila o gerador C do upstream,
  executa a análise estática e confirma a geração de arquivos C no diretório
  temporário. O código gerado não é commitado nem publicado como artifact. Resultado medido: mapper 0, PRG 2×16 KiB, vetores NMI `$8082`, RESET `$8000`, IRQ `$FFF0`; 1.305 instruções compiladas, 1 deixada para interpretador, 5 arquivos C, 545.655 bytes e 14.397 linhas.
- **Resultado da geração: CI SUCCESS** em [run 39186504796](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186504796). O run de métricas [39186469481](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186469481) confirma build do gerador e análise da ROM. O container local não resolve github.com, então a validação ocorreu no runner remoto.
- A licença upstream consultada é PolyForm Noncommercial 1.0.0; o uso de
  pesquisa/hobby não comercial está contemplado, mas redistribuição comercial
  não está autorizada pela licença atual. Dependências têm licenças próprias.
- Regra intacta: não copiar/adotar o port SMB de terceiros como base. O teste
  usa apenas o recompilador genérico com nossa ROM canônica.
- Risco crítico: NESRecomp documenta fallback interpretado para código não
  recompilado. O requisito final não permite interpretador 6502 em runtime.
  A instrumentação temporária confirmou a única cauda em bank 7, slot 7,
  CPU `$FFFF`, opcode `$FF`, tamanho 3. Run [39207665461](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39207665461) passou e contou exatamente 1 entrada, igual ao sumário do gerador.
- Contexto: bytes `$FFFA-$FFFF = 82 80 00 80 F0 FF`; vetor IRQ = `$FFF0`.
  O backend cycle semeia os vetores incondicionalmente e a caminhada a partir do
  destino IRQ alcança `$FFFF` ao tratar bytes de vetor/tabela como código.
- Controle em [run 39208412755](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39208412755): upstream pinado + telemetria reproduz 1 tail e 1.305 instruções. Um patch experimental temporário passou a respeitar `[[data_region]]` ao semear vetores; a configuração de teste declara bank 7, `$FFF0-$FFFE`, como dados. Resultado: vetor IRQ `$FFF0` pulado por anotação, 1.298 instruções compiladas e zero tails. Esta é evidência de que a anotação evita a descoberta falsa, não prova de que o jogo nunca usa IRQ.
- Patches e config do experimento: `tools/nesrecomp-tail-diagnostic.patch`,
  `tools/nesrecomp-data-region-vector.patch` e
  `tools/nesrecomp_smb_v026_data_region_experiment.toml`. Aplicados apenas à
  cópia temporária upstream. C gerado permanece não commitado/não publicado.
  Próximas etapas: verificar uso real de IRQ e comportamento de $4017; preparar
  runtime com trap em qualquer PC não compilado e remover totalmente o
  interpretador 6502; provar no host runner que o jogo executa e comparar
  estados/quadros determinísticos com a referência. Sem XEX nem teste físico.
- PR #19 permanece aberta e sem merge durante a avaliação:
  https://github.com/PedroMarioaBros/SMNEStoX360/pull/19

**Critérios deste checkpoint:** não confundir geração C com código executável
completo, runner validado, destino Xenon/PowerPC, XEX ou gameplay físico. Nenhum
XEX foi gerado e nenhum novo teste físico foi solicitado.

---

# Checkpoint atual — POC-001 de tradução estática criada (18/10/2026)

A PR [#18](https://github.com/PedroMarioaBros/SMNEStoX360/pull/18) foi integrada à `main` por squash no commit `d0822da58767ec8ee078752694e6d679445089dd`. A CI host exata do HEAD da PR passou no run [39183794667](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39183794667); a execução com o código tradutor também passou no run 39183681786. A primeira prova de conceito ROM-específica: `tools/static6502_poc.py`. Ela valida o SHA-256 canônico, extrai o PRG e converte diretamente seis instruções do reset ($8000-$8009) em statements C, sem despacho de opcode em runtime. O self-test compila C com `-std=c99 -Wall -Wextra -Werror -pedantic`, verifica registros/flags/escrita de barramento e exige rejeição do opcode $AD em $800A. O rascunho equivalente local passou; a branch inclui o self-test no script de regressão host para verificação pela CI. Evidência: [POC-001](evidence/STATIC_RECOMPILER_POC_001.md).

**Limite explícito:** isto NÃO é decompilação completa nem jogo funcional. Só seis instruções lineares; ainda sem branches, leitura/mapeamento de barramento NES, análise de dados misturados, teste diferencial de traces, build PowerPC/XEX ou teste físico. Não gerar XEX nem pedir teste de hardware. Próximo passo é expandir o decodificador/tradutor incrementalmente e fazer CI/host tests da branch.

---

# ORDEM DO PROPRIETÁRIO — 18/10/2026: destino nativo sem emulação

A direção definitiva é recompilar estaticamente a ROM específica `assets/canonical/SMB_v026.nes` para um executável nativo Xbox 360. A tradução CPU 6502→C e compilação para PowerPC/Xenon devem substituir a interpretação em runtime no produto final; não adotar um porte SMB de terceiros como base. O runtime C99 atual continua útil apenas como referência host para testes diferenciais. Ler [docs/NATIVE_RECOMPILATION_POLICY.md](NATIVE_RECOMPILATION_POLICY.md), [AGENTS.md](../AGENTS.md) e [próximas ações](NEXT_STEPS.md).

**Próximo trabalho:** inspeção offline do código/branches e prova de conceito incremental de tradutor estático 6502→C, preservando a ROM e toda evidência histórica. Nenhum novo XEX nem teste físico está autorizado automaticamente: S015/S018 tiveram Fatal Crash; S017 congelou/retornou sem validar sucesso. A causa de crash anterior não deve ser inventada.

**Estado honesto em 18/10/2026:** não foi confirmada nesta sessão uma descompilação estática implementada nem um XEX jogável. O checkpoint anterior ainda descreve o runtime interpretado e o bloqueio de hardware; a nova política altera o objetivo/ordem de trabalho, não apaga esse histórico.

---

# Checkpoint operacional — 17/10/2026 (S019)


**Prevenção de reteste acidental:** S019 também aposentou a publicação
de `smb360-S018-STATUS-NATIVE-XEX` no workflow
`.github/workflows/xex-status.yml`. O CI ainda compila apenas
para investigação, emite aviso de Fatal Crash e **não envia artifact
executável S018 para novos downloads**. Pacote antigo segue vetado.


## RESULTADO FÍSICO S018 = FATAL CRASH (NÃO REPETIR)

Proprietário reportou literalmente **"S018 resultou em Fatal
Crash Detected"** após executar o `default.xex` S018 no
Xbox 360. Isto suplanta TODO texto histórico anterior
de S018 "não testado". Não há relato de toast ou tempo
até o erro; não inventar tais observações. **S018 FALHOU
FISICAMENTE**. S015 também deu Fatal Crash; S017 01
congelou em tela preta e S017 02 reabriu Aurora sem
prova de CPU PASS. Não reexecutar nenhum deles.
Registro: [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).

### S019 — comparação direta dos arquivos físicos entregues

Branch `work/s019-s018-fatal-crash-evidence-20261017`.
Comparados os artifacts exatos #11728050710 (S017 **entregue
e testado** estágio 02) e #11824027270 (S018 testado).
Ambos XEX2 têm **397312 bytes**, PE PowerPC de **68608 bytes**,
entry `0x82015900`, ROM integral `SMB_v026.nes`
incluída exatamente uma vez por PE e idêntica no SHA256.
S017 02 SHA256 XEX
`cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee`;
S018 SHA256 XEX
`c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`.
O novo S018 contém a importação direta
`XNotifyQueueUI` **além** das três importações
do S017 02. Em `smb360_entry` do S018,
a PRIMEIRA chamada é `notify(text_started)` que
invoca XAM `XNotifyQueueUI`, **antes da ROM/CPU**.
Ordinal 656 confirmado em `OpenXeChain/xecorelib`
`xam/xam.def`. Este é o **principal suspeito**,
não causa física provada: outros bytes/código mudaram,
e não houve stack trace.

Evidência com tabela de imports real e hipóteses:
[docs/evidence/S019/README.md](evidence/S019/README.md).

### Decisão imediata

**Não gerar/enviar outro XEX automaticamente.**
Fazer revisão offline do loader, ABI/imports e
retorno do título, evitando novas notificações XAM
em entrypoints até conhecer a causa. Sem usar
Newlib, framebuffer/MMIO legado ou mexer em
NAND/ABadAvatar/Aurora. Nenhuma exigência de
identificar FreeMyXe/XeUnshackle, já descartada
como bloqueio a pedido do proprietário.
C99 ainda interpreta CPU 6502 no PPC; Mario
não está jogável nem tem vídeo Xbox. A ROM
do filho está preservada.

---

# Histórico — S018 e anteriores


### S018 — publicação, CI da main e entrega verificada (13/10/2026)

PR [#16](https://github.com/PedroMarioaBros/SMNEStoX360/pull/16)
integrada por squash `f94f8acd553777ea198d617ced63d924c9cde3a4`.
CI exato do merge, **todos completed/success**:
- XEX S018 https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357552
- Host (14 suítes normal/ASan, referência e frontend)
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357745
- XEX CRT-free S017 regressão
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357481
- Build canônico S015 regressão (NÃO executar S015)
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357713

Artifact `smb360-S018-STATUS-NATIVE-XEX` #11824027270
(92383 B compactados, expires 12/11/2026),
inclui `default.xex` 397312 bytes SHA-256
`c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`.
ZIP baixado e o XEX extraído; `SHA256SUMS.txt`
foi conferido e manifest `kind=S018_CANONICAL_VISIBLE_STATUS_UNTESTED`
foi lido. A ROM original permanece exata.
O proprietário recebeu XEX individual e ZIP no chat.
**S018 ainda NÃO TESTADA fisicamente.**
CI verde valida compilação e estrutura do arquivo,
não compatibilidade de runtime com Xbox/Aurora.


## Observação física comprovada S017: NÃO é boot validado

O proprietário executou ambas as versões S017 no Xbox 360 com
ABadAvatar e Aurora:
- **01, XEX mínimo:** tela preta e congelamento completo;
  sem Fatal Crash visível e sem retorno possível pelo menu.
  **FALHA / HARDWARE FREEZE**.
- **02, XEX com ROM canônica:** tela preta, sem erro visível,
  **Aurora reiniciado/relançado automaticamente**.
  Não há prova de 120 frames nem causa do retorno; pode ser
  término normal, erro antecipado ou recuperação do gerenciador.
- Não repetir S015 (Fatal Crash real) nem as duas S017.
  Logs: [issue #12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12),
  [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).

## S018 — novo XEX com telemetria de progresso (ainda NÃO físico)

Branch `work/s018-visible-progress-20261013`.
Novo `native_crtless_status.c`: ROM original 40976B embutida,
mesmo núcleo canônico C99 (interpreta CPU 6502 no PPC), sem
Newlib CRT e sem framebuffer; solicitação de avisos do
sistema por XAM para START, ROM INVALID, CPU STOP,
FRAME TIMEOUT ou 120 FRAMES PASS. A conclusão de 120
incrementos da PPU agora é testada com contador 64 bits
e watchdog finito, mas continua **evidência apenas do próprio
programa** até ser executado fisicamente.

`tools/verify_crtless_xex.py --allow-notifications` exige
agora importações exatas (as três de S017 + `XNotifyQueueUI`);
S017 mantém validação estrita das três funções sem a flag.
O workflow separado `xex-status.yml` compila um artifact
`smb360-S018-STATUS-NATIVE-XEX`. Primeiro CI branch
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38415557401
**completed/success**. A ROM não foi editada,
SHA256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
Avisos XAM podem não aparecer ou até provocar falha; o
S015 usou XAM com runtime diferente e sofreu Fatal Crash,
causa não comprovada. Sem imagem nem áudio Xbox.
Ver [S018](evidence/S018/README.md) e
[guia físico S018](TESTE_FISICO_S018.md).

**Estado: S017 fisicamente testado com FALHA na etapa 01 e
RETORNO INCONCLUSIVO na 02; S018 COMPILADO em CI, ainda NÃO
testado em console.** Nenhuma garantia universal nem
Mario jogável. Não insistir em identificar FreeMyXe/XeUnshackle.
Próxima tarefa no NEXT_STEPS.

---

# Histórico operacional — S017 e anteriores


### S017 — publicação e CI confirmado na main

PR [#14](https://github.com/PedroMarioaBros/SMNEStoX360/pull/14)
integrada por squash `40825ff3530d426b05d3e23b4ab02b8d82f12ebe`.
O CI da main para XEX **CRT-free** foi
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754775
`completed/success`; o build canônico legado apenas como
regressão está em `38206754778` `success`. O CI da branch
com código final é `38206678772` `success`; artifact #11728050710,
com `default.xex` mínimo 200704 B e `02_CANONICAL_CRTLESS/default.xex`
397312 B. SHA-256 PR mínimo
`e1e6eedc3722f442d02b4054247726927956e2681609b673c2f42f67cbcf5404`,
ROM embutida
`cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee`.
Confirmação física de boot: **NÃO TESTADO**.
Proprietário recebeu o novo pacote no chat. S015 fatal crash não
é evidência de falha automática do novo XEX; S017 também não
foi validado em console. Nenhuma garantia universal.


## S017 — novo XEX Xbox OS sem Newlib CRT (12/10/2026)

O usuário solicitou explicitamente prosseguir **independente do método de desbloqueio** e produzir um executável real. ABadAvatar é informação conhecida, mas não bloqueia implementação. Não alegar que existe executável universal comprovado sem teste físico.

Branch `work/s017-crtless-xex-20261012`.
**Avanço compilado:** criado entrypoint próprio `smb360_entry`, ligado diretamente por `-nostdlib -Wl,/entry:smb360_entry`, sem o runtime C Newlib que anteriormente importava APIs de filesystem/thread/debug desnecessárias. O novo PE importa **somente 3 funções**: `KeDelayExecutionThread` (kernel), `XamLoaderTerminateTitle` (XAM), `ExTerminateThread` (kernel fallback). Isso corrige uma dependência real do build S015/S016 e conserva formato de título Xbox OS.

Há **duas imagens separadas**: 01 mínimo nativo, aguarda ~5 s e retorna; 02 runtime original C99 + ROM `SMB_v026.nes` embutida, 120 quadros com watchdog, sem XAM input/notification, sem vídeo/áudio Xbox. A CPU 6502 **continua interpretada**, portanto o port estático ainda não foi atingido. A ROM e PRG/CHR permanecem byte-idênticos. O novo `tools/verify_crtless_xex.py` confere XEX2/PE PPC Xbox, entrypoint e importações exatas; exige ROM íntegra apenas na variante 02.

Bug novo encontrado e contornado: SynthXEX retorna falsamente "Out of memory" ao passar `realloc(buffer,0)` numa seção `.data` BSS-only, demonstrado nos arquivos PE do CI `38206298625` e fonte público upstream. Uma variável estática inicializada e usada força seção de raw size não-zero, sem afetar gameplay nem ROM. Ver [evidência S017](evidence/S017/README.md).

CI código completo: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206437946 `success` para ambas as imagens. **Apenas compilação/inspeção; S017 NÃO TESTADA em hardware.** Aviso LLD `/align specified without /driver; image may not run` permanece. S015 é **Fatal Crash em hardware**, permanentemente vetada. Etapa 01 primeiro, 02 só após retorno seguro e decisão consciente do proprietário. Não publicar "Mario jogável".

---


## Precisão de ambiente — entrada ABadAvatar CONFIRMADA (12/10/2026)

O proprietário identificou nominalmente o desbloqueio como **ABadAvatar**, em versão atual de uso, com ativação automática ao iniciar, sem procedimento manual com um jogo. Essa identificação **substitui** a hipótese anterior de que ABadAvatar seria apenas possível. Documentação primária: https://github.com/shutterbug2000/ABadAvatar . **ABadAvatar é o entrypoint do exploit, não necessariamente o patcher de permissões/XEX**: o próprio README avisa que não inclui XEX e usa payload separado em `BadUpdatePayload`, como FreeMyXe ou XeUnshackle. O patcher específico continua **DESCONHECIDO** e a compatibilidade retail/devkit continua sob investigação. Pergunta pendente: aparece tela de FreeMyXe/XeUnshackle ou vai direto ao Aurora? Se necessário, só conferir nomes de arquivos da mídia de desbloqueio, sem modificar nada. **S015 Fatal Crash CONFIRMADO; S016 não testado fisicamente; não solicitar novo XEX agora.** Registro [issue #12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12).


## Atualização de compatibilidade — 12/10/2026 (relato do proprietário)

O proprietário esclareceu que o Xbox 360 usa **desbloqueio exclusivamente por software**, descrito como a versão mais recente disponível, **não RGH/JTAG por hardware**. Ainda **não confirmou o nome do desbloqueio**; os candidatos BadUpdate/ABadAvatar com FreeMyXe ou XeUnshackle são hipóteses, não identificação definitiva. A diferença de compatibilidade retail/devkit de XEX depende do patcher e da versão exata. Orientações antigas do FreeMyXe indicavam restrição a XEX devkit-criptografado, mas o README atual já lista suporte; portanto não concluir incompatibilidade sem confirmar a versão. Fonte técnica e detalhes registrados na [issue #12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12). Até obter o nome do payload/patcher/launcher, **não recomendar novo teste físico S016**. S015 permanece Fatal Crash confirmado e os XEX S016 continuam não testados fisicamente.


## BLOQUEIO REAL: S015 causou Fatal Crash no Xbox 360

O proprietário baixou o artifact S015, executou
`default.xex` no console e obteve **Fatal Crash
imediato**, necessitando reiniciar o Xbox.
Portanto, o S015 é **TESTADO EM HARDWARE E FALHOU**.
Não afirmar que o S015 não foi testado; referências
antigas nesse sentido são históricas.
Não recomendar reexecutar S015.
Registro: https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .

Arquivo S015: 528384 B, SHA256
`48a9071765d3b9db8cfe9cfb91423b5f437ce990d262e380cd7bdd01dea7d077`,
commit `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`,
CI https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966878615
`success` **só** de compilação, nunca sucesso de boot.

## S016 — dois programas de isolamento, NÃO testados no console

HEAD entrada main `2a7a2f6af127adbbb8a025b801d2612fdba55561`,
branch `work/s016-isolate-fatal-crash-20261012`,
integrada pela PR #13:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/13 ,
commit squash `cf10bc38eb1dbf564a83438d60f07dace96766a7`.
O CI S016 do PR https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649408
passou, artifact #11726049878; host
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649340
quatro jobs PASS; compilação canônica
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649496 PASS.
CI pós-merge na main, commit `cf10bc38eb1dbf564a83438d60f07dace96766a7`:
S016 https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204726453
`success`, artifact #11726453526; host
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204726543
`success`; XEX canônico S015 apenas compilado
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204726479
`success`. **Nenhum novo XEX S016 foi testado fisicamente.**

1. `src/platform/canonical_xex/boot_kernel_only.c`: sem ROM
   e sem gameplay, apenas `KeDelayExecutionThread`
   (~5s) e retorno, sem funções de notificação,
   controle, vídeo bruto ou printf.
2. `src/platform/canonical_xex/boot_canonical_no_xam.c`:
   a ROM original embutida e `src/canonical/` com
   120 quadros, sem chamadas diretas XAM,
   stdout, áudio ou framebuffer.
3. `scripts/build-xex-boot-isolation.sh`: valida
   ROM e produz PE PowerPC 0x01f2 + XEX2 em
   `01_KERNEL_ONLY/` e `02_CANONICAL_NO_XAM/`,
   manifests/checksums. `.github/workflows/xex-boot-isolation.yml`
   publica artifact **DISTINTO** do S015. Instruções
   `docs/TESTE_FISICO_S016.md` ficam junto ao pacote.

**Fato descoberto pela leitura do PE compilado:**
a CRT do OpenXeChain liga `xam.xex` e
`XamLoaderLaunchTitle` mesmo em kernel-only.
Os novos programas **não** importam diretamente
`XNotifyQueueUI` ou `XamInputGetState`,
mas NÃO estão livres de toda dependência XAM.
A biblioteca CRT deve ser investigada em S017.

Falhas de desenvolvimento S016 preservadas:
- Workflow `38204175107`: verificação exigia
  ausência total de xam.xex, hipótese descartada
  por inspeção binária do artifact de debug.
- Workflow `38204354807`: compilou ambos e
  testou PE/ROM com PASS, mas etapa CI tinha
  `grep` shell malformado.
- Uma substituição textual incorreta gerou
  YAML duplicado e jobs inválidos; corrigido
  por reconstrução do arquivo e releitura exata.
- Evidência detalhada
  [S016](evidence/S016/README.md).

A ROM `assets/canonical/SMB_v026.nes` permanece
exclusiva, 40976 B e SHA-256
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
Não importar `nathsou/smb`, não editar PRG/CHR.
A CPU 6502 ainda é interpretada em C compilado PPC,
não é tradução estática PowerPC nem jogo jogável.

O próximo teste **não é obrigatório agora** e pode
causar outro Fatal Crash. Caso seja oferecido,
testar SOMENTE o novo pacote S016, **etapa 01 primeiro**;
etapa 02 somente se 01 retornar normalmente.
Qualquer crash interrompe os testes. Não mexer em NAND
ou firmware. Nenhum dos dois novos XEX foi testado fisicamente.

Próxima tarefa S017: distinguir startup/CRT/ABI/imports
versus runtime, diagnosticar aviso LLD e estudar renderização
segura; instruções em [NEXT_STEPS.md](NEXT_STEPS.md).

---

# Histórico dos checkpoints — S015 e anteriores


## S015 — diagnóstico XAM e dois controles Xbox

HEAD de entrada `884c5db3defec68fbf58793fccbbc69a225584c2`.
Branch `work/s015-observable-boot-20261009` integrada pela PR #11:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/11 .
Commit squash na main `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`. ROM original
`SMB_v026.nes` preservada byte a byte e verificada por SHA-256.
O core canônico continua sendo compilado sem nathsou.

`controller_map.[ch]` converte os botões Xbox para os oito botões NES,
com teste sintético dos dois ports $4016 e $4017; agora são 14 suítes.
O alvo Xbox passa a consultar ambos os controles com
`XamInputGetState` e tenta exibir notificações XAM no início
e após 120 quadros. Continua SEM framebuffer direto, sem vídeo
do Mario e sem saída de áudio para console.

`tools/verify_canonical_xex.py` conferiu PE PowerPC 0x01f2,
quatro seções e a ROM completa de 40976 bytes no offset PE
0x16c00, além do XEX2; CI:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37965972564
A falha intermediária 37966195621 foi uma comparação obsoleta
do campo `video_backend` no workflow; compilação já passava.
A checagem foi atualizada. O aviso LLD sobre /align permanece
sem teste físico para determinar seu impacto.

Guia ao proprietário: [TESTE_FISICO_S015.md](TESTE_FISICO_S015.md).
Evidência: [S015](evidence/S015/README.md).
O proprietário deve receber um aviso explícito com o artefato
e instruções quando os testes do build final estiverem verdes.
Nenhum boot Xbox foi comprovado; o runtime ainda interpreta
CPU 6502 em C compilado para PowerPC.
Próxima fase após teste físico: vídeo seguro sem MMIO antigo,
áudio Xbox, input em jogabilidade e tradução estática PPC.

---

# Checkpoints anteriores — S014 e predecessores


> **Organização documental de 09/10/2026 (sem nova versão do jogo):** consulte [STATUS_PARA_PEDRO.md](STATUS_PARA_PEDRO.md) e [painel central PMCN](https://github.com/PedroMarioaBros/OpenXeChain-X360-Builder/blob/main/docs/PAINEL_PMCN_XBOX360.md). O build nathsou antigo foi identificado como **LEGADO, execução manual apenas** no workflow `xbox360-cloud-build.yml`; não faz parte da rota canônica. O workflow `canonical-xex.yml`, a ROM e os testes seguem inalterados; **S015 continua pendente**, sem novo boot físico.


## S014 — primeiro XEX2 compilado com ROM e runtime canônicos (diagnóstico)

Entrada main `cdcda47f9952afa92b09228a4da100a463ef97c0`.
Branch `work/s014-canonical-xex-20261009` integrada
pela PR #10: https://github.com/PedroMarioaBros/SMNEStoX360/pull/10 .
Commit de merge/squash: `b3a0aa56d6b45be588cec0ebfd936f1cb070ce07`.
A ROM original `SMB_v026.nes` permanece versionada sem alterações,
SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
Nenhum arquivo ROM, PRG ou CHR foi modificado.

**Avanço implementado:** target `src/platform/canonical_xex/main.c`
que incorpora o **mesmo** `src/canonical/*.c` usado no host e
a ROM de 40976 bytes via `embedded_rom.S`. Executa
`smb360_machine_init` e prepara diagnóstico headless de 120
quadros, timeout e relatório de PC/ciclos/hash FNV dos pixels.
`scripts/build-canonical-xex.sh` verifica três SHA-256 e compila
C99 `-Wall -Wextra -Werror` com OpenXeChain PowerPC, gerando
`canonical.exe` e `default.xex`. Workflow novo
`.github/workflows/canonical-xex.yml` é independente do workflow
histórico que compila `nathsou/smb`. **Não copia outro gameplay**.

**Validação observada:**
- Compilação cruzada PowerPC do código exato
  `75eb79a35b6c60ff8ed4d4e17a88404af89c2325`,
  CI https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958120419 :
  `completed/success`, toolchain `ppc32-unknown-xbox360`,
  confirmado também pós-merge `b3a0aa56d6b45be588cec0ebfd936f1cb070ce07`
  (XEX canônico: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798039
  `success`, artifact id 11629836328; host:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798074
  `success`, quatro jobs).
  SynthXEX produzindo XEX2, hashes do XEX+PE verificados,
  manifest `CANONICAL_DIAGNOSTIC_NOT_PLAYABLE`.
  Artifact `smb360-CANONICAL-DIAGNOSTIC-xex`, id 11630285368,
  disponível no GitHub Actions por 30 dias.
- Primeiro run `37958041952` falhou por um comentário contendo
  `src/canonical/*.c` (interpretação `/*` por Clang Werror).
  Corrigido comentário no código sem alterar runtime/ROM.
- Regressão host no commit
  `0217fc98606e491c4458855e7f413b581474d36e`,
  CI https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958266320 :
  `completed/success`, jobs normal, ASan/UBSan, referência e
  frontend SDL. 13 suítes host PASS nos dois modos.
  Binjnes pixels/RAM idle 600/600 e controles 600/600,
  SDL readback 61.440 pixels e WAV determinístico
  479146 amostras, SHA-256
  `3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.

**Limite decisivo:** o `default.xex` novo foi compilado e
verificado como XEX2, **mas não iniciado fisicamente no Xbox**.
O linker avisa `/align specified without /driver; image may not run`.
O entrypoint desta etapa NÃO apresenta vídeo, recebe comandos ou
toca som; é um diagnóstico headless de execução do PRG original.
Não reutiliza `video_fb.c` com endereços hardcoded antigos
(build histórico #16 deu Fatal Crash). Não é jogo jogável.
A CPU 6502 segue interpretada em C no PowerPC; não existe
a tradução estática 6502→PPC sem interpretador desejada pelo proprietário.
CI legado `nathsou` segue issue #8 separada.

Próxima S015: investigar boot real, aviso linker, vídeo Xbox por API
segura, controles, saída PCM, e planejamento da tradução direta PPC.
Evidência: [S014](evidence/S014/README.md). Comandos e critérios
em [NEXT_STEPS.md](NEXT_STEPS.md). **Sem percentual global inventado**.

---

# Checkpoints anteriores — S013 e predecessores


## S013 — mixer NES, PCM 48 kHz, WAV canônico e SDL áudio

Entrada main: `6e158f17572e17bfa85a8543710a47fdd73497d1`.
Branch `work/s013-audio-pcm-20261009` integrada à main
pela PR #9: https://github.com/PedroMarioaBros/SMNEStoX360/pull/9 .
Commit squash: `64a7edaa76c14b069da283023bab38d1b64e1173`.
ROM exclusiva `assets/canonical/SMB_v026.nes`, 40.976 B,
SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`;
PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
ROM/PRG/CHR **não foram alterados**.

Implementados `src/canonical/audio_pcm.[ch]`: mixer não linear
NES (pulse1+pulse2 e triangle+noise+DMC), relógio integer de amostras
à frequência CPU NTSC 1.789.773 Hz para PCM mono signed 16-bit
48.000 Hz, filtros aproximados passa-altas 90/440 Hz e passa-baixas
14 kHz, ganho de headroom. `src/host/main.c` transmite samples
por `SDL_QueueAudio` quando dispositivo disponível e suporta
`--wav` para gravação RIFF PCM little-endian determinística.
Sem áudio disponível, o frontend continua funcionando.
AUDIO capturado no limite de cada instrução, **não** interpolado
por microciclos: fidelidade NES de áudio ainda não comprovada.

Novos testes: `tests/test_audio_pcm.c`, adicionados aos C99
normal + ASan/UBSan (13 suítes); `tests/test_host_audio.py`
faz duas capturas de 600 quadros e confere SHA, duração, contagem,
níveis não nulos e ausência de clipping. O workflow também arquiva
`canonical-s013-audio-wav` como artefato por 30 dias.

Validação nova S013: commit de código/CI
`5666e3437e7073e781862991fa9d0b5fb4bd61ed`,
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892058096 ,
**completed/success**, quatro jobs: host normal,
host sanitizado, reference binjnes e frontend SDL.
O merge S013 também passou na main: commit
`64a7edaa76c14b069da283023bab38d1b64e1173`,
run https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892331535
(quatro jobs `success`, artifact WAV `canonical-s013-audio-wav`,
id `11599295620`).
13/13 suites normal/san PASS; binjnes idle 600/600 pixels e RAM
e roteiro 600/600 pixels e RAM; readback SDL 61.440 pixels PASS.
A captura determinística `canonical-600frames.wav` tem
**479.146 samples**, min −5.889 / max +6.328, 159.921 não nulos,
sem saturação de 16 bits. SHA-256 de ambos os WAV:
`3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.
Artefato GitHub Actions identificado como
`canonical-s013-audio-wav` (artifact id 11598404861, 30 dias,
arquivo WAV descompactado 958.336 bytes).
Evidência integral: [S013](evidence/S013/README.md).

Tentativas falhas (workflows 37891903477, 37891908427,
37891911591): fixture inicial esperava silêncio com APU inicial,
mas o triangle DAC começa em 15 e mantém nível; correção
foi no teste, não no runtime. Workflow agora observa arquivos
`tests/test_audio*` para não perder regressões.

**Limitações:** houve síntese e gravação WAV reproduzível,
mas execução SDL auditiva só foi exercitada em ambiente
dummy; sem teste real com caixa de som.
Não há áudio de referência independente para comprovar
fidelidade ao NES. DMC DMA/IRQ ainda aproximados por instrução,
PPU não cycle-perfect. Xbox: nenhum XEX do core canônico
iniciado em hardware; compilador histórico `nathsou/smb` é
separado e possui falha registrada na issue #8.
Runtime atual ainda interpreta 6502 em software, não realiza
port nativo 6502→PowerPC sem emulador. Não inventar percentual
total com base em cinco canais APU.

Próxima S014: build canônico Xbox do `src/canonical`
com PRG/CHR incorporados e tela/loop seguros,
sem confundir com o XEX histórico; iniciar comparação
sonora independente no host. Procedimento NEXT_STEPS.md.

---

# Checkpoints anteriores — S012 e anteriores


## S012 — DMC do NES + primeiro DMA/IRQ em runtime

Entrada main: `9293f007b29060efa2c7349f2ff272eeca04d60f`.
Branch `work/s012-apu-dmc-20261009` integrada por PR #7:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/7 .
Commit squash na main: `af8f34487830f02b19170fb9b5fc107eaf360b33`.
ROM/PRG/CHR canônicos de `SMB_v026.nes` preservados:
ROM SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
Nenhum arquivo `assets/canonical/` foi modificado.

Adicionado em `src/canonical/apu.[ch]` o quinto canal digital:
DMC com DAC 7 bits, 16 períodos NTSC, $4010-$4013, bit4 de
$4015, amostragem de bytes do PRG por endereço $C000+$4012*64,
tamanho $4013*16+1, wrap $FFFF→$8000, buffer/shifter, delta
saturado, silêncio, loop e DMC IRQ bit7 ($4015).
`machine.c` conecta DMA do DMC ao barramento NROM original,
contabiliza stall estimado de quatro ciclos e amostra linha de IRQ
da APU em limites de instrução. Teste `test_apu_dmc.c`
verifica endereços, status/IRQ, bytes, loop, taxas, saturação de
DAC e integração com máquina/6502. O runner inclui 12 suítes.

**Validação real:** último commit de código
`e1a68b1d7ded75cb968556fe425b5fdcb584e78d`,
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890958510,
status **completed/success**, quatro jobs `host (0)`,
`host (1)` (ASan/UBSan), `reference`, `frontend`.
12/12 suítes em ambos os modos; `test_apu_dmc: PASS`.
`verify_repository.py` aprovou assets; binjnes idle **600/600**
pixels e RAM e roteiro **600/600** pixels e RAM.
O workflow host na **main** após merge também passou:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37891237452
(commit `af8f34487830f02b19170fb9b5fc107eaf360b33`).
SDL dummy readback 61.440 pixels PASS e captura 599 bate hash
`fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc`.
Os testes completos ocorreram no GitHub Actions, não em
checkout local; não confundir baseline gráfico com waveform de áudio.
Evidência: [S012](evidence/S012/README.md).

**Limite técnico crítico:** DMC implementado, mas DMA ainda *não*
é cycle-perfect (stall fixo e leitura após instrução; falta
arbitragem real CPU/OAM). IRQ tratada no boundary e não no
pino em cada microciclo. Não há teste independente de áudio.
Faltam mixer não linear, resampling e PCM no SDL,
Xbox 360 backend canônico com boot/vídeo/input/áudio seguros,
teste físico e tradução nativa PowerPC. A execução atual
do PRG 6502 é feita pelo runtime de software, não é
ainda o port direto desejado sem interpretador.
O antigo XEX #16 causou Fatal Crash, nunca declarar
funcional o hardware sem teste real.

Próxima tarefa S013: saída PCM/mixer com testes determinísticos
e captura de áudio da ROM canônica; após isso tratar os limites
de DMA/IRQ e Xbox. Comandos no NEXT_STEPS.md.

---

# Checkpoints anteriores — S011 e predecessores


## S011 — quarto canal APU: noise

HEAD de entrada: `c7e4cc7c7954d6fad528dd5a23a7a5b0c89fbf74`.
Branch desenvolvida: `work/s011-apu-noise-20261009`.
PR #6 integrada à main: https://github.com/PedroMarioaBros/SMNEStoX360/pull/6 .
Commit squash da integração: `d8a9c0a60a2aa46b92276d1ece0c0558204dc2e3`.
ROM do proprietário `SMB_v026.nes`, 40976 bytes, hash canônico
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`;
PRG/CHR preservados, conferidos também por SHA-256
(documentados em [identidade](SMB_V026_CANONICAL_ROM.md)).
Clone local remoto indisponível por DNS; publicação e CI via conexão
GitHub, sem copiar arquivos de ROM externa.

Implementado `src/canonical/apu.[ch]`: canal noise NTSC, $400C
envelope/volume/length halt, $400E modo e seleção dos 16 períodos,
$400F length + envelope start, $4015 bit3 de habilitação/status.
Registrador LFSR de 15 bits com seed 1, taps bit0/bit1 (longo) e
bit0/bit6 (curto), timer CPU/2, envelope quarter-frame,
length half-frame e saída DAC 0..15 silenciada por bit0/length.
$400D ignorado. Pulse1, pulse2, triangle e leitura $4017 (controle2)
preservados. Mais uma suíte de teste, `test_apu_noise.c`, agora
11 suítes host. Teste de barramento ampliado em `test_ppu_registers.c`.

**Validação de código no GitHub Actions**:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890367890
(commit `19b5e2977aef8d98bea0c84c044d13318685b2a5`).
Status **completed/success**, quatro jobs verdes: host normal,
host sanitizado (ASan/UBSan), reference e frontend SDL.
O merge S011 também passou em CI na main: commit
`d8a9c0a60a2aa46b92276d1ece0c0558204dc2e3`, workflow
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890597158
(quatro jobs concluídos com sucesso). Todas
as 11 suítes host passaram nos dois modos. O comparador binjnes
obteve 600/600 pixels e RAM no cenário idle e 600/600 pixels e RAM
com entrada. SDL readback conferiu 61.440 pixels; captura 599
idêntica à referência. Esses resultados dizem respeito ao
commit de código acima, não a commits documentais posteriores
e não verificam waveform de áudio.
Uma tentativa CI anterior, workflow `37890305871`, falhou porque
o novo teste de integração presumia LFSR=1 depois do RESET da CPU;
os sete ciclos de RESET já haviam avançado o registrador.
A correção captura o estado após RESET e confere que as escritas
$400C/$400E/$400F não o reinicializam. **Foi a fixture, não o
jogo, que precisou de correção.** Histórico: [S011](evidence/S011/README.md).

Limites: quatro unidades APU com nível digital implementado, sem DMC,
mixer não-linear, saída PCM/SDL ou som audível; IRQ APU ainda não ligada
à CPU, sem temporização por microciclos/validação de waveform.
Nenhum XEX da arquitetura atual foi validado em hardware;
build histórico #16 teve Fatal Crash. A CPU 6502 é executada
em software, não é um port nativo PowerPC livre de interpretação.
Próxima tarefa S012: DMC, bus DMA/IRQ, tests e PCM mais adiante;
comandos em NEXT_STEPS.md.

---

# Checkpoints anteriores (S010 e predecessores)


## S010 — terceiro canal APU, triangle, validado no host

Entrada: main `030813421ed1aff21e5b6289392a48c9f2c17d1c`;
branch `work/s010-apu-triangle-20261009` integrada via PR #5:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/5 .
Merge/squash publicado na main: `dee528eabe2eb0f0c349d265d3ec256ba4ec91cf`.
Implementado o canal triangle da APU Ricoh 2A03 em `apu.[ch]`:
registradores $4008/$400A/$400B, timer a cada ciclo CPU (pulse1/2
continuam CPU/2), contador linear com reload/control, length counter,
32 fases 0–15 e retenção do DAC ao interromper o sequenciador.
Leitura do status $4015 ganha bit2; escrita habilita/desabilita apenas
o length do triangle. $400B não reseta a fase. `tests/test_apu_triangle.c`
adicionado aos testes host (normal e san) e `test_ppu_registers.c`
ampliado para isolamento do barramento. Workflow passa a disparar
também em alterações em `tests/test_apu*`.

A ROM canônica **não foi alterada**. Hash SHA-256 da cópia do proprietário
verificado localmente: ROM `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`;
PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`;
CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.

**Validação do código**: commit `73ca0151d153448e8d13cca07bdf395fd93618cd`,
GitHub Actions https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889762682,
status `completed/success`; jobs host normal, ASan/UBSan, referência e
frontend SDL, todos aprovados. 10 suítes host passaram nos dois modos,
incluindo pulse1, pulse2, triangle e integração ao barramento.
Binjnes: idle 600/600 pixels e 600/600 estados RAM; entrada 600/600 pixels
e 600/600 estados RAM. SDL readback 61.440 pixels aprovado, quadro 599
com hash idêntico à referência. Não houve comparação de waveform de áudio.

Durante o desenvolvimento duas execuções CI falharam porque uma fixture
supunha parada do nível triangle imediatamente após $4008; o gate muda
no próximo quarter-frame. Fixture corrigida, CI final verde, sem alteração
artificial no core. Clone direto no container ficou bloqueado por DNS;
o teste completo foi feito no GitHub Actions, não localmente.
Detalhes e links em [evidence/S010](evidence/S010/README.md).

**Limites:** três canais com níveis DAC implementados (pulse1, pulse2,
triangle), mas ainda sem som audível. Faltam noise, DMC, mixer,
resampler/PCM/SDL áudio, IRQ APU ligada à CPU, precisão por microciclo
e validação independente de som. Nenhum novo `default.xex` desta
arquitetura canônica foi testado no Xbox 360. O core ainda executa 6502
por runtime de software, não traduz o jogo nativamente para PowerPC.
Próximo passo técnico: canal noise com testes e comparação diferencial;
ver NEXT_STEPS.md. Não declarar percentual global inventado.

---

# Checkpoints históricos preservados (S009 e anteriores)


## S009 — segundo canal pulse da APU integrado à main

HEAD de entrada: `248c8758f5421c7ed1c7bbf230b8e509c79f5d83`.
Pull request #4: https://github.com/PedroMarioaBros/SMNEStoX360/pull/4 .
Integração por squash: `652f4e2d7f71d26e6a4eb6f65d438aadd6f1c957`.
A ROM SMB_v026.nes, PRG e CHR continuam idênticos aos hashes
documentados em SMB_V026_CANONICAL_ROM.md; ROM 40976 bytes, NROM.

Código integrado: `src/canonical/apu.[ch]` trata pulse 1 e pulse 2 com
estado separado, temporizadores próprios, duty, envelope, length e sweep.
Pulse 1 mantém a negação por complemento de um; pulse 2 usa complemento
de dois. Escritas de $4000–$4007 e controle/status de ambos por $4015
são independentes. Saídas individuais 0–15; **ainda não são áudio PCM**.
Leitura $4017 continua exclusiva do controle 2; escrita $4017 é APU.
As novas verificações de barramento abrangem duas durações e controles.

Validação nova S009:
- Testes unitários isolados C99 com `-Wall -Wextra -Werror`:
  test_apu_pulse e test_apu_pulse2 passaram normalmente e com ASan/UBSan
  (detect_leaks=0). Duas fixtures novas falharam inicialmente por índices
  de length e de fase duty incorretos; foram corrigidas, sem alterar o
  runtime para mascarar erros.
- GitHub Actions do commit de código
  `3f877fd7d3802df35897d303105b7980d56ecd7a`:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889071037
  status **success**; passaram os quatro jobs (host normal, sanitizado,
  frontend SDL e referência).
- Comparação independente do binjnes: idle **600/600** pixels iguais
  e **600/600** RAM iguais; entrada **600/600** pixels e **600/600** RAM
  iguais. Cenas restritas aos roteiros existentes; não equivalência de
  áudio, ciclos ou gameplay completo.
- Código/provas reproduzíveis em [evidence/S009](evidence/S009/README.md).
  Clone local via GitHub falhou por DNS neste ambiente; a suíte completa
  foi executada no GitHub Actions, não no container.

Limites inalterados: triangle, noise, DMC, mixer não-linear, PCM, áudio SDL,
IRQ da APU ligada à CPU, precisão por microciclos, backend Xbox canônico e
teste físico estão pendentes. O runtime ainda executa PRG 6502 por CPU de
software, e não via tradução nativa PowerPC. Um job "Xenon Build" antigo
não comprova boot nem gameplay. Próxima ação: implementar triangle e seus
testes/regressões, conforme NEXT_STEPS.md.

---

# Histórico anterior — S008 e sessões precedentes


## S008 — base APU/pulse 1 em branch de trabalho

Entrada no main: `f24d735559ca74eee7467224b5023402e64fdf69`.
Branch desenvolvida: `work/s008-apu-pulse1-20261009`, integrada à main
por PR #3 (squash) em `f0af8dbf7236571760e03e6e36938ae61c44edab`.
A ROM canônica fornecida bateu os hashes completos de ROM, PRG e CHR;
nenhum byte foi alterado.

Novo módulo `src/canonical/apu.[ch]` implementa contador de quadros NTSC
4/5 passos e pulse 1: timer, duty, envelope, comprimento, sweep e mute.
Conectado ao barramento e à contagem de CPU, NMI e DMA. Escrita $4017 segue
para APU; leitura $4017 continua exclusivamente para o controle 2. Status
$4015 foi implementado somente para pulse 1 e frame IRQ; a IRQ não
está ligada ao CPU. Saída é apenas nível digital 0–15, **sem áudio audível**.
Demais canais e mixer/PCM pendentes. Temporalidade é agregada por instrução,
ainda não equivalente a clock por microciclo.

Validação local isolada do módulo: compilação `-std=c99 -Wall -Wextra -Werror`
e execução de `tests/test_apu_pulse.c` em normal e ASan/UBSan passaram.
LeakSanitizer desativado. O clone local do GitHub falhou por DNS: não
foi possível rodar o checkout integral neste ambiente. GitHub Actions para
o commit `a41845a49415e04c6d71407bb6928615c14c7195` concluiu
**success** em https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888335327
(já com APU ligada ao runtime e teste unitário incluído).
O commit posterior de asserções adicionais em `tests/test_ppu_registers.c`
foi também validado: GitHub Actions **success**, 4 jobs (host 0/1,
frontend SDL e referência), para o commit `af36b563ee19543acd826bd89a768748e6ae0b5e`:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888390752 .
O run contempla 600 capturas idle e 600 com roteiro (pixels e RAM).
Este sucesso não certifica precisão total de áudio/hardware e pertence ao
commit de código acima, não a commits documentais posteriores. Evidência: [S008](evidence/S008/README.md).

Nenhum novo `default.xex` nem teste físico. O XEX histórico com Fatal Crash
não é uma versão validada. A comparação diferencial curta passou; o merge foi concluído. Próximos
passos: pulse 2 e expansão da APU, mantendo a regressão e os testes por canal.
Ver NEXT_STEPS.md.

---

# Checkpoint anterior — histórico preservado

## Estado atual — S007, 08/10/2026

HEAD de entrada: edfdc149bb6d3a47cceac03a82f5bd345a5d9177.
Entregue frontend host SDL2 com PRG/CHR canônicos embutidos, janela direta,
teclado para dois jogadores e suporte implementado a dois gamepads/hotplug.
Build verifica hashes antes de gerar o header. Não carrega .nes em runtime.
Não altera src/canonical. Comandos e controles: [HOST_FRONTEND.md](HOST_FRONTEND.md).

Validado automaticamente com SDL dummy: 600 quadros, 5.794.218 instruções,
17.865.924 ciclos, 593 NMIs, **101 opcodes distintos**, PC $8057, frame 599,
linha/dot 241/6. Seis eventos de teclado atravessaram a fila SDL.
Leitura da saída do renderer: 61.440 pixels lógicos conferidos.
Quadro final idêntico ao binjnes S006/captura 599, SHA-256
fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc.
Execução em diretório vazio comprova ausência de dependência de ROM externa.

Sete suítes host passaram. Smoke SDL normal e ASan/UBSan passaram e produziram
quadros idênticos (leak detection desabilitado). Logs, hits, imagem, binário host
e hashes em evidence/S007/. Gamepads físicos, sessão manual e desempenho real
com janela NÃO testados. APU ainda ausente; nenhum XEX novo.
Recorde de instruções continua 57.751.826 da S006, não aumentou nesta sessão.
Próxima etapa: APU por canais, começando pela base temporal e pulse 1 com testes.

## Histórico — S006, encerramento 08/10/2026

HEAD recebido: e99b80760d72387a8729b2fb9ddd3056cfa0bf22.
Marco: comparação independente executada contra binjnes fixado, mesma ROM,
sem deslocar capturas nem excluir RAM: **600/600 quadros idle e 6000/6000
quadros com roteiro coincidem; toda a RAM de 2048 bytes também coincide**.
Evidências: [S006](evidence/S006/README.md). A ferramenta e fonte de referência
MIT estão no GitHub, sem uso da referência como gameplay do produto.

A comparação inicialmente encontrou RAM divergente em 5895 das 6000 capturas,
começando na captura 103 ($06fd/$074b), mesmo com imagens iguais. O barramento
lia $4017 do placeholder APU: writes eram interpretados como botões fantasmas.
Implementados controle 2, latch compartilhado por $4016 e shifts independentes;
$4017 write continua separado no placeholder APU. Após a correção: zero divergências.
Testes cobrem strobe, oito bits, retenção do latch, independência dos controles,
retorno 1 após oito leituras e separação das escritas de áudio.

Execução diferencial longa: **57.751.826 instruções**, 178.680.628 ciclos CPU,
captura/frame 5999, linha/dot 241/6, 5992 NMIs antes de processar o NMI dessa captura.
Novo maior número efetivamente medido; não equivale a certificação do jogo inteiro.
Roteiro solta os controles após intervalo 359, não é jogabilidade humana contínua.
Cobertura de opcodes não foi exportada pelo adaptador diferencial.

Runner tradicional de 5m: 15.391.133 ciclos, frame 516, linha/dot 214/185,
PC $8057, 99 opcodes, 510 NMIs, 333 sprite hits. Primeiro NMI continua
$8082/frame 3/ciclo 116.744. Quadro mantém hash f8f38478…659e5.
Log/hits completos em evidence/S006/start-right.txt. Execução normal e sanitizada
reproduziram log/quadro; sete suítes normais e ASan/UBSan passaram (sem leaks).

Etapas: ROM/persistência prontas; CPU oficial implementada; vídeo/controles
funcionais e comparados nestes cenários; falta bancada host interativa para
controle manual, APU/áudio e backend Xbox seguro. Nenhum XEX novo nem hardware
validado. Próximo trabalho e comandos em NEXT_STEPS.md.

## Histórico — S005, 07/10/2026 UTC

Entrada: `11cd0245985577ea7d6ef3a3e47ff015e06f13ed`. Esta seção substitui
os diagnósticos de HUD ausente das sessões anteriores: a conversão direta dos
61.440 índices do quadro mostra MARIO, MUNDO, TEMPO, pontuação e moedas.
Não houve correção de HUD; a conclusão visual anterior estava errada.
Imagem e evidência: [S005](evidence/S005/README.md).

Implementado nesta sessão: omissão do dot 340 no pre-render dos frames ímpares
NTSC quando background ou sprites estão habilitados. A paridade avança também
com rendering desligado. Testes cobrem duração dos frames, máscaras independentes
e habilitação/desabilitação junto ao limite. O teste novo falha no core anterior.
Não torna a PPU cycle-perfect; CPU/PPU continuam sincronizadas por instrução.

| Cenário S005 | Instruções | Ciclos CPU | Frame | Linha/dot | PC | Opcodes | NMIs |
|---|---:|---:|---:|---|---|---:|---:|
| Start/direita | 5.000.000 | 15.390.305 | 516 | 207/88 | $8057 | 99 | 510 |
| Prolongado | 50.000.000 | 154.288.005 | 5.180 | 219/340 | $8057 | 100 | 5.174 |

Sem opcode bloqueador; 333/4.997 sprite-zero hits respectivamente. Primeiro NMI
continua $8082, frame 3, ciclo CPU 116.744 após a entrada. Hashes dos quadros
permanecem iguais aos históricos. O limite máximo medido continua 50 milhões;
esta sessão não aumentou esse recorde. Hits completos nos logs S005.

Ferramenta nova: `python3 tools/frame_to_png.py build/gameplay.bin build/gameplay.png`
converte índices em PNG sem dependências externas. Paleta RGB ilustrativa,
sem emulação de sinal analógico ou emphasis; não usar cor RGB como prova de fidelidade.

Próxima tarefa: configurar referência independente com a mesma ROM e alinhar
reset/entrada/captura antes de comparar estados e pixels. Não presumir novamente
que todo o HUD está ausente. Áudio/APU e execução Xbox ainda pendentes.

## Ordem permanente de continuidade (atualizada na S002)

Todo novo chat/Work começa em [START_HERE.md](../START_HERE.md) e deve cumprir
[CONTINUITY_PROTOCOL.md](CONTINUITY_PROTOCOL.md). A cada avanço, registrar
resultados e tentativas. Antes da resposta final, atualizar este checkpoint,
[SESSION_LOG.md](SESSION_LOG.md) e [NEXT_STEPS.md](NEXT_STEPS.md), publicar
commits e confirmar HEAD remoto. O sucessor deve preservar e repetir o processo.

A S002 definiu o protocolo. A S003 colocou a ROM e os outputs no GitHub.
As medições históricas abaixo são da S001; o estado S005 está no início deste arquivo.
A tarefa ativa e os próximos comandos copiáveis estão em NEXT_STEPS.md.

## Retomar daqui

Repositório: PedroMarioaBros/SMNEStoX360, branch main.
Implementação desta sessão: commit `13e6249255c7f3a9b4dd1921bad360db4e3f9035`.
O commit 4c438b85fdb27b4deaf6cfec319e66ad2586b50c publicou a primeira versão
deste checkpoint, as evidências e o CI sintético.

**Marco: execução do PRG canônico no host, entrada NMI real em $8082,
quadros de título/fase e controles roteirizados. Ainda NÃO é port integral
validado nem jogo validado no Xbox 360.**

## Identidade e entrada

Na S001, a ROM foi recuperada dos arquivos do proprietário, sem obter outra ROM.
Na S003, por instrução expressa do proprietário, passou a ser versionada em
assets/canonical/SMB_v026.nes, com PRG, CHR e manifest na mesma pasta.
Os hashes coincidem com [SMB_V026_CANONICAL_ROM.md](SMB_V026_CANONICAL_ROM.md).
Não há mais dependência de anexos do ChatGPT para executar o core.
Outputs da S001 foram preservados em artifacts/session-S001-host.tar.xz;
inventário e procedimentos em [REPRODUCIBILITY.md](REPRODUCIBILITY.md).

## Auditoria inicial e progressão medida

HEAD recebido: `b8de2f29f36316f386a3ff4c54f91a8c76c08b0f`.

- NROM, RESET e timing PPU originais: compilados e executados com sucesso.
- Runner original: NÃO compilava; strings continham newlines literais dentro das
  aspas. Corrigido antes de qualquer medição.
- Após apenas corrigir o runner: 29.051 instruções, 94.210 ciclos CPU,
  frame 3, scanline 42, dot 261. Opcode bloqueador $99 em $8227; PC após fetch
  $8228. 24 opcodes encontrados, incluindo o bloqueador.
- CPU expandida, antes do NMI: 1.000.000 instruções, 3.006.944 ciclos,
  frame 100, scanline 253, dot 338, PC $8057; 30 opcodes. Era um laço de espera.
- NMI integrado: primeiro destino $8082 no frame 3; contador CPU 116.744
  **após** os 7 ciclos de entrada. Primeiro milhão: 27 NMIs, 84 opcodes;
  a espera seguinte dependia do sprite zero.
- Registradores e DMA: avançaram a inicialização, mas não resolveram sozinhos
  a espera por sprite zero.
- Compositor funcional: sprite zero deriva da sobreposição opaca de pixels
  CHR/background/sprite, com clipping. Nenhum bit de hit é forçado por um
  endereço ou condição específica do SMB.

## Resultados finais reproduzíveis (core 13e6249)

| Cenário | Instruções | Ciclos CPU | Frame PPU | Linha/dot | PC | Opcodes distintos | NMIs |
|---|---:|---:|---:|---|---|---:|---:|
| Sem entrada | 1.000.000 | 3.067.255 | 102 | 260/221 | $813D | 85 | 98 |
| Start + direita/pulo | 5.000.000 | 15.390.320 | 516 | 206/242 | $8057 | 99 | 510 |
| Mesmo roteiro, execução prolongada | 50.000.000 | 154.288.271 | 5.180 | 214/279 | $8057 | 100 | 5.174 |

Nenhum opcode bloqueou esses três cenários. Todos terminaram no limite de
instruções solicitado. PC $8057 é o laço entre NMIs, não evidência isolada
de travamento. O cenário longo registrou 4.997 eventos de sprite-zero hit.
Os números de frame são contadores zero-based, não confirmação de fidelidade
de todos os quadros.

Logs completos, incluindo hits por opcode e hashes de quadro:
- [Título](evidence/2026-10-07/title.txt)
- [Start/direita](evidence/2026-10-07/start-right.txt)
- [Execução prolongada](evidence/2026-10-07/long-run.txt)

O cenário de 5 milhões foi repetido: logs e 61.440 bytes de índices do quadro
foram idênticos. Uma execução com ASan/UBSan também gerou o mesmo log e quadro.
Isso demonstra repetibilidade desse cenário, NÃO equivalência com o NES.
O cenário de 50 milhões foi medido uma vez.

## Implementado e testado

- CPU: todos os 151 opcodes oficiais NMOS 6502 com aritmética Ricoh 2A03;
  page penalties, indireção com wrap de zero-page, JMP indirect page-wrap,
  stack, BRK/RTI, IRQ e NMI; unknown opcodes param.
- Operações RMW fazem write do valor original antes do modificado.
- ADC/SBC: 524.288 combinações de operandos/carry/flag D passaram.
- ASL/ROL/LSR/ROR: 10.240 casos de valor/carry/formas de endereço passaram.
- Testes direcionados: RESET, branches/page crossing, zero-page wrap,
  endereçamento indexado, JSR/RTS, stack wrap, BRK/RTI, IRQ, NMI, JMP bug,
  PPUSTATUS, arestas NMI e parada persistente em opcode não oficial.
- PPUCTRL/MASK/STATUS, OAMADDR/DATA, SCROLL/ADDR/DATA, latches v/t/fine-X,
  mirroring vertical, palette mirroring e masking de seis bits, CHR read-only,
  PPUDATA buffer e incremento 1/32.
- OAM DMA copia 256 bytes com wrap e acrescenta 513/514 ciclos.
- Controller: strobe alto lê A atual; serialização dos oito bits e retorno 1
  após oitavo bit.
- Renderização inicial: background, atributos, sprites, prioridade, flips,
  sprites 8x8/8x16, clipping, sprite zero e incremento vertical.
  Testes de render cobrem casos selecionados, não todas essas combinações.
- Compilação C99 com -Wall -Wextra -Werror; sete executáveis de testes passaram
  em configuração normal e ASan/UBSan.
- Wrapper recusou uma cópia deliberadamente alterada da ROM antes da execução.

**151 implementados não significa 151 instruções independentemente certificadas
em todos os modos e estados.** Testes externos de conformidade continuam necessários.

## Falhas observadas e limitações

1. PPU NÃO é cycle-perfect. Compositor faz acesso direto aos tiles, não pipeline
   de fetch/shift registers; snapshot por linha simplifica scroll, especialmente
   alterações durante a linha. Não modela dummy reads do CPU, microciclos de
   interrupções, atraso de polling IRQ após CLI/SEI/PLP ou janelas de supressão NMI.
2. Diagnóstico antigo de HUD ausente retirado na S005 por evidência dos pixels.
   A imagem tem elementos de HUD; fidelidade completa ainda exige referência.
3. Render não modela o bug de overflow de sprites, pipeline de avaliação,
   efeitos de escritas durante rendering, paleta analógica
   ou color emphasis. PPUSTATUS races, reset/power-up e CPU open bus incompletos.
4. DMA é cópia instantânea + stall agregado, sem arbitragem ciclo a ciclo.
5. APU permanece placeholder. Nenhum áudio correto produzido. $6000-$7FFF
   ainda retorna zero; não foi comprovado hardware de expansão adicional.
6. S006 validou pixels/RAM em dois cenários contra referência independente;
   demais cenários, áudio e alterações específicas da ROM não foram auditados integralmente.
7. LeakSanitizer falhou no ambiente sob ptrace (/proc inacessível). Repetido com
   ASAN_OPTIONS=detect_leaks=0: AddressSanitizer e UBSan passaram. Não afirmar
   que leak detection foi validado.
8. Push por git HTTPS local não tinha credencial. Publicação foi feita pela
   conexão GitHub autorizada, com expected HEAD, e árvore remota conferida.
9. CI do commit 4c438b85fdb27b4deaf6cfec319e66ad2586b50c foi consultado e
   concluiu success: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37561513602.
   As medições da ROM acima são locais. Não transferir esse sucesso
   automaticamente para commits futuros.

## Reproduzir

```sh
sh tests/run_host_tests.sh
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes \
  --instructions 5000000 --input tests/canonical_start_right.input \
  --frame build/gameplay.bin
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes \
  --instructions 50000000 --input tests/canonical_start_right.input \
  --frame build/long-run.bin
```

Não mudar a ROM para fazer o runtime passar. O runner direto só verifica header
e tamanho: use o wrapper que verifica SHA-256 de ROM/PRG/CHR.
Quadro exportado: 256x240 índices NES, sem RGB; hash independente de paleta visual.
Roteiro `tests/canonical_start_right.input`: Start nos frames 100–101,
direita+A em 220–269, direita em 270–359, soltar depois.

## Próximo bloqueio e sequência

Prioridade imediata: comparar quadros/VRAM/scroll com uma referência usando
EXATAMENTE a mesma ROM; localizar divergências demonstráveis e ajustar a PPU sem
patch de gameplay. Ampliar conformidade CPU com testes independentes.
Depois: entrada interativa host, áudio/APU e backend Xbox mínimo seguro.

Xbox: **nenhum novo XEX produzido ou testado nesta sessão.** Build histórico #16
teve Fatal Crash imediato. Estado de boot-test.xex não foi revalidado aqui.
Não ligar core novo ao vídeo antigo com endereços hardcoded.
A futura entrega deve embutir PRG/CHR canônicos no default.xex, sem ROM externa,
e o host usa agora a ROM versionada em assets/canonical/ para validação.

## Referências técnicas consultadas

Somente semântica de hardware, nunca substituição do gameplay:
- https://www.nesdev.org/wiki/CPU_interrupts
- https://www.nesdev.org/wiki/PPU_programmer_reference
- https://www.nesdev.org/wiki/PPU_memory_map

Antes de finalizar a próxima sessão, atualizar este checkpoint com medidas
novas e limites reais e confirmar commits no GitHub.

## Atualização de progresso — ciclo 20/10/2026 (dispatcher verificado e PCM comparado)

**CI:** [39432801245](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39432801245) — success.

- Perfil de dispatcher validado contra a ROM canônica: 18 candidatos brutos `JSR $8E04`, 12 callsites observados, 57 pares callsite/seletor e 47 destinos ROM únicos.
- A configuração temporária recebeu 17 seeds adicionais derivados do perfil. A configuração versionada e a ROM canônica não foram modificadas.
- A tentativa de seed de todos os PCs do miss-log foi rejeitada (run 39429365074; 2.072 entradas sequenciais indevidas). A instrumentação agora registra chamadas ao dispatcher inline e valida diretamente os bytes da ROM.
- Runtime sem interpretador: arquivo `cpu6502_interp.c` ausente de Ninja e símbolo `cpu_interp_step` ausente; quatro roteiros passaram com 100,0% dos ciclos nativos e sem `NATIVE_COVERAGE_MISS`.
- Paridade `--hash-out`: todos os 6.600 quadros em quatro roteiros; PCM/WAV 48 kHz mono 16-bit idêntico byte a byte nos quatro cenários.

**Percentuais estimados:** ROM/geração C isolada 100%; remoção estrutural do interpretador experimental 100%; equivalência por quadro e PCM 100% apenas nos roteiros amostrados; cobertura integral permanece em aberto; Xenon/PowerPC/XEX e teste físico 0%; **progresso funcional estimado 20%**.

Evidência detalhada em [INLINE_DISPATCH_PROFILE_20261020.md](evidence/INLINE_DISPATCH_PROFILE_20261020.md). O código C gerado não foi publicado. Não gerar XEX nem solicitar teste físico.

### Próximo passo
Enumerar os 18 candidatos a `JSR $8E04`, suas tabelas inline e os seis callsites candidatos que não apareceram no perfil. Expandir os roteiros apenas após análise do PRG; manter gates de miss, hash/frame e PCM. PR #20 aberto; PR #19 pausado.


## Plano mestre ponderado — revisão 21/10/2026

- Plano de referência: [`docs/PROJECT_PLAN_AND_PROGRESS.md`](PROJECT_PLAN_AND_PROGRESS.md).
- CI mais recente confirmada: [run 39434891134](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39434891134) — success.
- Base ponderada do progresso: **22%** (estimativa, não medição de linhas/tempo). A estimativa anterior de 20% era qualitativa; o novo documento declara pesos e contribuições por etapa.
- Entregáveis já provados: identidade da ROM; geração C experimental; remoção estrutural do interpretador no build experimental; 57 pares de dispatcher verificados contra o PRG; quatro cenários host sem interpretador; 6.600 quadros com hashes iguais e PCM/WAV idêntico byte a byte.
- Ainda não provados: cobertura integral, arquitetura/licença final aprovada, conformidade integral NES, toolchain Xenon/PowerPC, XEX final, integração Xbox e QA de release.
- Próxima tarefa: analisar os seis candidatos brutos ainda não observados (`$83A3`, `$921B`, `$9743`, `$C7A8`, `$C987`, `$D2E5`) e decidir quais são alcançáveis/código com base na ROM; criar roteiros determinísticos apenas com justificativa.
- Nenhum XEX final criado ou teste físico solicitado. PR #20 continua aberto; POC próprio em PR #19 permanece pausado.
