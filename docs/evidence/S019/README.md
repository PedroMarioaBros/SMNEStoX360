# S019 — investigação do Fatal Crash físico do S018 (17/10/2026)

## Evidência diretamente reportada pelo dono do Xbox 360

> "S018 resultou em Fatal Crash Detected"

**Estado do S018: FALHOU EM HARDWARE REAL**, não "não testado"
e não "passou porque o CI estava verde". Relato no contexto de
Xbox 360 com desbloqueio por software ABadAvatar e Aurora.
O proprietário **não informou** duração até o erro nem se
notificações apareceram antes do Crash. Não alegar que foi
imediato, nem que não houve notificação; não inferir stack trace.

S015 também deu Fatal Crash. S017 estágio 01 congelou em tela preta;
S017 estágio 02 mostrou tela preta e relançou Aurora, mas não
provou conclusão de quadros. A S018 **não deve ser executada
novamente** e não há justificativa para reutilizar XEX S015/S017.

- Issue histórica/física: [#12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12)
- Issue dos testes físicos: [#15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15)
- PR da S018: [#16](https://github.com/PedroMarioaBros/SMNEStoX360/pull/16),
  commit squash `f94f8acd553777ea198d617ced63d924c9cde3a4`.
- CI de compilação S018
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357552
  `success` **apenas do processo de build/validação estática**.
- Artifact entregue S018 #11824027270.

## Comparação binária com o MESMO S017 estágio 02 recebido e testado

Comparamos os ZIP originais dos artifacts reais:
- **S017 usado pelo usuário:** artifact #11728050710
  (`smb360-S017-NATIVE-CRTLESS-DIAGNOSTIC` do commit
  `c30d2c47733c35400f0ca871c2844bd502c88176`),
  `02_CANONICAL_CRTLESS/default.xex`.
- **S018 usado pelo usuário:** artifact #11824027270
  (`smb360-S018-STATUS-NATIVE-XEX`), `default.xex`.

| Item | S017 02 | S018 |
|---|---|---|
| Resultado físico | Tela preta, Aurora reabriu, sem prova de 120 quadros | **Fatal Crash Detected** |
| XEX | 397312 bytes | 397312 bytes |
| PE | 68608 bytes | 68608 bytes |
| SHA-256 XEX | `cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee` | `c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0` |
| Entry PowerPC indicado por PE | `0x82015900` | `0x82015900` |
| ROM `SMB_v026.nes` no PE | 1 cópia idêntica | 1 cópia idêntica |
| Importações kernel | `ExTerminateThread`, `KeDelayExecutionThread` | As mesmas |
| Importações XAM | `XamLoaderTerminateTitle` | `XamLoaderTerminateTitle` **+ `XNotifyQueueUI`** |

Os PEs têm mesmas regiões de seções e tamanhos brutos:
`.text` RVA 0x10000 (23552 bytes raw), `.rdata`
RVA 0x20000 (2048), `.data` RVA 0x30000 (512),
`.rodata` RVA 0x50000 (41472), mas **o código dentro
das seções mudou**, inclusive a lógica de contagem de frames.
Esses dados foram lidos dos dois PEs dos artifacts por parser
de PE e arquivo ZIP com `struct` e `zipfile`, não inferidos
de logs de compilação. Os dois contêm a ROM de 40976 bytes
com SHA256
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.

## Suspeita principal, mas NÃO prova de causa

`src/platform/canonical_xex/native_crtless_status.c`
faz `notify(text_started)` **como primeira operação de
`smb360_entry`**, antes de validar ROM e iniciar a CPU.
`notify` chama diretamente `XNotifyQueueUI(0x22, 0xff,
1ull, text, NULL)`; S017 02 **não fazia essa chamada**.
OpenXeChain `xecorelib/xam/xam.def` define
`XNotifyQueueUI @656`; existem exemplos de homebrew
que resolvem ordinal 656 dinamicamente. O ordinal correto
**não é prova de ABI correta, import loader funcional
ou segurança da API nesse título**.

Logo, a chamada/importação de notificação XAM é uma hipótese
**prioritária** para o crash S018, coerente com o Fatal Crash
do S015, mas não foi isolada fisicamente e não se pode descartar
falha anterior ao entrypoint, ABI/stack, import table,
semântica de retorno ao Aurora, ou outras mudanças de código.
**Não atribuir causa certa** ao XAM com base só na associação.

## Decisão S019 e próxima engenharia

1. **Não entregar novo teste em hardware nesta etapa.** Há um
   Fatal Crash novo e um freeze físico anteriores; novo XEX
   pode provocar o mesmo problema.
2. Evitar `XNotifyQueueUI` no caminho crítico de inicialização
   de próximo protótipo até conseguir diagnóstico independente.
   Evitar também raw framebuffer/MMIO, `printf`/Newlib CRT
   e loops infinitos de fallback de terminadores.
3. Revisar `XamLoaderTerminateTitle`, `ExTerminateThread`,
   protótipos ABI PowerPC, biblioteca de importação XAM, versão
   de sistema/endian e processamento de exceções antes de
   outro build físico. Não depender de identificação do patcher.
4. Fazer testes offline/simulados e referência do núcleo, sem
   publicar "CPU PASS" a partir de um retorno à Aurora.
5. Qualquer próxima proposta de XEX deve ter objetivo técnico
   que difira materialmente dos XEX que travaram, inspeção
   de binário, CI verde e critérios verificáveis de observação,
   com teste físico opt-in. Sem isso, não insistir em retestes.
6. ROM/PRG/CHR do proprietário preservados, sem jogos de terceiros.
   Ainda não existe Mario jogável com vídeo/áudio no Xbox, nem
   port estático 6502→PPC — o núcleo é interpretado C99 no PPC.

**Status:** S018 = `HARDWARE_TESTED_FAIL_FATAL_CRASH`.
S017 01 = `HARDWARE_TESTED_FAIL_BLACK_SCREEN_FREEZE`.
S017 02 = `HARDWARE_TESTED_AURORA_RETURN_INCONCLUSIVE`.
