# Próxima ação — após feedback físico S017 (13/10/2026)

## Fechamento implementação S018 (13/10/2026)

- PR [#16](https://github.com/PedroMarioaBros/SMNEStoX360/pull/16)
  merged `f94f8acd553777ea198d617ced63d924c9cde3a4`.
- CI main S018 https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357552
  **success**, host `38416357745`, S017 regressão
  `38416357481`, canônico `38416357713` success.
- Artifact S018 #11824027270 ZIP com XEX válido
  397312 B SHA-256 `c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`;
  ROM intacta. Arquivo já entregue ao proprietário no chat.
- **S017 01 congelou**; **S017 02 retornou ao Aurora
  sem evidência de 120 frames concluídos**. Não pedir
  reteste de S015/S017 nem atribuir boot PASS.
- Próximo resultado útil: observar se S018 mostra
  avisos START/ROM INVALID/CPU STOP/TIMEOUT/PASS,
  e se retorna ou congela. **S018 ainda não foi
  testada no Xbox** e chamar XAM pode travar.
  A decisão de teste pertence ao usuário, em pasta
  isolada, sem tocar em NAND/ABadAvatar/Aurora.
- Se XAM não mostrar avisos, **não insistir cegamente
  em novas notificações**. Buscar método de tela segura
  oficial da plataforma Xbox 360, ou evidência
  de execução por depuração/telemetria suportada.
- Só declarar próximo marco de jogo portado quando
  gameplay/render/input nativos forem testados
  fisicamente. CPU 6502 ainda interpretada em C PPC.


## Hardware: S017 01 FALHOU; S017 02 VOLTOU AO AURORA, sem prova de PASS

O proprietário executou ambos os XEX: estágio 01 teve **tela
preta congelada**, sem menu nem Fatal Crash; estágio 02 ficou
em **tela preta** e **Aurora relançou automaticamente**, sem
mensagem de erro. Não assumir CPU/PPU 120 quadros PASS ou
retorno voluntário pela XAM. Não repetir S015, S017 01 ou
S017 02. A causa do retorno ou congelamento segue aberta
(issues #12 e #15). Identidade exata do patcher não bloqueia
a engenharia, conforme ordem expressa do proprietário.

## Implementação S018

O novo arquivo `src/platform/canonical_xex/native_crtless_status.c`
incorpora ROM exata e CPU/PPU canônicas em XEX PowerPC Xbox sem
Newlib CRT, com um novo canal opcional de notificação
`XNotifyQueueUI`: `CPU START`, `ROM INVALID`,
`CPU STOP`, `FRAME TIMEOUT`, `120 FRAMES PASS`.
Esse último só é emitido após observar 120 incrementos
de `machine.ppu.frame`, com watchdog de 1.000.000 de
instruções por quadro. Avisos suprimidos = inconclusivo.
Não existe Mario gráfico e a CPU 6502 continua interpretada C.

Workflow `.github/workflows/xex-status.yml`,
script `scripts/build-xex-s018-status.sh`, validador
`tools/verify_crtless_xex.py --allow-notifications`,
[guia](TESTE_FISICO_S018.md).
CI branch [run 38415557401](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38415557401)
**success**. Antes de oferecer qualquer novo teste, conferir
PR e CI da main no **commit exato**, SHA do XEX e manifest.
Testar S018 envolve risco real de freeze/Fatal Crash; não
apagar/instalar firmware, exploit, Aurora ou substituir
`BadUpdatePayload/default.xex`. A decisão física é do dono.

Próxima engenharia se START/PASS não aparecer: isolar XAM UI,
corrigir caminho de término de título `XamLoaderTerminateTitle`
sem inferir que `ExTerminateThread`/loop causou 01 travar;
investigar importações/cabeçalhos e alternativa de retorno.
Para objetivo principal, planejar saída vídeo Xbox por API
segura (sem MMIO antigo) + input e áudio, depois tradução
direta estática NES 6502→PPC. Não declarar jogo funcional.

---

# Histórico de próximos passos S017 e anteriores


### Resultado fechado S017, 12/10/2026

- PR [#14](https://github.com/PedroMarioaBros/SMNEStoX360/pull/14)
  merged `40825ff3530d426b05d3e23b4ab02b8d82f12ebe`.
- CI S017 do commit merge
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754775
  **success**, duas imagens nativas PowerPC geradas sem Newlib CRT,
  importando três funções Xbox OS (kernel/XAM).
- Artifact da branch exata com os dois XEX:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206678772
  #11728050710; versão para o proprietário já entregue no chat,
  com SHA256 e verificações. Não entregar novamente S015.
- O proprietário solicitou independência do desbloqueio como
  objetivo de engenharia; **não voltar a bloquear código
  perguntando FreeMyXe/XeUnshackle**. A implementação é
  Xbox OS, não atrelada a API de exploit específico;
  isso não garante aceitação por qualquer ambiente de boot.
- O próximo dado útil é somente o retorno **físico da Etapa 01 S017**:
  voltou ao gerenciador após ~5s ou Fatal Crash.
  **Etapa 02 só depois** de 01 ter retornado normalmente.
  Nenhuma atualização de flash/NAND/boot.
- Depois de boot comprovado, implementar vídeo Xbox por
  API segura, áudio, controles e futuramente tradução
  direta PPC sem emulação. O novo binário canonical ainda
  é somente CPU/PPU sem saída gráfica e com intérprete.
- **Hardware atual**: S015 Fatal Crash real; S017
  etapas 01 e 02 NÃO TESTADAS. Não declarar Mario jogável.


## S017 ativa — XEX nativo PowerPC sem runtime C, boot físico pendente

**PROSSIGA INDEPENDENTEMENTE DO NOME DO PATCHER.** O proprietário pediu um executável Xbox 360 real, sem bloqueio por desconhecimento de FreeMyXe/XeUnshackle. Não prometer compatibilidade universal sem evidência física.

- Workflow S017 `.github/workflows/xex-crtless.yml`, script `scripts/build-crtless-xex.sh`, ponto de entrada `src/platform/canonical_xex/native_crtless_probe.c`; código canonical em `native_crtless_canonical.c` e `native_crtless_memory.c`.
- **01_MINIMAL:** `build-crtless-xex/default.xex`, código mínimo PowerPC, sem Newlib CRT, só 3 imports e retorno após ~5 segundos.
- **02_CANONICAL:** `build-crtless-xex/02_CANONICAL_CRTLESS/default.xex`, ROM canônica embutida e core CPU/PPU 120 quadros, também sem CRT nem backend de vídeo/áudio. Não jogável.
- Antes de novo reteste físico, confirmar artifact **da main** e checksums; verificar explicitamente que nenhum binário é o antigo S015 que deu Fatal Crash. Executar 01 separado; 02 só após 01 retornar ao gerenciador. O risco de crash permanece e o usuário decide testar.
- Validação: `python3 tools/verify_repository.py`, `bash scripts/build-crtless-xex.sh`, `python3 tools/verify_crtless_xex.py --pe build-crtless-xex/crtless.exe --xex build-crtless-xex/default.xex`, verificar `--embedded-rom assets/canonical/SMB_v026.nes` em 02. Rodar host tests e confirmar CI do commit exato.
- A sonda 01 **ainda não representa Mario**, mas removeu o Newlib CRT que contaminava todos os XEX anteriores. O core canônico está compilado no XEX 02. Próximo progresso real: vídeo Xbox por API comprovada, áudio/input e mais adiante geração nativa estática PPC do gameplay.
- Erro SynthXEX `realloc(ptr,0)` em seção `.data` BSS-only diagnosticado e contornado no código C, sem mudar a ROM. Ver `docs/evidence/S017/README.md`.
- Nenhum novo S017 foi testado no Xbox; não confundir CI com boot real, e não declarar garantia de uso em todos os métodos de desbloqueio.

---


## HISTÓRICO S016, SUPERADO EM S017 — entrada ABadAvatar automática

O proprietário confirmou **ABadAvatar** na versão mais recente que usa, ativado automaticamente. Não continuar perguntando qual é o **entrypoint**: essa dúvida está resolvida. **Ainda não se sabe qual payload pós-exploit** (`FreeMyXe`, `XeUnshackle` ou outro) roda em `BadUpdatePayload`; o README oficial ABadAvatar https://github.com/shutterbug2000/ABadAvatar informa que ABadAvatar **não inclui XEX**, logo não determina sozinho como é autorizada/lançada uma aplicação XEX. Perguntar apenas se o pós-boot mostra FreeMyXe, XeUnshackle ou segue direto ao Aurora; se oculto, leitura passiva de nomes de arquivos pode ajudar. Não exigir atualização, substituição ou alteração de `launch.ini`, NAND, payload nem reinício de exploit por causa da investigação. Não declarar causa do Fatal Crash. A S015 continua vetada e a S016 permanece sem validação física; examinar primeiro a compatibilidade do patcher específico e os cabeçalhos/assinaturas XEX.


## HISTÓRICO S016, SUPERADO EM S017 — investigação de desbloqueio software

O proprietário confirmou **desbloqueio por software**, sem RGH/JTAG físico, e disse usar a versão mais recente; **FreeMyXe versus XeUnshackle e suas versões ainda não estão confirmados**. Hipótese mais plausível: BadUpdate/ABadAvatar + patcher, mas não assumir. Antes de solicitar qualquer S016 físico:
1. Perguntar qual tela/nome aparece ao ativar o desbloqueio: **FreeMyXe ou XeUnshackle**; registrar a versão se conhecida, sem pedir CPU key, DVD key ou credenciais.
2. Conferir se o loader é Aurora/XeXMenu e se outros XEX homebrew já carregam. Esse dado separa problema de formato/segurança XEX de startup e API.
3. Analisar `default.xex` S015 e XEX de isolamento S016 quanto a tipo retail/devkit, criptografia, imports e permissões. As recomendações antigas da wiki FreeMyXe sobre retail encryption diferem do README atual, que declara suporte devkit; considerar **versão** antes de converter XEX.
4. Manter S015 vetado por Fatal Crash. Evitar modificações de NAND, flash ou configuração persistente. Até resolver compatibilidade, **nenhuma convocação para novo teste físico**.

Registro permanente: https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .


**BLOQUEIO CONFIRMADO:** o proprietário executou S015 no Xbox 360,
e o `default.xex` produziu **Fatal Crash imediato**; reiniciou
o console. **NÃO REEXECUTAR S015.** Registro:
https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .
Não declarar boot validado, mesmo que o GitHub Actions esteja verde.

Leia `START_HERE.md`, `AGENTS.md`,
`docs/CONTINUITY_PROTOCOL.md`, `docs/WORK_CHECKPOINT.md`,
`docs/SESSION_LOG.md` e
`docs/evidence/S016/README.md`, na ordem de protocolo.
Conferir HEAD remoto antes de editar. Preservar a ROM do
proprietário em `assets/canonical/SMB_v026.nes` de 40976 B
(SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`).

## O que foi implementado na S016

A falha física do S015 não distingue erro de loader/XEX,
startup/ABI/CRT, `XNotifyQueueUI`, leitura de gamepad
nem interpretador 6502. A causa é **indeterminada**.

Arquivos de isolamento, separados de `main.c` S015:
- `src/platform/canonical_xex/boot_kernel_only.c`: sem
  ROM/jogo; aguarda ~5s com `KeDelayExecutionThread` e retorna.
- `src/platform/canonical_xex/boot_canonical_no_xam.c`:
  ROM íntegra embutida, `src/canonical/`, no máximo 120
  quadros, sem chamadas diretas a XAM, sem `printf` ou vídeo.
- `scripts/build-xex-boot-isolation.sh`: produz
  `01_KERNEL_ONLY/default.xex` e
  `02_CANONICAL_NO_XAM/default.xex`, PE PowerPC/XEX2,
  SHA256SUMS e manifestos independentes.
- `.github/workflows/xex-boot-isolation.yml`: CI
  específico, artifact
  `smb360-S016-BOOT-ISOLATION-NOT-PLAYABLE`.
- `docs/TESTE_FISICO_S016.md`: sequência condicional
  e aviso explícito de que o teste ainda pode travar o console.

**Descoberta de linker:** o OpenXeChain CRT importa
`xam.xex` e `XamLoaderLaunchTitle` MESMO para o
programa mínimo que não chama funções XAM diretamente.
O S016 retira só chamadas diretas
`XNotifyQueueUI`/`XamInputGetState`.
Não declarar "sem dependência XAM". Exame preservado
em `docs/evidence/S016/README.md`.

## Próximo comando e critério

Com toolchain OpenXeChain pinada:

```sh
python3 tools/verify_repository.py
bash scripts/build-xex-boot-isolation.sh
(cd build-boot-isolation/01_KERNEL_ONLY && sha256sum -c SHA256SUMS.txt)
(cd build-boot-isolation/02_CANONICAL_NO_XAM && sha256sum -c SHA256SUMS.txt)
```

Também rodar os testes do core que não foram modificados:

```sh
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
python3 tests/test_host_audio.py
```

**PR / CI:** revisar diff, zero alterações na ROM;
comprovar GitHub Actions S016 e C99 regressão no commit
exato. Só integrar quando novos workflows estiverem
verdes; registrar erros reais e correções no diário.

**Hardware:** nunca solicitar reteste do S015. Na S016,
após build verde, o proprietário pode escolher testar
a **etapa 01 isolada**; caso retorne ao menu
normalmente, considerar a 02; parar no primeiro
Fatal Crash. Ausência de teste não é fracasso nem êxito
do novo pacote. Idealmente registrar gerenciador,
tipo de desbloqueio e logs do crash se acessíveis.
Nenhuma mudança de NAND, firmware ou bootloader.

## Próxima engenharia S017

- Investigar se o firmware/desbloqueio aceita
  XEX2 SynthXEX, ABI/imports/startup e warning
  `/align specified without /driver; image may not run`.
- Diferenciar importação XAM pela CRT de chamadas diretas.
- Só depois integrar vídeo via API segura, controles/PCM
  e futura tradução estática 6502→PowerPC desejada.
- Nenhum `default.xex` da rota antiga/nathsou, nem
  endereços de vídeo MMIO hardcoded do Fatal Crash #16.

## Fechamento S016 — build validado, diagnóstico físico pendente

- PR #13 https://github.com/PedroMarioaBros/SMNEStoX360/pull/13
  integrada por squash: `cf10bc38eb1dbf564a83438d60f07dace96766a7`.
- CI exato do merge: `38204726453` isolamento S016,
  `38204726543` host e `38204726479` XEX canônico:
  **três success**.
- Artifact *somente diagnóstico* S016 #11726453526, 177016 B,
  expira em 11/11/2026; duas etapas em pastas diferentes.
- **Não recomendar S015**. Não solicitar automaticamente
  novo teste físico: primeiro confirmar compatibilidade
  do desbloqueio/loader, pois novo crash é possível.
  Se proprietário quiser continuar e estiver informado do risco,
  guiar SOMENTE etapa 01 inicialmente.
