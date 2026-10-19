# Política obrigatória — descompilação/recompilação nativa de SMB_v026 para Xbox 360

**Ordem explícita do proprietário — 18/10/2026. Esta política prevalece sobre planos antigos que tratem o interpretador 6502 como destino final.**

## Objetivo obrigatório

Produzir um porte de Super Mario Bros. que use a ROM específica `assets/canonical/SMB_v026.nes` como fonte de verdade e rode no Xbox 360 desbloqueado como executável `default.xex`, **sem emulador de NES nem interpretador 6502 durante a execução final**.

O caminho de engenharia exigido é investigar e implementar uma **descompilação/recompilação estática específica desta ROM**: analisar o PRG 6502, traduzir o código de máquina para uma representação de alto nível compilável (C é aceitável e pode ser preferível a C++), compilar o resultado para PowerPC/Xenon e adaptar os serviços de plataforma necessários para o Xbox 360.

Não prometer que uma ferramenta transforma automaticamente a ROM inteira num porte completo. O código CPU, dados de jogo e recursos do NES têm de ser identificados e validados. Vídeo/PPU, áudio/APU, temporização, interrupções, DMA, memória mapeada e entrada do controle precisam de tratamento nativo equivalente; traduzir apenas instruções da CPU não resolve esses componentes por si só.

## Restrições que todo chat/agente deve cumprir

1. **Não substituir o trabalho por um porte C/C++ de terceiros.** Não adotar, copiar ou migrar para um projeto GitHub preexistente que já reimplementa Super Mario Bros. como base do produto. O código executável do jogo deve ser derivado da ROM canônica do proprietário por ferramentas e código de análise/recompilação deste projeto.
2. **Não apresentar emulação ou interpretação como solução final.** O runtime C99 atual que interpreta 6502 é útil como bancada de referência, comparação e testes; não satisfaz o objetivo final nativo. Não o descrever como recompilação estática.
3. Preservar exatamente a ROM canônica e seus assets. ROM: 40.976 bytes; SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`; PRG 32 KiB; CHR 8 KiB; mapper 0/NROM-256. Validar hashes antes de cada trabalho de análise/build.
4. Não substituir o jogo por uma recriação manual de gameplay, um clone, uma ROM diferente ou um projeto pronto. Uma tradução automatizada de instruções pode produzir C pouco legível; legibilidade não é critério de fidelidade, equivalência verificável é.
5. Separar explicitamente os marcos: (a) desmontagem/análise, (b) tradução estática gerada, (c) compilação host, (d) comparação de comportamento contra a ROM/runtime de referência, (e) compilação PowerPC/XEX, (f) integração nativa de vídeo/áudio/controle, (g) validação no Xenia se aplicável e no Xbox físico. CI verde ou XEX estruturalmente válido não prova jogo jogável.
6. A referência host atual pode continuar a ser usada para testes diferenciais, mas não pode ser embutida como interpretador no produto final. Toda divergência deve gerar uma evidência reproduzível; não mascarar opcode não suportado nem tratar instruções desconhecidas como NOP.
7. Antes de escrever um tradutor do zero, investigar offline o estado real do repositório, a ROM canônica, os scripts e os checkpoints existentes. Avaliar ferramentas genéricas de desmontagem/tradução 6502 somente como ferramentas; isso não autoriza adotar um porte de jogo de terceiros. Registrar decisão técnica, limitações e licença de cada ferramenta incorporada.
8. Trabalhar incrementalmente: primeiro identificar mapeamento PRG/endereços/entrypoints/vetores e cobertura do código executável; depois desenhar um tradutor determinístico 6502→C e testes unitários por opcode/flags/endereçamento; em seguida comparar traces/estado/quadros com a referência; só então integrar ao toolchain PowerPC/XEX. Manter uma lista explícita de opcodes, endereços indiretos, tabelas de dados misturadas ao código e casos ainda não suportados.
9. Não iniciar novo teste físico nem gerar/enviar outro XEX de teste automaticamente. S015 e S018 deram Fatal Crash; S017 congelou/retornou sem prova de sucesso. Seguir primeiro a investigação offline e as instruções atuais de NEXT_STEPS.md; nenhum artefato antigo pode ser apresentado como jogável.
10. Não inventar progresso, testes ou artefatos. Atualizar START_HERE, AGENTS, WORK_CHECKPOINT, NEXT_STEPS e SESSION_LOG quando a arquitetura/etapa mudar. Publicar no GitHub e confirmar o HEAD remoto antes de afirmar que as instruções foram salvas.

## Critérios de conclusão do objetivo nativo

O objetivo só pode ser marcado concluído quando: (1) o código CPU do PRG usado no executável é código recompilado estaticamente, sem interpretador 6502 em runtime; (2) a ROM exata e seus dados são preservados; (3) PPU/vídeo, input, temporização e áudio necessários funcionam por integração nativa; (4) testes diferenciais cobrem os cenários relevantes; (5) o XEX inicia e permite jogar no Xbox 360 real. Relatar separadamente qualquer critério ainda pendente.

## Próxima tarefa autorizada

Inspecionar o código/branches atuais e os artefatos existentes; determinar a melhor estratégia técnica para a ROM canônica; criar uma prova de conceito pequena e verificável do tradutor estático 6502→C, sem apagar nem substituir o runtime de referência. Registrar o plano, comandos e evidências no checkpoint e diário. Não declarar que a decompilação já foi realizada.
