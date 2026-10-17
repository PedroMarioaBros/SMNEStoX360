# S018 — teste de progresso observável, após o retorno físico da S017

> **AVISO NOVO — 17/10/2026: S018 DEU FATAL CRASH DETECTED NO
> XBOX 360 REAL. NÃO EXECUTAR ESTE ARQUIVO NOVAMENTE.**
> O restante deste documento descreve apenas o roteiro histórico
> do teste que falhou. O próprio proprietário confirmou a falha;
> não há outro reteste autorizado ou recomendável.
> A investigação prossegue sem novo executável no
> [dossiê S019](evidence/S019/README.md) e
> [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).


**O usuário testou realmente as duas variantes S017 em 13/10/2026:**

- **01:** tela preta, congelamento completo do console; não apareceu
  Fatal Crash nem havia como voltar ao menu/Aurora. **FALHA.**
- **02:** tela preta, sem erro visível, com reinício/retorno
  **automático ao Aurora**. Isso é fato observado, **NÃO demonstra
  que a CPU NES concluiu os 120 quadros**. O código S017 usava
  espera curta após erro e espera de cinco segundos após sucesso,
  mas o usuário não relatou duração; o retorno também poderia
  ocorrer por encerramento abrupto do aplicativo.

Não volte a executar nenhuma das versões anteriores S015 ou S017.
A S015 causou Fatal Crash imediato; a S017 01 travou. O repositório
registra essas observações nas issues
[#12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12)
e [#15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15).

## O que mudou na S018

Compila um **XEX nativo PowerPC Xbox 360**, não um executável de PC,
com o mesmo `smb360_entry` **sem Newlib CRT**, embutindo a ROM
exata `SMB_v026.nes` de 40976 bytes, sem substituir PRG/CHR.
CPU 6502 continua interpretada em C compilado para PPC; a
tradução estática ainda **não** existe.

O novo módulo `native_crtless_status.c` distingue cinco estados
por avisos XAM (serviço `XNotifyQueueUI`/ordinal 656):

| Notificação | O que significa se aparecer |
|---|---|
| `SMB360 S018 CPU START` | Entrou na rotina do título e tentou a API XAM |
| `SMB360 S018 ROM INVALID` | ROM/tamanho/header ou marcador falhou |
| `SMB360 S018 CPU STOP` | A máquina virtual encontrou uma parada |
| `SMB360 S018 FRAME TIMEOUT` | Watchdog excedido ao avançar um quadro |
| `SMB360 S018 120 FRAMES PASS` | O próprio programa observou 120 incrementos do contador de frames da PPU |

Esses avisos dependem da interface XAM e **podem não aparecer no
Aurora mesmo quando o programa chegou àquela etapa**. A ausência
de aviso não comprova nada. O estágio 02 anterior conseguiu
voltar ao Aurora, mas a causa do encerramento não foi estabelecida.
O primeiro teste S015, que usava `printf` antes das notificações,
sofreu Fatal Crash, portanto uma nova notificação no runtime
CRT-free é uma **hipótese experimental, não garantia de segurança**.

A S018 aguarda ~2 segundos após pedir `START` e ~5 segundos
após emitir o resultado final. A qualquer momento um novo
Fatal Crash/travamento é possível. O programa não renderiza
nenhum pixel do Mario nem produz áudio; tela preta continua
esperada para uma execução normal.

## Teste físico opcional, somente com artefato S018 e CI aprovada

1. Primeiro confirme que seu console voltou ao estado normal
   no Aurora e que os demais jogos/homebrews funcionam.
2. Descompacte apenas o artifact `smb360-S018-STATUS-NATIVE-XEX`
   da pipeline `S018 Xbox canonical status diagnostic`, obtido
   do commit aprovado **da main**.
3. Copie o arquivo **novo** `default.xex` para **uma pasta
   isolada e distinta da S015 e da S017**. **Nunca** substitua
   `BadUpdatePayload/default.xex`, arquivos do Aurora, plugins,
   `launch.ini`, NAND ou o exploit ABadAvatar.
4. Executá-lo é opcional, sabendo que ele pode novamente travar.
   Anote **qualquer** mensagem visível e se voltou ao Aurora
   normalmente ou congelou. Se travar, interrompa.
5. **Não repita o mesmo binário** após um travamento e não rode
   em loop. Não faça flash, formatação ou mudanças persistentes.

O resultado positivo mais forte seria observar o aviso
`120 FRAMES PASS` e retorno normal ao Aurora, ainda sem
validar jogo com vídeo/áudio, input, fidelidade de hardware
ou remoção do interpretador 6502.

## Testes estáticos e limites

O build confere PowerPC PE `0x01f2`, subsystem Xbox, ponteiro
de entrada coincidente entre PE e XEX2, ROM íntegra exatamente
uma vez no PE, funções importadas **exatamente**:
`KeDelayExecutionThread`, `ExTerminateThread`,
`XamLoaderTerminateTitle`, `XNotifyQueueUI`.
Sem Newlib, filesystem, rede ou MMIO.

**Compilação correta NÃO prova que as notificações aparecem
nem que o console consegue executar o arquivo.**
