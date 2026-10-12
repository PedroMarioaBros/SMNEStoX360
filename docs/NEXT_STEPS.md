# Próxima ação — após resultado físico S015, 12/10/2026

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


## Confirmação importante (12/10/2026): ABadAvatar em entrada automática

O proprietário confirmou **ABadAvatar** na versão mais recente que usa, ativado automaticamente. Não continuar perguntando qual é o **entrypoint**: essa dúvida está resolvida. **Ainda não se sabe qual payload pós-exploit** (`FreeMyXe`, `XeUnshackle` ou outro) roda em `BadUpdatePayload`; o README oficial ABadAvatar https://github.com/shutterbug2000/ABadAvatar informa que ABadAvatar **não inclui XEX**, logo não determina sozinho como é autorizada/lançada uma aplicação XEX. Perguntar apenas se o pós-boot mostra FreeMyXe, XeUnshackle ou segue direto ao Aurora; se oculto, leitura passiva de nomes de arquivos pode ajudar. Não exigir atualização, substituição ou alteração de `launch.ini`, NAND, payload nem reinício de exploit por causa da investigação. Não declarar causa do Fatal Crash. A S015 continua vetada e a S016 permanece sem validação física; examinar primeiro a compatibilidade do patcher específico e os cabeçalhos/assinaturas XEX.


## Condição anterior ao próximo teste — desbloqueio SOFTWARE (12/10/2026)

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
