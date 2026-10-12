# S017 — Executável Xbox 360 PowerPC com ponto de entrada sem CRT

## Objetivo técnico

O executável S015 entregue anteriormente **deu Fatal Crash imediatamente
no Xbox 360 real**, obrigando a reinicialização do console. Não executar
novamente a S015. Registro: https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .

Nos diagnósticos S016, mesmo o programa mínimo importava
`DbgPrint`, `CreateFileA`, `ReadFile`, `WriteFile`,
`XamLoaderLaunchTitle` e outras funções, trazidas pelo
Newlib CRT, independentemente da lógica do jogo.

**S017 é uma nova compilação nativa para Xbox 360, que corrige
essa dependência estrutural:**
- função de entrada `smb360_entry` ligada diretamente ao entrypoint
  PE/XEX, **sem Newlib CRT** e sem `main`;
- somente 3 imports de funções de sistema:
  `KeDelayExecutionThread`, `XamLoaderTerminateTitle`
  e `ExTerminateThread` (fallback);
- apenas 2 módulos importados: `xboxkrnl.exe` e `xam.xex`;
- aguarda aproximadamente 5 segundos e solicita encerramento
  do título pelo serviço da XAM (não escreve em vídeo, NAND
  ou endereços físicos);
- a CPU é **PowerPC Xbox 360**, arquivo efetivo `XEX2`,
  `SUBSYSTEM:XBOX`, e entrada do PE == entrada indicada no XEX.

## Dois executáveis nativos separados

O artifact S017 inclui:
- **`default.xex` na raiz — ETAPA 01:** mínimo sem jogo,
  sem Newlib CRT, aguarda cerca de cinco segundos e encerra.
- **`02_CANONICAL_CRTLESS/default.xex` — ETAPA 02:** executa
  o core CPU/PPU C99 da ROM canônica `SMB_v026.nes` de
  40976 bytes **incorporada no próprio XEX**; processa
  até 120 quadros com watchdog e retorna. Ainda sem
  vídeo/áudio Xbox nem input de jogabilidade.

A etapa 02 depende de primeiro observar **etapa 01
retornando normalmente ao gerenciador** no Xbox. Nunca
executar 02 se 01 falhar. Se a etapa 02 indicar erro
de ROM/CPU/watchdog, aguarda ~1 segundo antes de solicitar
o retorno; se concluir, aguarda ~5 segundos. O intervalo
não é prova infalível de CPU OK porque o loader pode
encerrar por motivos externos.

Ambas as variantes possuem apenas três imports:
`KeDelayExecutionThread`, `XamLoaderTerminateTitle`
e `ExTerminateThread`, não trazem Newlib startup
ou `XNotifyQueueUI`/`XamInputGetState`.

## O que ainda NÃO é

**Não é o Super Mario jogável.** A etapa 01 não inclui ROM; a etapa 02
incorpora a ROM original e executa CPU/PPU sem imagem nem áudio no Xbox.
São dois diagnósticos separados, não um jogo pronto.

**Não foi validado em console**. Um XEX2 estruturalmente correto
pode ainda falhar em determinadas combinações de firmware,
patcher, loader ou toolchain. Não existe garantia honesta
de compatibilidade universal apenas por compilar. A ausência
de teste físico é uma limitação real, não um sucesso presumido.

## Procedimento físico opcional, sujeito a risco de novo crash

Se decidir executar o S017 após verificar que o ZIP contém
o `BUILD_MANIFEST.txt` com
`kind=S017_NATIVE_CRTLESS_XBOX_OS_BOOT_DIAGNOSTIC`
e `runtime_test=NOT_TESTED_ON_XBOX_HARDWARE`:

1. Faça uma pasta nova e isolada, por exemplo
   `Hdd1:\\Apps\\SMB360_S017_CRTLESS\\`.
2. Copie apenas `default.xex` desta etapa para lá,
   mantendo S015 e arquivos do Aurora/desbloqueio intactos.
3. Execute a partir de um gerenciador de arquivos já funcional,
   nunca substitua `BadUpdatePayload/default.xex` do desbloqueio.
4. Registre se retorna ao menu após cerca de 5 segundos,
   apresenta Fatal Crash ou não executa.
5. Se **etapa 01** retornar normalmente, somente então considerar
   `02_CANONICAL_CRTLESS/default.xex` em pasta separada e
   observar o comportamento; não usar o mesmo nome de pasta
   ou sobrescrever o primeiro arquivo.
6. Se houver qualquer Fatal Crash, interrompa. Não repetir,
   não alterar NAND, firmware, exploit, plugins ou launch.ini.

O resultado do S017 separa incompatibilidade de startup/CRT
de falhas anteriores, mas ainda não prova jogabilidade.
A etapa 02 já incorpora o núcleo e ROM originais nesse novo ponto
de entrada. Após observar **boot físico bem-sucedido**, a próxima
etapa é implementar vídeo Xbox por API segura, controles e áudio,
sem regressar ao startup antigo.

## Validação automática / integridade

O workflow específico executa
`bash scripts/build-crtless-xex.sh` e
`python3 tools/verify_crtless_xex.py` sobre os
binários compilados, verificando:
- PE32 PowerPC machine=0x01f2 e subsystem=0x0e
- XEX2, entrada PE e XEX coincidentes
- importações exatas e ausência de rotinas Newlib de arquivo/CRT
- checksums SHA-256 do PE e XEX
- nenhuma ROM na etapa 01 e a ROM canônica exata, uma vez, na 02

A ROM do usuário `assets/canonical/SMB_v026.nes` não foi
alterada e segue exclusiva para a futura integração do Mario.
