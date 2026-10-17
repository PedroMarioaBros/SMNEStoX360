# S018 — Hardware S017 observado e novo diagnóstico PowerPC com telemetria visível

> **CORREÇÃO HISTÓRICA EM 17/10/2026:** S018 foi TESTADA
> no console pelo proprietário e produziu **Fatal Crash
> Detected**. Textos abaixo que dizem "ainda não testada"
> descrevem apenas a situação quando foram escritos,
> **NÃO a situação atual**. S018 NÃO deve ser
> reexecutada. Ver [S019](../S019/README.md) e
> [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).


## Histórico físico confirmado pelo proprietário em 13/10/2026

Em console Xbox 360 com entrada de desbloqueio via software
ABadAvatar e dashboard Aurora:

- **S017 estágio 01 (CRT-free, sem ROM)**: ao executar
  `default.xex`, ficou **tela preta e congelamento**,
  sem Fatal Crash e sem possibilidade de abrir menu,
  dashboard original ou retornar ao Aurora.
  **Conclusão: FALHA FÍSICA / BLACK SCREEN FREEZE.**
- **S017 estágio 02 (CRT-free, ROM canônica embutida)**:
  ficou em **tela preta**, sem mensagem de erro e
  **o Aurora foi reiniciado/relançado automaticamente**.
  A conclusão dos 120 quadros, sucesso do núcleo C99
  e o mecanismo exato do retorno NÃO foram demonstrados.
  O binário tinha caminhos de erro `return 0`
  por ROM/CPU/watchdog, todos chegando a um encerramento
  aparentemente parecido com o de sucesso;
  a única distinção era 1 s versus 5 s de atraso,
  sem cronometragem fornecida.
- **S015** anterior deu **Fatal Crash imediato**.
  Nenhuma das três ocorrências demonstra boot
  ou Mario jogável com vídeo nativo.

Evidências: [issue #12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12)
e [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).
Relato do próprio proprietário, sem inferir causa do crash
ou tempo transcorrido. Não pedir reteste desses XEX antigos.

## Análise de fonte após relato

O estágio 01 tem somente `KeDelayExecutionThread`,
`XamLoaderTerminateTitle()`, `ExTerminateThread()`
e um `for(;;)` caso o fallback retorne.
Esse loop pode explicar um travamento, mas a
observação de tela preta sem telemetria não permite
provar que o entrypoint sequer iniciou ou que
o código atingiu o loop.

O estágio 02 tinha a ROM exata e passava ao mesmo
caminho de retorno sem indicadores visíveis.
A observação de Aurora retornando é compatível
com vários mecanismos distintos: encerramento
normal, erro precoce ou recuperação pelo gerenciador.
Não é prova de CPU 6502/PPU PASS.

## Solução experimental S018

Arquivos novos:
- `src/platform/canonical_xex/native_crtless_status.c`:
  XEX Xbox PPC com entrada `smb360_entry` diretamente
  ligada, **sem Newlib CRT**; mesmos `src/canonical/*.c`
  do runtime e mesma ROM `assets/canonical/SMB_v026.nes`
  embutida. Aguarda incrementos concretos do contador
  `machine.ppu.frame` por 120 vezes, 1.000.000
  instruções limite cada quadro, verificando erro de
  `smb360_machine_step`. O frame count é uma
  observação *interna*, não benchmark independente
  nem confirmação por hardware.
- Avisos do sistema, em UTF-16 estático, via
  `XNotifyQueueUI` (XAM ordinal 656), função
  identificada em Freestyle Dashboard, Xenia
  e em homebrew FreeMyXe:
  `SMB360 S018 CPU START`,
  `SMB360 S018 ROM INVALID`,
  `SMB360 S018 CPU STOP`,
  `SMB360 S018 FRAME TIMEOUT`,
  `SMB360 S018 120 FRAMES PASS`.
  Em caso de bloqueio/supressão XAM, **ausência
  de notificação não comprova falha nem sucesso**.
  O S015 tinha `printf` + XAM e deu Fatal Crash,
  logo a API de notificação também deve ser tratada
  como experimental, com risco de crash/freeze.
- `scripts/build-xex-s018-status.sh` gera um XEX
  específico distinto de S017, sem usar nathsou,
  Newlib, MMIO GPU ou áudio. Não é jogo jogável.
- `tools/verify_crtless_xex.py` aceita opção
  `--allow-notifications` **somente sob seleção
  explícita**: exige quatro importações em vez das
  três originais. O comportamento estrito S017
  sem a flag permanece inalterado.
- `.github/workflows/xex-status.yml` compila,
  verifica cabeçalhos, entrypoint/imports, SHA-256
  e ROM integral única. Emite artifact
  `smb360-S018-STATUS-NATIVE-XEX` com guia
  `docs/TESTE_FISICO_S018.md`.

Nenhum novo teste no Xbox foi realizado nesta sessão.
A CPU do NES ainda é **interpretada** por código C
que roda no PowerPC, não é tradução PPC estática.
SEM framebuffer e áudio Xbox reais.
**Não afirmar que o Mario pode ser jogado**.

## Validação pendente

CI da branch `work/s018-visible-progress-20261013`
deve completar antes de sugerir testar o novo
arquivo. Checar também host tests, hashes ROM/PRG/CHR,
manifesto, logs `lld-link`, retorno XAM e status real
da PR. Manter registro de falhas e não
substituir o exploit, NAND ou Aurora.

## Publicação e verificação final

- PR #16 https://github.com/PedroMarioaBros/SMNEStoX360/pull/16
  merge `f94f8acd553777ea198d617ced63d924c9cde3a4`
  (nenhuma alteração em `assets/canonical/`).
- CI pós-merge
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357552
  **completed/success** XEX S018;
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357745
  **completed/success** host;
  `38416357481` S017 e `38416357713`
  regressão canônica **success**.
- Artifact S018 `smb360-S018-STATUS-NATIVE-XEX`
  #11824027270 (92383 B ZIP, válido até 12/11/2026);
  `default.xex` 397312 B SHA256
  `c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`.
  Os checksums foram conferidos em ZIP extraído
  junto ao manifesto `kind=S018_CANONICAL_VISIBLE_STATUS_UNTESTED`.
  Proprietário recebeu artifact individual + pacote completo.
- **A entrega não substitui o teste físico**: S018 ainda
  não foi executado no Xbox, pode não mostrar notificações
  ou até travar; o teste é opcional. A ROM permaneceu
  byte-idêntica e o backend Xbox ainda não renderiza Mario.
