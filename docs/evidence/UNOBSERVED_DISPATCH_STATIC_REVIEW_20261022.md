# Revisão estática dos seis candidatos não observados — SMB_v026 (22/10/2026)

## Escopo e integridade

Esta revisão usa somente a ROM canônica `assets/canonical/SMB_v026.nes` da branch `research/nesrecomp-evaluation-20261018`, verificada com tamanho de 40.976 bytes e SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`. O PRG NROM-256 mapeia linearmente CPU `$8000-$FFFF`.

O alvo desta tarefa são os seis bytes candidatos a `JSR $8E04` que não apareceram nos quatro roteiros perfilados. A inspeção confirma que, em cada endereço listado, os três bytes são `20 04 8E`; também permite inferir um provável tamanho da tabela inline pela sequência de palavras de destino seguida por bytes que formam uma instrução plausível. Isso é **análise estática preliminar**, não prova de que cada callsite seja alcançável em runtime ou de que todos os seletores foram enumerados.

## Resultados por candidato

| Callsite | Palavras little-endian após o JSR (preview) | Hipótese de limite da tabela | Confiança / observação |
|---|---|---|---|
| `$83A3` | `$CFEC, $83B0, $83BD, $83F6, $8461` | 5 entradas; bytes em `$83B0` começam `AE 1B 07` (instrução plausível) | Alta para tabela inline; alcançabilidade ainda não provada. |
| `$921B` | `$9224, $8567, $9237` | 3 entradas; bytes em `$9224` começam `A9 00` | Alta para tabela inline; os bytes seguintes após as três palavras já formam código plausível. |
| `$9743` | `$974C, $9778, $9A69` | 3 entradas; bytes em `$974C` começam `20 BB 9B` (`JSR $9BBB`) | Alta para tabela inline; limite sustentado por uma instrução plausível na sequência. |
| `$C7A8` | `$C3A4, $C7B7, $C4A8, $C5A3, $C63D, $C69C` | 6 entradas; bytes em `$C7B7` começam `A0 05` | Alta para tabela inline; `$C7B7` também é um dos destinos e aparenta ser início de código. |
| `$C987` | `$D432, $D5D3, $D64F, $D64F, $D607, $D631, $D63D` | 7 entradas; bytes em `$C998` começam `A9 00` | Alta para tabela inline; destino repetido `$D64F` é permitido e não deve ser deduplicado na tabela. |
| `$D2E5` | `$D311, $D2F2, $D312, $D34E, $D3A2` | 5 entradas; bytes em `$D2F2` começam `A0 05` | Alta para tabela inline; `$D2F2` é destino e parece início de código. |

## Interpretação

- Os seis candidatos são **mais consistentes com dispatchers inline reais do que com coincidências em dados**, porque cada um ocorre em contexto de bytes plausivelmente executáveis, seguido por uma sequência de palavras little-endian que apontam para endereços da faixa ROM e termina em uma instrução plausível.
- As quantidades acima são **limites prováveis da tabela**, inferidos por inspeção dos bytes, não uma prova formal de todos os seletores válidos. Os previews além do limite provável passam a interpretar bytes de código como palavras e não devem ser tratados como entradas.
- Esta revisão não autoriza adicionar seeds indiscriminadamente. Os seeds existentes continuam temporários e a configuração permanente não foi modificada.
- A próxima ação é cruzar os seis callsites com o grafo de controle estático e com os caminhos de entrada/estado do jogo; então desenhar cenários determinísticos específicos. Só os cenários que realmente alcançarem os callsites podem ser usados para afirmar cobertura dinâmica.
- Para cada novo cenário, manter os quatro roteiros atuais como regressão e exigir: ROM/hash intactos; build sem unidade/símbolo do interpretador; zero `NATIVE_COVERAGE_MISS`; hashes de estado/trace iguais por quadro; PCM/WAV idêntico byte a byte. Se um callsite continuar inalcançado, registrar a razão e a incerteza em vez de marcar como coberto.

## Limites desta entrega

Não foi executada nesta revisão uma nova simulação de gameplay nem uma nova CI de runtime; portanto não há novos resultados dinâmicos para declarar. Nenhuma ROM foi modificada. Não houve build Xenon/PowerPC, XEX, integração Xbox ou teste físico. Esta é uma atualização de evidência estática e priorização do próximo experimento.
