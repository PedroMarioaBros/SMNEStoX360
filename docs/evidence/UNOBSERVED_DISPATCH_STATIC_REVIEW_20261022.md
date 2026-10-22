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

- Os seis candidatos têm estrutura compatível com dispatchers inline (palavras little-endian que apontam para a faixa ROM e um limite plausível), mas **isso não basta para provar que são código ativo**. A análise do contexto trouxe um alerta importante: cinco candidatos (`$83A3`, `$921B`, `$9743`, `$C7A8`, `$C987`) aparecem após um `JMP` incondicional ou `RTS` no fluxo linear de bytes. Podem ser blocos alternativos alcançáveis por referências não lineares, mas também podem ser bytes não executados; é necessário rastrear referências de entrada. O sexto (`$D2E5`) vem após um branch condicional e sua alcançabilidade depende do estado das flags.
- As quantidades acima são **limites prováveis da tabela**, inferidos por inspeção dos bytes, não uma prova formal de todos os seletores válidos. Os previews além do limite provável passam a interpretar bytes de código como palavras e não devem ser tratados como entradas.
- Esta revisão não autoriza adicionar seeds indiscriminadamente. Os seeds existentes continuam temporários e a configuração permanente não foi modificada.
- A próxima ação é cruzar os seis callsites com o grafo de controle estático e com os caminhos de entrada/estado do jogo; então desenhar cenários determinísticos específicos. Só os cenários que realmente alcançarem os callsites podem ser usados para afirmar cobertura dinâmica.
- Para cada novo cenário, manter os quatro roteiros atuais como regressão e exigir: ROM/hash intactos; build sem unidade/símbolo do interpretador; zero `NATIVE_COVERAGE_MISS`; hashes de estado/trace iguais por quadro; PCM/WAV idêntico byte a byte. Se um callsite continuar inalcançado, registrar a razão e a incerteza em vez de marcar como coberto.

## Revisão adicional de fluxo linear

A inspeção de contexto revelou que a desassemblagem linear não é suficiente para classificar os callsites:

- `$83A3`: há um `JMP $EEE9` imediatamente antes do bloco candidato.
- `$921B`: há um `JMP $9264` antes do bloco candidato.
- `$9743`: o bloco candidato aparece após um `RTS`.
- `$C7A8`: há um `JMP $C7DB` antes do bloco candidato.
- `$C987`: há um `JMP $D67A` antes do bloco candidato.
- `$D2E5`: a execução linear depende de um branch `BCS` anterior; o branch pode saltar por cima do callsite.

Esses terminadores não provam que os bytes seguintes sejam dados: tabelas de função, destinos de branches/jumps ou dispatchers podem criar entradas não lineares. Porém, eles impedem concluir que os candidatos sejam alcançáveis apenas pela proximidade de bytes. **A próxima análise precisa localizar referências de controle que entrem nesses blocos e determinar as condições necessárias**, antes de escolher inputs ou acrescentar seeds.

## Busca heurística de referências de entrada (passagem 1)

Foi feita uma varredura byte a byte do PRG canônico para procurar padrões de referência direta: `JMP abs` (`$4C`), `JSR abs` (`$20`) cujo operando fosse um dos seis endereços, e os oito branches relativos do 6502 cujo destino calculado coincidisse com um candidato.

| Candidato | JMP/JSR direto encontrado | Branch relativo encontrado | Ocorrências brutas dos dois bytes do endereço |
|---|---|---|---|
| `$83A3` | nenhuma | nenhuma | nenhuma |
| `$921B` | nenhuma | nenhuma | `$A727` |
| `$9743` | nenhuma | nenhuma | nenhuma |
| `$C7A8` | nenhuma | nenhuma | nenhuma |
| `$C987` | nenhuma | nenhuma | `$8822`, `$D0CC` |
| `$D2E5` | nenhuma | nenhuma | nenhuma |

**Como interpretar:** esta é uma busca de padrões em bytes, não um grafo de controle 6502 validado. Ela pode produzir falsos positivos quando um byte de dados coincide com opcode e falsos negativos quando a entrada é indireta (por exemplo, tabela de ponteiros, salto indireto, retorno/stack ou código que a varredura linear não alinhou). Mesmo assim, o fato de não aparecer referência direta é um sinal útil: não devemos criar seeds nem roteiros para esses candidatos sem antes entender a origem da entrada. O próximo passo técnico é obter um mapa de blocos alcançáveis a partir dos vetores/raízes reconhecidos pelo recompilador e/ou rastrear tabelas de ponteiros relevantes.

## Limites desta entrega

Não foi executada nesta revisão uma nova simulação de gameplay nem uma nova CI de runtime; portanto não há novos resultados dinâmicos para declarar. Nenhuma ROM foi modificada. Não houve build Xenon/PowerPC, XEX, integração Xbox ou teste físico. Esta é uma atualização de evidência estática e priorização do próximo experimento.


## Passagem 2 — referências cruzadas às tabelas de despacho (22/10/2026)

Ao ampliar a busca para os destinos anteriores aos callsites e para as palavras de todas as tabelas depois de cada `JSR $8E04`, surgiram ligações mais concretas. Isso corrige a conclusão provisória de que a falta de referência direta aos seis callsites significaria falta de entrada: os fluxos chegam primeiro a blocos anteriores aos callsites.

| Callsite não observado | Caminho estático identificado na ROM | Seletor/condição relevante | Estado da prova |
|---|---|---|---|
| `$83A3` | A tabela do dispatcher em `$8215` tem seletor 2 → `$838B`; `$838B` executa `JSR $83A0`; o bloco em `$83A0` segue por `LDA $0772` até `$83A3`. A palavra de tabela está em `$821C`. | `$8215`, A = 2 | Ligação estática encontrada; o seletor 2 não apareceu nos quatro perfis atuais. |
| `$921B` | A tabela em `$8215` tem seletor 3 → `$9218`; o bloco começa com `LDA $0772` e segue ao callsite `$921B`. A palavra está em `$821E`. | `$8215`, A = 3 | Ligação estática encontrada; seletor 3 não observado. |
| `$9743` | A tabela em `$9664` tem seletor 1 → `$9740`; o bloco em `$9740` executa `LDA $0733` e segue ao callsite `$9743`. A palavra está em `$9669`. | `$9664`, A = 1 | Ligação estática encontrada; seletor 1 não observado. |
| `$C7A8` | A tabela inline em `$C27F` possui entradas para `$C7A0` nas posições 18, 20, 21, 22 e 23 (`$C2A6`, `$C2AA`, `$C2AC`, `$C2AE`, `$C2B0`). O bloco `$C7A0` segue até `$C7A8`. | `$C27F`, seletor em {18, 20, 21, 22, 23} | Ligações estáticas encontradas; nenhum desses seletores foi observado no perfil anterior. |
| `$C987` | A tabela em `$C88F` tem seletores 16–22 → `$C965` (entradas `$C8B2-$C8BE`). O fluxo chega ao `JSR $C982` em `$C976` apenas quando `$0747 == 0`; o bloco `$C982` segue até `$C987`. | `$C88F`, seletor 16–22 **e** `$0747 == 0` | Ligação estática condicional; seletores 16–22 não observados. A condição de RAM também não foi provada em teste. |
| `$D2E5` | A tabela em `$C88F` tem seletor 29 → `$D2D9` (palavra em `$C8CC`). O bloco lê `$0746`, compara com 5 e só passa ao dispatcher em `$D2E5` se a condição `BCS $D311` não for tomada. | `$C88F`, A = 29; depois `$0746 < 5` | Ligação estática condicional; seletor 29 não observado. |

### Correção importante sobre o que significa “referência”

A primeira varredura procurou instruções `JMP abs`, `JSR abs` e branches com destino exatamente igual ao callsite. Isso não detectou os caminhos em que uma tabela inline seleciona um **bloco anterior**, que então cai por fluxo sequencial no callsite. O relatório anterior fica como registro do método inicial e desta limitação; a tabela acima é a análise ampliada.

### Próximo experimento

A melhor próxima tentativa não é prolongar o tempo de jogo sem objetivo nem inserir os seis callsites como seeds arbitrários. A evidência sugere primeiro criar cobertura dirigida para os seletores parentais ainda ausentes: `$8215:2`, `$8215:3`, `$9664:1`, `$C27F:18/20/21/22/23`, `$C88F:16-22` e `$C88F:29`. Para os dois caminhos condicionais, registrar também os valores exigidos de `$0747` e `$0746`. Os roteiros só contarão como cobertura dinâmica quando o profiler observar de fato esses pares callsite/seletor e o runner sem interpretador concluir sem miss.

A análise estática não modificou a ROM, não mudou os seeds permanentes e não executou uma nova CI dinâmica. A última prova dinâmica continua sendo a registrada em `docs/evidence/INLINE_DISPATCH_PROFILE_20261020.md`.
