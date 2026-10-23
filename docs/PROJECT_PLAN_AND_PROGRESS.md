# Plano mestre do projeto — SMB_v026 → Xbox 360 nativo

**Revisão:** 23/10/2026  
**Branch de trabalho:** `research/nesrecomp-evaluation-20261018`  
**PR de avaliação:** [#20 — Evaluate NESRecomp against the canonical SMB_v026 ROM](https://github.com/PedroMarioaBros/SMNEStoX360/pull/20)  
**Últimas CIs validadas:** [push 39924745832 — success](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39924745832) e [PR 39924751591 — success](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39924751591), commit `38abad83e59d81d853a53d8237bc1a0b15e86d38`.

Este é o plano de referência para comunicar andamento, decisões, dependências e critérios de aceite. Percentuais são estimativas de conclusão de entregáveis, ponderadas por importância para o produto final; não representam linhas de código, tempo gasto ou garantia de prazo.

## 1. Objetivo final e restrições que não mudam

Entregar um `default.xex` executável no Xbox 360 que reproduza o SMB da ROM modificada canônica `SMB_v026.nes`, usando **recompilação estática 6502 → código nativo**. A ROM é a fonte canônica do jogo, não um projeto de terceiros.

Critérios obrigatórios:
- Identidade da ROM preservada: 40.976 bytes, SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- Não substituir o jogo pela implementação de outro port de SMB.
- Não deixar interpretador de instruções 6502 como caminho de execução em runtime. A remoção deve ser demonstrada estruturalmente e por execução; qualquer caminho nativo ausente deve falhar explicitamente durante os testes.
- Reproduzir o comportamento necessário do NES: CPU e flags/ciclos, mapa de memória e mapper, PPU/vídeo/scroll/sprites, interrupções, DMA, APU/áudio, controladores e sincronização.
- Integrar o runtime ao ambiente Xbox 360/Xenon/PowerPC, suas convenções ABI, imports, startup, vídeo, áudio e controles.
- Reproduzir os testes de host de forma determinística e documentar evidências, hashes, limitações e artefatos.
- Não gerar nem solicitar teste físico de XEX antes dos gates de host e da revisão da cadeia de build/ABI. Testes físicos anteriores tiveram resultados ruins (Fatal Crash/black screen), então não serão usados como ferramenta de diagnóstico inicial.

## 2. Resumo executivo do estado real

### O que foi provado
- ROM canônica validada por tamanho e SHA-256 na CI.
- NESRecomp, fixado em uma revisão específica, compilou e gerou C a partir da ROM em CI.
- Em um runner experimental, o interpretador foi removido do build: `cpu6502_interp.c` não aparece em `build.ninja`, e o símbolo `cpu_interp_step` não aparece no executável.
- A instrumentação de estado foi primeiro aprovada em [39919520366](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39919520366). A telemetria filtrada foi então validada nos dois CIs do commit `32c2208`: [push 39923036457](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39923036457) e [PR 39923040038](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39923040038).
- Sete roteiros passaram sem interpretador, totalizando 24.600 quadros, com hashes de estado/trace iguais por quadro e PCM/WAV idêntico byte a byte em todos os cenários. Não houve `NATIVE_COVERAGE_MISS`; `cpu6502_interp.c` ficou fora do build e `cpu_interp_step` ausente do executável experimental.
- O cenário `obstacle` atingiu no máximo página 2, `x=$C6`, e reinicializou após a queda; não resolveu o bloqueio. O perfil continua em 62 pares únicos (callsite/seletor), 12 callsites e 50 destinos. Os 16 seletores prioritários continuam sem confirmação dinâmica. Nenhuma fase concluída nem Game Over foi demonstrado.
- O log caiu de 2.315.428 para 431.490 bytes (≈81% menor). O cruzamento da tabela de `$C88F` mostra seletores 16–22 como IDs de objeto `$24–$2A` (plataformas grandes), e seletor 29 como ID `$31` (bandeira). Os sete cenários não carregaram nenhum desses IDs; os modos vitória/Game Over em `$8215` também não foram observados. A nova instrumentação contextual para correlacionar os slots e os callsites ainda aguarda CI.

### O que ainda não foi provado
- Cobertura de todos os caminhos, seletores, estados e fases do jogo.
- Ausência de todos os casos de código que ainda não foram alcançados pelos roteiros.
- Aceite final de NESRecomp como base: patches e seeds experimentais são temporários; a licença upstream é PolyForm Noncommercial 1.0.0 e precisa ser tratada explicitamente para qualquer distribuição/uso comercial.
- Portação/compilação de código nativo para Xenon/PowerPC, ABI e imports Xbox.
- Inicialização e estabilidade do XEX, renderização final, controles e áudio no Xbox.
- Aceite de release.

**Não existe ainda um XEX final validado.** A prova atual é de viabilidade de recompilação e paridade em cenários limitados de host, não de jogo pronto.

## 3. Plano por etapas e percentuais

Os pesos abaixo somam 100%. A coluna “conclusão estimada” mede a etapa; “contribuição” é peso × conclusão.

| ID | Etapa / entregável | Peso | Conclusão estimada | Contribuição |
|---|---|---:|---:|---:|
| P1 | Requisitos, ROM canônica, hash e política do projeto documentados | 5% | 100% | 5,0% |
| P2 | Avaliação de arquitetura/recompilador, licença e prova inicial de geração C | 10% | 60% | 6,0% |
| P3 | Recompilação estática, fluxo de controle e remoção completa do interpretador em toda a ROM | 20% | 15% | 3,0% |
| P4 | Runtime NES completo e integrado (CPU/PPU/APU/mapper/DMA/IRQ/entrada/tempo) | 20% | 10% | 2,0% |
| P5 | Testes de host, cobertura determinística e paridade diferencial de estado/vídeo/áudio | 10% | 60% | 6,0% |
| P6 | Toolchain Xenon/PowerPC, ABI, CRT/startup, imports e build nativo | 15% | 0% | 0,0% |
| P7 | Integração Xbox: ciclo de vida, vídeo, controles, áudio e desempenho | 10% | 0% | 0,0% |
| P8 | QA final, empacotamento, reprodutibilidade, documentação e aceite | 10% | 0% | 0,0% |
|  | **Total ponderado do projeto** | **100%** |  | **22,0%** |

**Percentual de planejamento adotado nesta revisão: 22%.** É uma estimativa ponderada de progresso técnico em relação ao produto final, não uma afirmação de que 22% do jogo já esteja pronto. A estimativa anterior de 20% era uma medida aproximada por marcos; a presente tabela explicita pesos e reduz o risco de confundir uma boa prova de host com portabilidade Xbox concluída. Recalcular após cada gate relevante e explicar qualquer alteração de método.

### Interpretação por etapa

- **P1 — 100%:** ROM, hash, regras e limites estão registrados e verificáveis.
- **P2 — 60%:** geração C e execução de pesquisa estão provadas, mas arquitetura final, licença e viabilidade do alvo Xenon ainda não estão aprovadas.
- **P3 — 15%:** o build experimental sem interpretador passou em caminhos observados; 6 de 18 candidatos de dispatcher ainda não foram observados, e a cobertura global continua sem prova. Os 17 seeds derivados de perfil são temporários e não constituem solução geral.
- **P4 — 10%:** há um runner de referência funcional no host para testes, mas integração completa/aceite dos componentes do NES para o produto final ainda não foi demonstrada.
- **P5 — 60%:** a infraestrutura de teste diferencial está funcional; sete cenários totalizam 24.600 quadros comparados e sete saídas PCM/WAV coincidentes. Faltam cobertura de seletores/fluxo ampla, transições e fases, além de critérios independentes de vídeo/áudio.
- **P6–P8 — 0%:** nenhum build Xenon/PowerPC final, XEX de produto ou aceite Xbox foi demonstrado neste fluxo de trabalho.

## 4. Procedimentos e gates de engenharia

### Etapa A — consolidar a cobertura estática (agora)
1. Manter a ROM e o SHA-256 como entrada imutável.
2. Enumerar os 18 candidatos brutos a `JSR $8E04`, as tabelas inline adjacentes e os seletores observados.
3. Analisar os seis candidatos ainda não exercitados: `$83A3`, `$921B`, `$9743`, `$C7A8`, `$C987`, `$D2E5`. Os bytes adjacentes são pistas, não prova de que cada candidato seja um caminho alcançável em execução.
4. Inspecionar o código de chamada e as tabelas para decidir quais cenários/inputs podem atingir cada candidato de forma válida. Não inventar seeds nem tratar cada PC interpretado como entrada de função.
5. Criar inputs determinísticos novos, registrando o estado inicial, sequência de controles, duração, caminho alcançado e resultado esperado.
6. Aplicar instrumentação de falha explícita para registrar PC, banco/slot, opcode e origem do último salto/chamada sempre que um caminho nativo estiver ausente.
7. Gerar o código em diretório temporário; não comitar nem publicar C gerado.
8. Gate A: zero `NATIVE_COVERAGE_MISS` nos roteiros aprovados; nenhum caminho de fallback interpretado; cada seed explicado e verificado contra ROM; regressão dos quatro roteiros existentes aprovada.

**Saída:** mapa de cobertura/dispatch versionado, testes determinísticos ampliados e evidência de execução nativa para caminhos adicionais. Isso ainda não é prova matemática de cobertura total.

### Etapa B — decidir arquitetura e eliminar dependências experimentais
1. Revisar a revisão upstream pinada, licença PolyForm Noncommercial 1.0.0 e licenças de dependências.
2. Separar claramente o que é framework genérico do que é código específico de um port de SMB; não copiar/adotar o projeto de jogo de terceiros como base.
3. Avaliar se as mudanças experimentais necessárias podem ser generalizadas e mantidas de forma sustentável, ou se exigem uma camada própria.
4. Substituir seeds manuais por descoberta estática/regras genéricas de fluxo de controle sempre que tecnicamente possível. Onde não for possível, documentar a limitação e provar todos os destinos válidos por análise/execução.
5. Definir a estratégia para dados executáveis, tabelas inline, código em fronteiras de slot, interrupções e endereços de retorno.
6. Gate B: arquitetura e licença aprovadas, build repetível a partir de commit pinado, nenhum interpretador no artefato de runtime e relatório de cobertura conhecido.

### Etapa C — fechar a conformidade do NES em host
1. Inventariar requisitos por subsistema: 6502/flags/ciclos; mapa de memória e mapper 0; PPU; scroll/sprites; NMI/IRQ/DMA; APU; controladores; timing.
2. Para cada subsistema, adicionar testes de unidade ou cenários de referência.
3. Usar a mesma ROM e as mesmas entradas no runner de referência e no recompilado.
4. Comparar estado/trace por quadro; quando houver divergência, reduzir ao primeiro quadro/ciclo reproduzível antes de mudar código.
5. Comparar quadros de vídeo e artefatos PPU/scroll relevantes, não somente hashes globais.
6. Comparar PCM/WAV byte a byte, incluindo formato, número de amostras, temporização e transições de áudio.
7. Cobrir boot/reset, título, Start, movimento, pulo, colisão, morte, transição de fase, power-ups, pausa/espera, limites de scroll, reset, IRQ/NMI e sequências longas.
8. Gate C: matriz de testes documentada e sem divergências não explicadas; cobertura de gameplay definida por estados/transições e não apenas por número de quadros.

### Etapa D — prova de portabilidade Xenon/PowerPC (só após gates A–C)
1. Identificar e fixar a toolchain que gera código compatível com Xbox 360/Xenon/PowerPC.
2. Validar formato de objeto, convenções ABI, alinhamento, relocations, imports, CRT/startup, stack, heap e símbolos necessários.
3. Compilar primeiro um executável de diagnóstico mínimo, sem ligar subsistemas de vídeo de baixo nível nem endereços MMIO adivinhados.
4. Inspecionar o XEX gerado: formato, entry point, imports, seções, alinhamento e dependências.
5. Validar o build em CI e preservar logs/artefatos com hash; não tratar “compilou” como “executa”.
6. Gate D: executável mínimo passa inspeção estática e os testes disponíveis de loader/ABI antes da integração do jogo.

### Etapa E — integrar runtime ao Xbox
1. Projetar ciclo de vida Xbox e inicialização/encerramento sem interferir em áreas de memória não documentadas.
2. Implementar/validar apresentação de vídeo e formato de framebuffer com APIs/contratos conhecidos.
3. Integrar XInput/controles e mapear estados ao runtime NES.
4. Integrar áudio e sincronização sem bloquear a execução do jogo.
5. Medir CPU, memória, latência, sincronização e estabilidade em host primeiro; otimizar somente com medições.
6. Gate E: build de CI repetível, diagnósticos claros, sem crash no teste controlado de inicialização e subsistemas testados separadamente.
7. Só após autorização explícita e checklist de risco, considerar um teste físico incremental. Nenhum teste físico é solicitado ou executado por este plano automaticamente.

### Etapa F — QA, release e entrega
1. Executar a matriz completa de cenários e regressão de build limpo.
2. Confirmar que o runtime final não contém interpretador nem dependências temporárias de pesquisa.
3. Confirmar que o jogo usa a ROM canônica como fonte e que não há substituição de gameplay por código de terceiros.
4. Validar desempenho, estabilidade, controles, áudio, vídeo e ciclos de reinício/saída.
5. Gerar `default.xex`, registrar SHA-256, commit de origem, versões de ferramentas, logs de build e instruções de execução.
6. Auditar licenças, redistribuição, arquivos incluídos e documentação.
7. Gate F: checklist de aceite assinado em evidência, artefato reproduzível e limitações conhecidas declaradas.

## 5. Como o progresso será reportado

A cada ciclo de trabalho, atualizar:
- **Etapa executada** e objetivo específico.
- **Procedimentos feitos**, com paths/commits e configurações relevantes.
- **Teste/CI** e resultado real (success/failure/inconclusivo).
- **Evidências quantitativas** (hash, quadros, cenários, destinos, divergências).
- **O que foi provado** versus **o que ainda é hipótese**.
- **Percentual por etapa e total ponderado**, com explicação se mudar.
- **Bloqueios, riscos, decisão tomada e próxima ação**.
- **Artefatos produzidos** e se são permanentes ou temporários.

Não contar como progresso concluído: tempo de execução, tentativa sem evidência, geração C sem runner, build que apenas compila, teste não repetível, cenário não exercitado, ou XEX que não foi validado. Não esconder falhas; uma falha de CI deve ser registrada e diagnosticada antes de declarar a etapa aprovada.

## 6. Riscos principais e respostas

| Risco | Consequência | Mitigação / gate |
|---|---|---|
| Fluxo dinâmico ou tabela inline não descoberta | Miss de código nativo ou comportamento incompleto | Análise estática + perfil dirigido + fail-fast; não alegar cobertura total |
| Dependência de seeds manuais | Configuração frágil e não generalizável | Generalizar análise, documentar exceções e revisar seeds |
| Interpretador residual | Viola requisito de recompilação estática | Inspeção de build/símbolos + execução fail-fast |
| Licença upstream não permite uso pretendido | Impede distribuição/uso comercial | Revisão de licença antes de fixar arquitetura ou distribuir |
| Diferenças de timing/PPU/APU | Desync, glitches, áudio incorreto | Testes diferenciais e regressão por quadro/ciclo/PCM |
| Toolchain/ABI Xenon desconhecida ou incompatível | XEX não carrega ou falha | Prova mínima de toolchain e inspeção de imports antes de integrar jogo |
| Uso de MMIO/endereço não validado | Fatal Crash | Não usar endereços adivinhados; integrar subsistemas gradualmente |
| Confundir prova de host com jogo Xbox pronto | Estimativa enganosa | Percentuais ponderados e gates separados para host, XEX e hardware |

## 7. Próximas ações em ordem

1. **Concluído nesta revisão:** cruzar os seis callsites com as tabelas parentais. O relatório `docs/evidence/UNOBSERVED_DISPATCH_STATIC_REVIEW_20261022.md` registra as entradas e condições: `$8215` seletores 2/3, `$9664` seletor 1, `$C27F` seletores 18/20/21/22/23 e `$C88F` seletores 16–22/29. O caminho `$C987` exige `$0747 == 0`; o caminho `$D2E5` exige `$0746 < 5`.
2. Criado `tools/analyze_unobserved_dispatchers.py` e integrado ao harness/CI para validar os bytes e destinos de tabela na ROM canônica. Isso verifica a análise estática, não conta como cobertura dinâmica.
3. **Próximo:** criar cenários de perfil dirigidos para os pares callsite/seletor ainda ausentes. Para `$C987` e `$D2E5`, acompanhar também as condições de RAM registradas.
4. Reexecutar os quatro roteiros atuais e todos os novos em runtime sem interpretador; exigir zero `NATIVE_COVERAGE_MISS`, hashes por quadro e PCM/WAV idênticos.
5. Fazer revisão independente de seeds e de cobertura; registrar caminhos ainda não demonstrados.
6. Só então consolidar a decisão de arquitetura/licença e prosseguir para a matriz completa de conformidade host.
7. Depois dos gates host, iniciar o trabalho de toolchain Xenon/PowerPC e executável mínimo.
8. Integrar o jogo ao Xbox e preparar release somente após os gates anteriores.

**Estado da etapa atual (23/10/2026):** a CI [39919520366](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39919520366) concluiu success e validou os snapshots `[SMB_STATE]`. Seis roteiros/21.000 quadros continuaram passando sem interpretador, com paridade de estado/trace e PCM/WAV. Os snapshots mostram os cenários atuais estagnando por volta da página 2, x≈$D2, e depois reinicializando; nenhum dos 16 seletores prioritários foi observado. A próxima CI instrumenta `$0746/$0747` e adiciona um cenário de salto antecipado; o progresso ponderado continua **22%**. Nenhum build Xenon/PowerPC, XEX ou teste físico foi realizado.
