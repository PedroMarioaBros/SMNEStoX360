# S016 — crash real S015 e isolamento de boot (12/10/2026)

## Entrada e observação física, independente de CI

HEAD `main` inicial:
`2a7a2f6af127adbbb8a025b801d2612fdba55561`.

O proprietário informou que baixou o pacote S015, que continha
`BUILD_MANIFEST.txt`, `canonical.exe`, `default.xex`,
`SHA256SUMS.txt` e `TESTE_XBOX_S015.md`. Executar
`default.xex` causou **Fatal Crash imediatamente no Xbox 360**;
foi necessário reiniciar o console. Não há mensagem de stack,
PC, erro XAM ou outro log; não se sabe se `main` iniciou.
Este resultado **substitui** a suposição S015 de boot "não
testado": S015 foi **TESTADO E FALHOU**, definitivamente.
Issue [#12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12).

Build S015 afetado:
- código merge `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`
- https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966878615
  `success` *somente* compilação PowerPC/XEX2
- artifact `smb360-CANONICAL-DIAGNOSTIC-xex` #11634357018
- `default.xex`, 528384 B; SHA-256
  `48a9071765d3b9db8cfe9cfb91423b5f437ce990d262e380cd7bdd01dea7d077`
- Nenhum registro de alteração ou corrupção da ROM original.

**Ordem ao próximo agente:** NUNCA instruir a executar novamente
S015; remover de recomendações "pronto para teste" e usar S016.

## Engenharia e suspeitas (não comprova causa)

O entrypoint S015 fazia `printf` e invocava
`XNotifyQueueUI` já no início, antes de validar a ROM;
também importava `XamInputGetState`, OpenXeChain CRT
e outras chamadas. O linker avisou
`/align specified without /driver; image may not run`.
O container XEX2/PE existe, mas isso não garante carregamento
pelo kernel, imports certos ou execução.

Para separar hipóteses, novos códigos:
- `src/platform/canonical_xex/boot_kernel_only.c`: programa
  mínimo sem ROM, stdio, vídeo, input ou chamadas *diretas*
  à XAM, aguarda ~5 segundos via `KeDelayExecutionThread`
  e retorna.
- `src/platform/canonical_xex/boot_canonical_no_xam.c`:
  ROM original e `src/canonical/` (CPU 6502/PPU) por
  120 quadros, sem `printf`, `XNotifyQueueUI`,
  `XamInputGetState` ou framebuffer, aguarda ~5s
  após sucesso e retorna.
- `scripts/build-xex-boot-isolation.sh` constrói
  `01_KERNEL_ONLY/default.xex` e
  `02_CANONICAL_NO_XAM/default.xex` com toolchain
  OpenXeChain pinada, compilador C99 `-Wall -Wextra -Werror`,
  cabeçalho PE PowerPC 0x01f2, XEX2, manifestos
  e SHA256SUMS independentes. Apenas segunda imagem
  inclui ROM original 40976 B byte-idêntica.
- `.github/workflows/xex-boot-isolation.yml`
  roda essas verificações e arquiva apenas os dois
  diagnósticos, com guia `docs/TESTE_FISICO_S016.md`.
  Esses XEXs NÃO foram executados no console.

### Descoberta importante: importação XAM pelo CRT

A primeira tentativa do workflow S016
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204175107
falhou deliberadamente numa assertiva que exigia ausência
de `xam.xex` no PE. Extraído o artifact de debug
#11726229014 e inspecionada a `boot.exe` das duas
variantes, constatou-se que **ambas** importavam:
- `xam.xex@21256.0+1861.0`
- `XamLoaderLaunchTitle`
- `xboxkrnl.exe@21256.0+1861.0`

A própria CRT/ambiente OpenXeChain injeta a importação
XAM do fluxo de encerramento do título **sem nenhuma
chamada XAM explícita no fonte de diagnóstico**.
Logo, anunciar "nenhuma importação XAM" teria sido falso.
Os testes corrigidos verificam que não há
`XNotifyQueueUI` e `XamInputGetState` em nenhuma
das variantes, sem prometer eliminação de todas as
dependências de `xam.xex`. Isso **não demonstra**
que a CRT causou o Fatal Crash.

Na próxima tentativa, ambos os PE foram
**compilados e passaram suas verificações binárias**
(magic, CPU, checksum de ROM e ausência de funções
XAM explícitas). Workflow
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204354807
concluiu `failure` somente porque o YAML continha
um `grep` shell sem aspas corretas; o log mostra
`S016 BOOT ISOLATION build PASS` antes desse passo.
Esse erro de CI também foi corrigido e será revalidado.

Uma edição subsequente do workflow tinha duplicado o
final do YAML, gerando uma execução sem jobs válida
(`38204319276`); conteúdo foi limpo e o workflow
normal voltou a executar. Erros e correções preservados.

## Regras de segurança / próximo passo

- S015 comprovadamente **FATAL CRASH**, não "passou".
- S016 ambos os executáveis novos **hardware UNTESTED**;
  *até mesmo o kernel-only pode causar novo crash*
  se toolchain, loader ou ambiente estiverem incompatíveis.
- Teste físico S016 é **opcional e sequencial**:
  01 sozinho; 02 só se 01 retornar normalmente;
  interromper ao primeiro crash. Não alterar NAND,
  firmware, exploit, jogos, Aurora ou outras pastas.
- Falta resolver ou isolar warning `/align` e
  analisar formato/versão/imports do XEX.
  Vale confirmar também tipo de desbloqueio
  RGH/JTAG/FreeMyXe antes de qualquer novo teste.
- Host C99 original permanece validado mas não equivale
  a execução Xbox; port nativo estático 6502→PPC
  ainda não foi construído.
- Integridade da ROM canônica preservada:
  SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
