# S014 — primeiro XEX2 do runtime canônico, diagnóstico sem framebuffer (09/10/2026)

## Fonte e limites de conteúdo

- HEAD inicial da `main`: `cdcda47f9952afa92b09228a4da100a463ef97c0`.
- Branch de desenvolvimento: `work/s014-canonical-xex-20261009`.
- **Somente** `assets/canonical/SMB_v026.nes` do proprietário, 40.976 bytes.
  Nenhum arquivo ROM/PRG/CHR foi alterado e **nenhuma** cópia externa
  do jogo foi usada.
- SHA-256 ROM: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- SHA-256 PRG: `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`.
- SHA-256 CHR: `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- `tools/verify_repository.py` e checagem separada de três SHA-256
  executadas antes de qualquer compilação Xbox. `.incbin` embute
  **o arquivo NES completo** no executável, com PRG iniciando no
  offset 16 e CHR no offset 32784; sem seletor de ROM.

## Entrega técnica

1. `src/platform/canonical_xex/main.c`: entrypoint independente
   para o Xbox-OS, com `smb360_machine_init`, execução real do PRG
   6502 da ROM do proprietário por `smb360_machine_step`,
   processamento por 120 quadros (VBlank), detecção de opcode
   não suportado/timeout, checksum FNV-1a dos pixels e
   registros de diagnóstico `printf` nos quadros 0/59/119.
   O checksum, o resultado do loop e a impressão **não foram
   executados em hardware**: somente estão compilados no XEX.
2. `src/platform/canonical_xex/embedded_rom.S`: símbolo no
   `.rodata` ligado via `.incbin "assets/canonical/SMB_v026.nes"`,
   ROM inteira, sem gameplay de terceiros.
3. `scripts/build-canonical-xex.sh`: target OpenXeChain
   `-std=c99 -O2 -Wall -Wextra -Werror` com **todos os fontes
   `src/canonical/*.c`**, entrypoint próprio e `embedded_rom.S`.
   Gera `build-canonical-xex/canonical.exe`, `default.xex`,
   SHA256SUMS e BUILD_MANIFEST.
4. `.github/workflows/canonical-xex.yml`: novo CI independente
   de `xbox360-cloud-build.yml` legado; utiliza OpenXeChain
   pin `eed1fa65bf9577fd31625764b320a90182ea9ade`,
   cache e fallback de toolchain, confere magic XEX2,
   SHA256SUMS, manifest, e publica artifact
   `smb360-CANONICAL-DIAGNOSTIC-xex`, retenção 30 dias.
   Nenhum componente `nathsou/smb` é compilado ou baixado.
5. `.github/workflows/canonical-host.yml` foi configurado
   para disparar testes host ao alterar o target Xbox canônico
   ou seu builder. Isso é regressão, não prova de boot.

## Testes realmente observados

- Primeira compilação cruzada:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958041952
  **failure**: `main.c:2:60: error: '/*' within block comment
  [-Werror,-Wcomment]`. O aviso era provocado pelo padrão
  `src/canonical/*.c` dentro de um comentário C. Corrigido
  somente o comentário no commit
  `75eb79a35b6c60ff8ed4d4e17a88404af89c2325`,
  sem alterar jogo/runtime/testes.
- Workflow do mesmo commit corrigido:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958120419
  status **completed/success**, job `OpenXeChain + canonical core`.
  Logs confirmam verificação da ROM, PRG e CHR, toolchain
  `ppc32-unknown-xbox360`, compilação do C99, SynthXEX
  `XEX built`, magic XEX2, verificação de SHA256SUMS
  e manifesto `CANONICAL_DIAGNOSTIC_NOT_PLAYABLE`.
- Artifact publicado e conferido no workflow:
  `smb360-CANONICAL-DIAGNOSTIC-xex`, id `11630285368`
  (arquivo zip da plataforma, tamanho compactado reportado
  de 153453 bytes). Contém `default.xex`, `canonical.exe`,
  `SHA256SUMS.txt` e `BUILD_MANIFEST.txt`.
  Acesso em **Actions → run 37958120419 → Artifacts**.
- Aviso da toolchain documentado: `lld-link: warning:
  /align specified without /driver; image may not run`.
  A existência de XEX2/compilação **NÃO garante boot**.
  Sem acesso físico ao console, nenhum teste de boot ou
  quadro mostrado no Xbox foi realizado nesta sessão.
- A validação host da S013 continua sendo o baseline,
  com 13 suítes normais/san, comparação binjnes
  pixels/RAM 600+600 e captura PCM. Testar novamente
  se qualquer novo código no core for alterado.

## Escolha de segurança

O target novo **não chama** `video_fb.c` legado (endereços
MMIO hardcoded que provocaram Fatal Crash no build #16),
não escreve no framebuffer e não usa `audio_xex.c` antigo.
Ele é um **diagnóstico headless**, ainda não um jogo jogável.
Não anuncia tela/controles nem áudio no Xbox. A futura camada
de vídeo precisa usar API de apresentação validada,
sem dereferenciar endereços descobertos empiricamente.

O PRG 6502 ainda é interpretado em C pelo runtime existente;
a compilação dele para instruções PowerPC **não é uma
tradução nativa estática do jogo sem interpretador**.
Esse requisito do proprietário segue pendente.

## Teste físico pendente (somente proprietário do hardware)

Se decidir testar o artifact diagnóstico, preservar os quatro
arquivos e seus SHA256SUMS. Executar o `default.xex` apenas
como diagnóstico do ambiente Xbox desbloqueado.
Resultado esperado **se o console conseguir executar**:
código headless roda até 120 quadros, registra hashes/PC/ciclos
se o ambiente fornecer stdout e retorna. Não espere ver Mario
na tela. Registrar modelo de desbloqueio, dashboard, se aparece
Fatal Crash, se retorna ou trava; não afirmar que foi
executado enquanto esses dados não forem fornecidos.

## Próxima sessão

S015: reforçar boot/tracing do target canônico em hardware,
resolver ou isolar o aviso de linker, integrar entrada de controle
e um caminho de apresentação gráfico seguro/validado; só então
testar PCM/Xbox áudio. Em paralelo, projetar tradução 6502→PPC
sem interpretador, mantendo o PRG original; comparar waveform
com binjnes e refinar DMC DMA/IRQ.
Instruções operacionais atualizadas em `docs/NEXT_STEPS.md`.
