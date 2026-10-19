# Evidência POC-002 — leitura de registrador e desvio condicional (18/10/2026)

## Objetivo

Ampliar a prova estática da ROM canônica para incluir uma leitura absoluta do
barramento e o desvio relativo BPL presente no polling de VBlank no reset.

## Trecho ROM

Endereços CPU e bytes do PRG:
- $8000: 78 — SEI
- $8001: D8 — CLD
- $8002: A9 10 — LDA #$10
- $8004: 8D 00 20 — STA $2000
- $8007: A2 FF — LDX #$FF
- $8009: 9A — TXS
- $800A: AD 02 20 — LDA $2002
- $800D: 10 FB — BPL $800A

A instrução BPL usa deslocamento relativo com sinal: endereço seguinte $800F,
mais -5 bytes, resulta no destino $800A. O bloco precisa conter o destino para
que a tradução estática seja autocontida.

## Alterações nesta etapa

- A tabela agora inclui LDA absoluto com callback de leitura e BPL relativo.
- O gerador cria labels C e um `goto` para o destino do branch; não usa um
  interpretador 6502 em runtime.
- Branches cujo destino não está no trecho solicitado são rejeitados
  explicitamente, em vez de gerar C incorreto.
- Self-test traduz oito instruções de $8000 até $800E, compila o C com flags
  estritas, testa o polling por duas leituras simuladas ($00 depois $80), testa
  callback de escrita e testa rejeição do branch para fora do bloco.

## Critérios e limites

Esta POC só demonstra o bloco de reset/polling, não o jogo inteiro. O callback
simulado não implementa o PPU real; temporização/ciclos e efeitos colaterais de
registradores ainda não são equivalentes ao hardware NES. Ainda não há cobertura
geral de branches, saltos, chamadas/retornos, interrupções ou análise de código
alcançável. Não constitui build PowerPC/XEX nem validação Xbox.

## Validação

A execução CI desta branch deve ser consultada antes de declarar o código
aprovado. O relatório final desta etapa deve registrar o resultado do run exato
da PR e não inferir sucesso apenas da existência do workflow.
