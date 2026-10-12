# Próxima ação — após resultado físico S015, 12/10/2026

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
