# COMECE AQUI — continuidade do SMNEStoX360

Esta é a entrada operacional para qualquer novo chat, Work ou agente.
O GitHub é a fonte persistente do estado do projeto; não depender da memória
de uma conversa anterior. A ROM e os assets estão em assets/canonical/;
consulte [REPRODUCIBILITY.md](docs/REPRODUCIBILITY.md) para um clone autossuficiente
em arquivos do projeto.

## Ordem obrigatória de leitura

1. [AGENTS.md](AGENTS.md): regras permanentes.
2. [Protocolo de continuidade](docs/CONTINUITY_PROTOCOL.md): como iniciar,
   registrar avanços e encerrar toda sessão.
3. [Checkpoint atual](docs/WORK_CHECKPOINT.md): implementação, evidências e limites.
4. [Próxima ação](docs/NEXT_STEPS.md): tarefa ativa e próximos comandos.
5. [Histórico de sessões](docs/SESSION_LOG.md): o que já foi feito e tentado.
6. [ROM canônica](docs/SMB_V026_CANONICAL_ROM.md): identidade e arquitetura.

Leia o HEAD remoto antes de editar. As instruções de continuidade devem ser
mantidas e cumpridas por cada sucessor, que também deve deixar o estado pronto
para o próximo.

## Comando de retomada para colar em outro chat

> Continue o projeto PedroMarioaBros/SMNEStoX360. Leia START_HERE.md no HEAD
> atual do GitHub e siga integralmente AGENTS.md e o protocolo de continuidade.
> Leia o checkpoint, o histórico e NEXT_STEPS.md antes de trabalhar.
> Execute a próxima tarefa pendente, preservando a ROM canônica.
> Registre cada avanço e tentativa relevante, atualize o próximo comando,
> faça commits e confirme a publicação remota antes da resposta final.
> Não reinicie etapas concluídas sem uma razão técnica registrada.
> Use a ROM versionada em assets/canonical/ e confira seus hashes.
> Se faltar acesso ao GitHub, registre o bloqueio real, sem inventar progresso.

> **ATUALIZAÇÃO FÍSICA CRÍTICA — 17/10/2026:** o proprietário
> confirmou `S018 resultou em Fatal Crash Detected` no Xbox 360.
> A S018 está **TESTADA E FALHOU**, não "aguardando teste".
> **NÃO EXECUTAR NOVAMENTE S018, S015 ou S017.**
> S017 01 congelou em tela preta; S017 02 retornou ao
> Aurora sem confirmar conclusão da CPU. A comparação exata
> de binários localizou a importação/chamada adicional de
> `XNotifyQueueUI` no S018, hipótese prioritária mas
> **não comprovada como causa do crash**. Ver
> [S019](docs/evidence/S019/README.md), [issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15)
> e [NEXT_STEPS](docs/NEXT_STEPS.md). Nenhum novo XEX físico
> está liberado para testar. ROM canônica intacta.

## Estado resumido

**ATUALIZAÇÃO 13/10/2026 — RESULTADO FÍSICO S017:** o proprietário
testou ambas as etapas: **01 travou em tela preta sem Fatal Crash e
sem retorno ao menu; 02 ficou em tela preta e retornou/relançou o
Aurora automaticamente, sem mensagem de erro**. NÃO declarar 01 ou 02
como prova de CPU/frame PASS. Não pedir reexecução. A S018
introduziu mensagens XAM de progresso/error/PASS em novo XEX
nativo CRT-free; veja [evidência S018](docs/evidence/S018/README.md),
[issue #15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15)
e [próximas ações](docs/NEXT_STEPS.md). S018 ainda sem hardware testado.
Acesso ABadAvatar está confirmado; não bloquear a engenharia por
identificação do segundo patcher.

**ATUALIZAÇÃO HISTÓRICA 12/10/2026 — S017:** o S015 testado pelo proprietário
causou Fatal Crash real. Pela PR #14, integrada na main como
`40825ff3530d426b05d3e23b4ab02b8d82f12ebe`, foram gerados
**dois executáveis XEX2 PowerPC Xbox OS sem startup Newlib CRT**:
01 entrada nativa mínima e 02 com ROM `SMB_v026.nes` e core
canônico de CPU/PPU. O CI S017 main
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754775
PASSOU, mas S017 **não foi testado no hardware** e não é
Mario jogável. O usuário já recebeu os dois executáveis.
A Etapa 02 só deve ser executada após boot real bem-sucedido
da Etapa 01, e qualquer Fatal Crash exige interromper testes.
Ver [guia S017](docs/TESTE_FISICO_S017.md) e
[checkpoint S017](docs/WORK_CHECKPOINT.md). Não insistir na
identificação do patcher; o proprietário pediu que a engenharia
avance independente desse dado.


Runtime canônico executa no host e já produziu quadros; fidelidade gráfica
continua pendente. S005 corrigiu o timing NTSC de frames ímpares e demonstrou
que o HUD já estava presente. S006 comparou pixels/RAM com referência, corrigiu
o controle 2. S007 entregou frontend host SDL2 com assets embutidos e smoke
automatizado; teste manual pendente. S008/S009 implementaram pulse1 e
pulse2 da APU; S010 implementou triangle. S011 acrescentou noise; S012 o DMC e DMA/IRQ inicial.
S013 gerou a primeira captura PCM/WAV da ROM canônica:
mixer não linear, saída SDL opcional, WAV 48 kHz/16 bits
de 479.146 samples, 13 suítes host aprovadas,
600+600 pixels/RAM idênticos à referência. Ainda não há
teste com dispositivo físico nem waveform NES independente.
**S014 compilou pela primeira vez o runtime canônico e a ROM
SMB_v026.nes para XEX2 PowerPC próprio**, sem nathsou:
novo artifact de diagnóstico headless, ver evidence/S014.
S015 ampliou o diagnóstico Xbox canônico com notificações XAM
de início/conclusão, leitura de dois gamepads Xbox e validação
de que a ROM inteira está realmente no PE PowerPC. O teste
sintético de controles amplia para 14 suítes host.
**S015 falhou em hardware real: Fatal Crash imediato no Xbox 360 em
12/10/2026; o proprietário precisou reiniciar. NÃO repetir o S015.**
A falha está na [issue #12](https://github.com/PedroMarioaBros/SMNEStoX360/issues/12).
S016 isola boot do kernel de execução da ROM, sem XAM, com testes
separados e guia [S016](docs/TESTE_FISICO_S016.md).
Novo teste físico depende de build S016 comprovado e orientação explícita.
Ainda sem imagem nem áudio do Mario no Xbox e sem
tradução estática PowerPC; o PRG 6502 é interpretado em C.
Consulte sempre o checkpoint e NEXT_STEPS.md: este resumo não substitui os dois.
