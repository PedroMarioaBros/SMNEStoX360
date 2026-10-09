# S015 — diagnóstico Xbox observável, dois gamepads e verificação do PE

Data 09/10/2026, fuso America/Sao_Paulo.
HEAD inicial da main: `884c5db3defec68fbf58793fccbbc69a225584c2`.
Branch `work/s015-observable-boot-20261009`.
A reorganização documental anterior já classificava `xbox360-cloud-build.yml`
como legado/manual. Nenhum código do `nathsou/smb` entrou neste target.

## Preservação da ROM e autenticidade binária

- `assets/canonical/SMB_v026.nes`, 40976 bytes.
- ROM SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- PRG SHA-256 `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`.
- CHR SHA-256 `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- Nenhum arquivo em `assets/canonical/` alterado; a ROM inteira,
  e não apenas CHR ou dados recriados, continua embutida via
  `src/platform/canonical_xex/embedded_rom.S`.
- Novo `tools/verify_canonical_xex.py` analisa a **saída PE**
  (`canonical.exe`): assinatura MZ/PE, máquina PowerPC,
  seções, presença **byte a byte dos 40976 bytes da ROM**
  em um único offset, e SHA-256 do PE e do XEX2.
  É executado obrigatoriamente pelo build, além de
  `tools/verify_repository.py`.
- Build observado https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37965972564:
  `completed/success`, PE `machine=0x01f2`, quatro seções;
  ROM integral presente no offset bruto PE `0x16c00`;
  PE SHA-256 `b9a51e70e200d6008a1c17283685f58b09461bfbfdf8d1d992c5f5ddda28eb5a`,
  XEX SHA-256 `ef477319cd0925192c43153f1665fb33180121d81cd665df91eb64965699ab63`.
  Esses hashes são **de um commit intermediário** e mudarão
  se o código/manifesto mudar; nunca confundir com hash final.

## Código novo S015

- `src/canonical/controller_map.[ch]`: conversão Xbox 360
  XInput → oito botões do NES (A/B/Select/Start/cima/baixo/
  esquerda/direita), cancelamento de direções opostas.
- `tests/test_controller_map.c`: valida os oito botões
  individualmente, combinações, direções incompatíveis e
  leitura serial real dos dois controles pelo barramento
  `$4016/$4017` sem mudar o PRG original.
  `tests/run_host_tests.sh` agora cobre 14 suítes C99 normais
  e 14 sob ASan/UBSan.
- `src/platform/canonical_xex/main.c` agora chama a
  função XAM `XamInputGetState` para os usuários 0 e 1
  por quadro e transmite botões aos dois controladores
  canônicos. Tem notificações de sistema XAM
  `XNotifyQueueUI` de START/PASS e erro, e processamento
  com limite de 120 quadros e duração finita.
  O texto UTF-16 reside em armazenamento estático,
  não em ponteiro local que expira. Nenhuma escrita em
  endereços MMIO/framebuffer antigos.
  A implementação de entrada usa os exports oficiais
  `XamInputGetState @401` e `XNotifyQueueUI @656`,
  conferidos no `xam/xam.def` do
  https://github.com/OpenXeChain/xecorelib .
- `docs/TESTE_FISICO_S015.md` descreve passo a passo
  o download, instalação isolada e observação de START/PASS,
  Fatal Crash, travamento, retorno ao dashboard.
  O build copia esse guia para `build-canonical-xex/TESTE_XBOX_S015.md`
  e o CI publica junto com o `default.xex`.
- Manifest agora deixa explícito
  `build_kind=CANONICAL_DIAGNOSTIC_NOT_PLAYABLE`,
  `video_backend=XAM_NOTIFICATIONS_ONLY`,
  `input_backend=XAM_INPUT_TWO_PADS`,
  `probe_mode=120_FRAMES_WITH_XAM_START_PASS_NOTICES` e
  `xbox_hardware_validation=NOT_TESTED`.

## Falhas intermediárias e correções

- Após ajustar o manifesto, workflow
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966195621
  registrou `failure` na etapa que ainda fazia
  `grep '^video_backend=NONE_SAFE_HEADLESS$'`.
  O compilador, SynthXEX e a verificação PE/ROM completa
  **tinham passado**; falhou um assert antigo do CI,
  atualizado posteriormente para `XAM_NOTIFICATIONS_ONLY`.
- O aviso preexistente do LLD
  `/align specified without /driver; image may not run`
  ainda aparece. Ele é produzido no código de `lld-link`
  quando se passa `/align` sem `/driver`.
  O teste de upstream
  https://sources.debian.org/src/llvm-toolchain-22/1%3A22.1.8-1~deb13u4/lld/test/COFF/align.s
  documenta o mesmo aviso genérico. **Não há evidência
  de que mudar para `/driver` seja apropriado para XEX**.
  Não mascarar aviso ou alegar que ele causou crash sem
  teste físico. Os cabeçalhos PE PowerPC são válidos
  estruturalmente segundo nosso verificador, mas isso
  não prova que o kernel aceitará o executável.

## Distinção essencial: compilação ≠ hardware

- XEX compilado com chamadas XAM não significa que o serviço
  mostrará avisos numa configuração real do Xbox. O pop-up
  pode ser suprimido, fazendo a ausência de notificação
  um resultado inconclusivo.
- A execução de 120 quadros, input e notificações **só foi
  validada por compilação e testes C99 sintéticos**;
  ainda não houve execução no Xbox 360.
- Não há desenho de Mario na tela nem áudio do console.
  A XAM fornece potencialmente **notificação do dashboard**,
  não é um frontend gráfico próprio.
- O runtime PowerPC usa um interpretador C da CPU 6502,
  não tradução estática 6502→PPC sem interpretador.
- O build histórico que usa nathsou permanece separado.

### Próximo passo exato

Após PR merge e retestes CI da main, o proprietário deverá
fazer o **teste físico de boot S015**, seguindo
`docs/TESTE_FISICO_S015.md` do artifact final de 30 dias.
Registrar se aparecem START/PASS, travamento/Fatal Crash,
retorno ao dashboard, tempo e dashboard/desbloqueio usado.
Só promover boot a `VALIDADO_HARDWARE` com resultado real.
Em paralelo avançar proposta de backend gráfico Xbox seguro,
com API testada — **não** endereços hardcoded do legado.

## Integração PR #11

- Diff revisto: **15 arquivos**; nenhum `assets/canonical/`
  alterado.
- PR #11 https://github.com/PedroMarioaBros/SMNEStoX360/pull/11
  incorporada por squash na main
  `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`.
- Commit do código validado:
  `94a01f0af3798984d5ff88d16800690db10a64fc`.
  - XEX: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966684272
    `success`, artifact `smb360-CANONICAL-DIAGNOSTIC-xex`
    id `11633472546`, inclui `TESTE_XBOX_S015.md`.
  - Host: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966684264
    `success`, quatro jobs (normal, san, referência, SDL).
  - O mesmo XEX passou por `tools/verify_canonical_xex.py`:
    PE PowerPC `0x01f2`, quatro seções,
    ROM de 40976 bytes no offset PE `0x16c00`.
- Pós-merge: workflows `37966878615` e `37966878630`
  checar antes de declarar pacote da main pronto para teste.

## Regressão após merge na main

- Código em main gerado por squash PR #11:
  `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`.
- GitHub Actions **host** no merge:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966878630 ,
  **completed / success**, quatro jobs:
  normal `host (0)`, ASan/UBSan `host (1)`,
  referência binjnes, frontend SDL.
- `test_controller_map: PASS` nos dois modos,
  14 suítes host; idle pixels/RAM 600/600,
  input pixels/RAM 600/600, SDL readback 61440,
  PCM WAV 479146 samples sem clipping e hash
  `3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.
- O XEX no merge corresponde ao workflow
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966878615 ;
  sua conclusão deve ser registrada separadamente.
- A versão completa da branch `94a01f0af3798984d5ff88d16800690db10a64fc`
  já passou no workflow de XEX
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966684272 ,
  artifact `smb360-CANONICAL-DIAGNOSTIC-xex`
  #11633472546 contendo guia ao proprietário.
