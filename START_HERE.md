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

## Estado resumido

Runtime canônico executa no host e já produziu quadros; fidelidade gráfica
continua pendente. S005 corrigiu o timing NTSC de frames ímpares e demonstrou
que o HUD já estava presente. S006 comparou pixels/RAM com referência, corrigiu
o controle 2. S007 entregou frontend host SDL2 com assets embutidos e smoke
automatizado; teste manual pendente. S008 iniciou a APU 2A03, com primeiro
canal pulse. S009 implementou pulse 2, com CI e comparação 600+600 pixels/RAM
aprovados contra a referência. Ainda sem áudio audível ou PCM; próxima etapa
técnica é triangle, depois noise, DMC e mixer. Nenhum novo XEX da arquitetura
canônica foi validado no Xbox.
Consulte sempre o checkpoint e NEXT_STEPS.md: este resumo não substitui os dois.
