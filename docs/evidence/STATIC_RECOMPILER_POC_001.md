# Evidência POC-001 — tradução estática 6502 → C (18/10/2026)

## Objetivo desta etapa

Comprovar o primeiro caminho mínimo de tradução estática derivado da ROM
canônica: decodificar um trecho conhecido do PRG, gerar C com operações
diretas e compilar/executar esse C sem despachante de opcodes em runtime.
Esta evidência não afirma descompilação integral, equivalência do jogo ou
funcionamento no Xbox 360.

## Fonte de verdade

- Arquivo: `assets/canonical/SMB_v026.nes`
- Tamanho: 40.976 bytes
- SHA-256 esperado e verificado pelo tradutor:
  `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
- PRG iNES offset 16, 32.768 bytes, CPU address range $8000-$FFFF.
- ROM/PRG/CHR não foram alterados.

## Implementação

- `tools/static6502_poc.py`: lê e verifica hash/layout da ROM; traduz um
  trecho delimitado a partir de um endereço PRG; gera uma função C com uma
  instrução 6502 convertida em statements diretos por opcode; rejeita opcode
  ainda não suportado em vez de substituir por NOP.
- `tests/run_host_tests.sh`: inclui `python3 tools/static6502_poc.py --self-test`.
- Trecho real escolhido no reset: $8000-$8009:
  `78 D8 A9 10 8D 00 20 A2 FF 9A`
  = SEI, CLD, LDA #$10, STA $2000, LDX #$FF, TXS.
- O endereço seguinte é $800A. A instrução ali é LDA $2002 (opcode $AD), ainda
  não suportada pelo tradutor e intencionalmente usada para verificar falha
  explícita/fechada.

## Teste executado no ambiente desta sessão

Comando equivalente contra a ROM anexada, cujo hash foi conferido:
`python3 tools/static6502_poc.py --rom /mnt/data/SMB_v026.nes --self-test`

Resultado:
- PASS: SHA-256 canônico validado.
- PASS: seis instruções extraídas diretamente do PRG foram traduzidas.
- PASS: C gerado compilou com `-std=c99 -Wall -Wextra -Werror -pedantic`.
- PASS: asserções verificaram A=$10, X=$FF, S=$FF, flags de SEI/CLD/LDX e
  uma escrita de barramento em $2000 com valor $10.
- PASS: tentativa de traduzir opcode $AD em $800A é recusada com erro explícito.
- Limite: somente trecho reto de seis instruções; sem teste diferencial com o
  runtime completo, sem PPU/APU/mapeamento de memória NES implementados aqui,
  sem build PowerPC/XEX e sem teste físico.

## Próxima etapa

Expandir a tabela de tradução com instruções efetivamente alcançadas no PRG,
incluindo leitura/escrita por callbacks de barramento, branches e blocos de
controle. Construir testes unitários por opcode/flags/endereçamento e comparar
estado/traces contra o core host de referência. Antes de chamar o tradutor de
completo, medir cobertura de código alcançável e distinguir código de dados.
Nenhum XEX/teste físico foi produzido ou solicitado nesta etapa.
