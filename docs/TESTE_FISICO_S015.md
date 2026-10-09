# S015 — teste físico de inicialização no Xbox 360 desbloqueado

**ATENÇÃO:** `default.xex` deste teste **não é um jogo jogável**.
Serve somente para verificar se o Xbox 360 carrega o executável
canônico, reconhece as importações da XAM e executa o runtime 6502
com a ROM original `SMB_v026.nes` durante 120 quadros.
Não há vídeo do Mario nem áudio neste teste.

O código novo **não escreve no framebuffer diretamente**.
Ele pede notificações ao dashboard usando o serviço XAM
`XNotifyQueueUI` (ordinal 656); esse recurso pode ser bloqueado
ou não aparecer em alguns ambientes. O CI garante **compilação**,
não que o console mostre a notificação. O linker ainda emite
`/align specified without /driver; image may not run`.

## Antes de iniciar

1. Use somente o Xbox 360 **já desbloqueado**, com gerenciador de
   arquivos ou Aurora/Freestyle instalado. **Não altere NAND,
   firmware, exploit, dashboard nem faça flash.**
2. No [GitHub Actions — Canonical NES Xbox 360 diagnostic XEX](https://github.com/PedroMarioaBros/SMNEStoX360/actions/workflows/canonical-xex.yml),
   escolha a execução aprovada **mais recente da branch main**.
   No rodapé, em **Artifacts**, baixe
   `smb360-CANONICAL-DIAGNOSTIC-xex`.
   Confira que o artifact é do S015, não o `nathsou` legado:
   dentro dele `BUILD_MANIFEST.txt` traz
   `build_kind=CANONICAL_DIAGNOSTIC_NOT_PLAYABLE`;
   `project_commit` é o commit de main usado no build.
3. Descompacte o ZIP. Localize
   `build-canonical-xex/default.xex` (ou `default.xex`
   no diretório extraído, dependendo do programa ZIP).
   Não troque o `default.xex` de jogos já instalados.
4. Copie o `default.xex` para **uma pasta nova e isolada**
   no HDD ou dispositivo FAT32, por exemplo
   `Hdd1:\Apps\SMB360_S015\default.xex`.
   Se possível, guarde junto `BUILD_MANIFEST.txt` e
   `SHA256SUMS.txt` para identificar a versão.
   Esse XEX tem a ROM completa embutida, não requer
   `.nes` externo.

## Como executar e o que observar

1. Abra `default.xex` da pasta isolada pelo gerenciador do Xbox.
2. **Se a XAM conseguir mostrar notificações**, aparece um pop-up
   curto com o texto `SMB360 S015 START - 120 frames`.
3. O runtime tenta processar 120 quadros da ROM original com
   o mesmo núcleo C99 testado no PC; lê controles Xbox 1 e 2,
   convertendo os oito botões para entradas NES. Não haverá
   imagem do Mario porque não existe backend gráfico nesse teste.
4. Em caso de conclusão, o programa pede a notificação
   `SMB360 S015 120 FRAMES PASS`, permanece ativo por
   aproximadamente 4 segundos e retorna ao dashboard.
   O tempo total pode ser superior a 6 segundos dependendo
   do console. **O pop-up pode não aparecer mesmo que o
   executável tenha iniciado**, caso o dashboard suprima avisos.
5. Em caso de erro interno pode pedir uma notificação
   `ROM INVALID`, `CPU STOP` ou `FRAME TIMEOUT`.
   Anote exatamente a mensagem. Se aparecer `Fatal Crash`,
   tela preta permanente, freeze ou reinício, interrompa o
   teste e reinicie normalmente o console. Não insista sem
   analisarmos os resultados. Não deixe o programa executando
   em loop.

## O que me enviar de volta

Responda com as observações, sem precisar entender o código:

- **A:** chegou a abrir, ficou tela preta, travou, reiniciou,
  mostrou `Fatal Crash`, ou retornou ao dashboard?
- **B:** apareceu `SMB360 S015 START`, `120 FRAMES PASS`,
  alguma mensagem de erro, ou nenhuma notificação?
- **C:** aproximadamente quanto tempo levou e se o dashboard
  respondeu normalmente depois?
- **D:** qual seu gerenciador (Aurora, Freestyle, outro) e qual
  tipo de desbloqueio (RGH, JTAG ou outro, se souber)?
  Se puder, uma foto da mensagem/erro ajuda.

## O que a evidência permite concluir

- `START` visto: indica que pelo menos o entrypoint e a XAM
  chegaram até a solicitação de notificação; **não** valida o
  runtime inteiro.
- `PASS` visto: evidência concreta de passagem pelos 120
  quadros (segundo o próprio programa), **não** garante vídeo,
  áudio, fidelidade NES nem jogo jogável.
- Sem notificação: **resultado inconclusivo** — pode ser filtro
  da XAM, retorno rápido, erro de importação ou falha de boot.
- Nenhum desses resultados converte a CPU 6502 em
  código nativo PPC: o runtime ainda interpreta instruções.

Se o teste não for executado, o estado permanece
`hardware_validation=NOT_TESTED`. CI verde não equivale a
prova de funcionamento no Xbox.
