# SMB_v026 — fonte canônica do port Xbox 360

A partir desta etapa, o projeto trata exclusivamente a ROM fornecida pelo proprietário como fonte de verdade do jogo.

## Identidade verificada

- Arquivo local: SMB_v026.nes
- Tamanho: 40.976 bytes
- SHA-256 ROM: 57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047
- Formato: iNES
- Mapper: 0 / NROM
- PRG: 32 KiB
- CHR: 8 KiB
- Mirroring: vertical
- Trainer: ausente
- SHA-256 PRG: 9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4
- SHA-256 CHR: 5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0

## Vetores 6502 lidos diretamente do PRG

- RESET: $8000
- NMI: $8082
- IRQ/BRK: $FFF0

## Regra de arquitetura

O código recompilado de terceiros não é mais a fonte canônica do jogo. A conversão deve partir do PRG/CHR acima e ser validada contra esta ROM.

Pipeline pretendido:

SMB_v026.nes -> análise PRG 6502 + dados CHR -> núcleo reproduzível -> validação host -> backend Xbox 360/OpenXeChain -> default.xex

A validação host é uma bancada de equivalência e não substitui nem altera a origem do port.

## Critério de fidelidade

Nenhuma versão será descrita como port integral da SMB_v026 enquanto comportamento derivado do PRG, dados e CHR desta ROM não estiver comprovadamente preservado.
