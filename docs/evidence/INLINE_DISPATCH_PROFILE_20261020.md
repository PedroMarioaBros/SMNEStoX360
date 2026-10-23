# Evidência de despacho inline — SMB_v026 (20/10/2026)

## Execução e ferramenta

CI: [run 39432801245](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39432801245), **success**. Commit do evento de execução: `ad6ebe07e481d89fceeb15c2c338e149b134402f`; a branch de trabalho também contém a correção do mapa de cenários diferenciais.

A entrada ROM foi verificada em tempo de CI:
- SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
- Mapper 0 / PRG de 32.768 bytes.

O profiler temporário intercepta o ponto de entrada do dispatcher inline em `$8E04`, tanto no fetch recompilado quanto no passo interpretado, e registra o endereço de retorno empilhado pelo JSR, o seletor em A, o endereço da palavra inline e o destino decodificado. A ferramenta `tools/verify_nesrecomp_dispatch_profile.py` valida cada registro contra os bytes do PRG canônico, em vez de confiar somente no log do profiler.

## Resultado estático + perfil

- Varredura do PRG encontrou **18 candidatos brutos** aos bytes `JSR $8E04`.
- Os cenários perfilados alcançaram **12 callsites**.
- Foram validados **57 pares únicos (callsite, seletor)** e **47 destinos ROM únicos**. O verificador validou os bytes de cada callsite e todas as palavras de tabela selecionadas: **PASS**.
- Os 57 registros foram observados pelo caminho nativo no build de referência perfilado; isso não significa que todos os callsites/seletor possíveis no ROM foram explorados.
- **17 endereços que não existiam na lista da configuração base** foram acrescentados apenas à cópia temporária do TOML: `$8245`, `$860D`, `$9061`, `$9071`, `$91CD`, `$AEDC`, `$AEEA`, `$B069`, `$B0E9`, `$B245`, `$B269`, `$B36D`, `$B376`, `$BC85`, `$C2F1`, `$C328`, `$C9D8`. Nem a configuração do repositório nem a ROM foram modificadas.

## Alvos observados

A tabela abaixo é o conjunto de entradas dinâmicas efetivamente observadas nos quatro roteiros perfilados, e não uma lista exaustiva provada estaticamente para todo o jogo.

| JSR/callsite | Seletor A | Palavra inline | Destino lido do PRG |
|---:|---:|---:|---:|
| $8215 | 0 | $8218 | $8231 |
| $8215 | 1 | $821A | $AEDC |
| $8234 | 0 | $8237 | $8FCF |
| $8234 | 1 | $8239 | $8567 |
| $8234 | 2 | $823B | $9061 |
| $8234 | 3 | $823D | $8245 |
| $856A | 0 | $856D | $858B |
| $856A | 1 | $856F | $859B |
| $856A | 2 | $8571 | $8652 |
| $856A | 3 | $8573 | $865A |
| $856A | 4 | $8575 | $92AA |
| $856A | 6 | $8579 | $86A8 |
| $856A | 7 | $857B | $889D |
| $856A | 8 | $857D | $86E6 |
| $856A | 9 | $857F | $85BF |
| $856A | 10 | $8581 | $85E3 |
| $856A | 11 | $8583 | $8643 |
| $856A | 12 | $8585 | $86FF |
| $856A | 13 | $8587 | $8732 |
| $856A | 14 | $8589 | $8749 |
| $92C8 | 0 | $92CB | $92DB |
| $92C8 | 1 | $92CD | $88AE |
| $92C8 | 2 | $92CF | $88AE |
| $92C8 | 3 | $92D1 | $93FC |
| $92C8 | 4 | $92D3 | $92DB |
| $92C8 | 5 | $92D5 | $88AE |
| $92C8 | 6 | $92D7 | $88AE |
| $92C8 | 7 | $92D9 | $93FC |
| $9664 | 0 | $9667 | $98E5 |
| $9664 | 2 | $966B | $9A2E |
| $9664 | 7 | $9675 | $98E5 |
| $9664 | 22 | $9693 | $9B0E |
| $9664 | 23 | $9695 | $9B0E |
| $AEDF | 0 | $AEE2 | $8FE4 |
| $AEDF | 1 | $AEE4 | $8567 |
| $AEDF | 2 | $AEE6 | $9071 |
| $AEDF | 3 | $AEE8 | $AEEA |
| $B04C | 0 | $B04F | $9131 |
| $B04C | 6 | $B05B | $91CD |
| $B04C | 7 | $B05D | $B069 |
| $B04C | 8 | $B05F | $B0E9 |
| $B04C | 10 | $B063 | $B245 |
| $B04C | 11 | $B065 | $B269 |
| $B34E | 0 | $B351 | $B35A |
| $B34E | 1 | $B353 | $B376 |
| $B34E | 2 | $B355 | $B36D |
| $BDBD | 0 | $BDC0 | $BDD2 |
| $C27F | 2 | $C286 | $C30E |
| $C27F | 4 | $C28A | $860D |
| $C27F | 5 | $C28C | $C328 |
| $C27F | 6 | $C28E | $C2F1 |
| $C88F | 0 | $C892 | $C8E0 |
| $C88F | 26 | $C8C6 | $BC85 |
| $C907 | 2 | $C90E | $CA77 |
| $C907 | 5 | $C914 | $C9D8 |
| $C907 | 6 | $C916 | $CA77 |
| $C907 | 18 | $C92E | $CA77 |

## Recompilação nativa sem interpretador e paridade

Depois de aplicar os 17 novos seeds somente ao TOML temporário:
- O runtime de teste não incluiu `cpu6502_interp.c` no `build.ninja`, e `cpu_interp_step` não apareceu nos símbolos do executável.
- Quatro cenários passaram sem `NATIVE_COVERAGE_MISS`, todos com 100,0% dos ciclos em código nativo.
- Os hashes por quadro coincidiram entre o build sem interpretador e o runner de referência em **6.600 quadros** no total.
- A saída PCM foi comparada por bytes; WAV metadata e PCM coincidiram em cada cenário:

| Cenário | Quadros | Amostras PCM | SHA-256 do PCM |
|---|---:|---:|---|
| Idle/título | 600 | 479.146 | `2cef570772cd9740096484f400d2119e9f1e04e6ff84f0bedc6b85f1377e108c` |
| Rota curta | 600 | 479.147 | `74abe61c0278cb3368c448cbaace9138cdbb8e595b05324459ee0a500784d342` |
| Movimento/pulo | 1.800 | 1.437.568 | `b03e2bd423f2456d5b5872d6a6d3263ae4ef16f6f7845e0aa5792158c4629ee9` |
| Rota longa | 3.600 | 2.875.201 | `bf45771e7c88f372c0e19e73fb5468c9c0f92eda9d1b9d258881ce3ff216e67a` |

## Limites

Esse resultado confirma fluxo nativo e equivalência observável nos caminhos perfilados. **Não prova cobertura completa do jogo**: só 12 dos 18 candidatos brutos a callsite apareceram nos cenários usados, e outros seletores/estados podem existir. Os 17 seeds são baseados em perfil e permanecem temporários; não são um algoritmo geral de descoberta de fluxo nem configuração final aprovada. Não houve build Xenon/PowerPC, XEX ou teste físico. O C gerado permaneceu em diretório temporário e não foi publicado.
