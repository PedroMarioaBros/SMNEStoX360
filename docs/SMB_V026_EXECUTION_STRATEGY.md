# Estratégia de execução da SMB_v026

## Evidência obtida diretamente da ROM

A análise conservadora iniciada nos vetores NMI/RESET/IRQ encontrou inicialmente:

- 1.288 instruções alcançáveis
- 2.825 bytes classificados como código por fluxo direto
- 8,62% dos 32 KiB de PRG
- 42 alvos diretos de JSR
- 25 alvos diretos de JMP

A varredura também encontrou caminhos que chegam a bytes não pertencentes ao conjunto oficial de opcodes 6502. Isso demonstra que uma desmontagem linear/estática simples não é suficiente para separar código e dados com segurança; saltos indiretos, tabelas e/ou caminhos calculados precisam de análise dinâmica.

## Decisão de engenharia

Não converter cegamente os 32 KiB para C.

A SMB_v026 permanece a fonte canônica. A primeira implementação de fidelidade deve executar o PRG original em uma camada 6502 determinística e implementar/interceptar apenas o hardware NES necessário (memória, PPU, APU, controles e temporização). Essa camada será validada em host antes de receber o backend Xbox 360.

Depois de existir equivalência reproduzível, rotinas quentes poderão ser recompiladas estaticamente para PowerPC sem mudar a fonte de verdade.

## Critérios antes de novo build jogável Xbox

1. Hash integral da ROM validado.
2. PRG e CHR extraídos e validados.
3. CPU 6502 passa testes unitários de instruções/flags/endereço.
4. Mapa de memória NROM implementado.
5. Boot da SMB_v026 reproduzível em host.
6. Quadros/estado do jogo comparáveis de forma determinística.
7. Só então integrar vídeo/controle/áudio Xbox por etapas.
