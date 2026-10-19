# Continuidade obrigatória — SMNEStoX360

1. Comece por START_HERE.md e leia a sequência indicada. Cumpra
   docs/CONTINUITY_PROTOCOL.md em toda sessão, inclusive sessões só documentais.
   O GitHub é a fonte persistente; não depender da memória do chat.
2. SMB_v026.nes é a fonte exclusiva do jogo. Nunca substituir PRG/CHR ou gameplay
   por uma recriação de terceiros. Não obter outra ROM para completar dados.
3. Leia HEAD e arquivos antes de editar. Edite → releia → compile com
   -Wall -Wextra -Werror → teste. Verifique escapes/newlines em C.
4. Rode sh tests/run_host_tests.sh. Execução canônica deve passar pelo wrapper
   tools/run_canonical.py (validação SHA-256 ROM/PRG/CHR).
5. Por ordem explícita do proprietário (S003), a ROM, PRG e CHR canônicos ficam
   versionados em assets/canonical/. Todo arquivo necessário à continuidade deve
   estar no GitHub, inclusive evidências e ferramentas. Não depender de anexos,
   memória ou arquivos exclusivos do ChatGPT. Conferir hashes antes de executar.
   Nunca incluir credenciais, tokens ou URLs assinadas.
6. Opcodes não oficiais devem parar. Não tratar como NOP para mascarar erro.
7. Não inventar métricas, commits ou sucesso de hardware. Separar implementado,
   compilado, teste sintético, validado na ROM e validado no Xbox.
8. A cada avanço ou tentativa relevante, registre resultados, decisões e falhas
   no GitHub, com checkpoints intermediários em trabalhos longos. Antes de
   responder ao final, atualize docs/WORK_CHECKPOINT.md, acrescente entrada em
   docs/SESSION_LOG.md e atualize docs/NEXT_STEPS.md com próximo comando,
   resultado esperado e bloqueios. Faça commits, publique e confirme HEAD remoto.
   Commit apenas local não satisfaz a ordem. Se bloqueado, informe a falha real.
9. Atualizações curtas ao usuário durante o trabalho. Não pedir nova autorização
   para etapas rotineiras já autorizadas; teste físico depende do proprietário.
10. Build antigo #16: Fatal Crash no hardware. Backends antigos são históricos.
    Não usar seus endereços de vídeo hardcoded. Não declarar XEX2/CI como jogo funcional.

11. Esta ordem é permanente e se aplica a cada sucessor. Preserve-a no repositório
    e deixe a próxima sessão pronta para continuar. Não remover numa limpeza.
12. Não reiniciar etapas concluídas sem razão técnica registrada; retestes são
    verificações, não novo progresso. Histórico de tentativas deve ser preservado.


13. **Objetivo final obrigatório: recompilação estática nativa da ROM SMB_v026.nes.**
    Leia e cumpra docs/NATIVE_RECOMPILATION_POLICY.md antes de mexer na arquitetura.
    Não adotar um porte/reimplementação SMB de terceiros como base do produto e não
    entregar emulação/interpretação 6502 como solução final. O runtime interpretado
    atual é apenas referência host. Traduzir o PRG 6502 para C (ou representação
    equivalente compilável) e compilar para PowerPC/Xenon; tratar separadamente PPU,
    APU, temporização, DMA, memória mapeada e backend Xbox. Não declarar progresso
    sem código/evidência. S015/S018 deram Fatal Crash e S017 foi inconclusivo; não
    retestar nem emitir novo XEX automaticamente.

14. POC de recompilação estática em andamento: execute `python3 tools/static6502_poc.py --self-test` e `sh tests/run_host_tests.sh`. A ferramenta deve rejeitar opcodes ainda não suportados; não ampliar cobertura sem testes de flags/endereçamento e comparação diferencial. A evidência atual cobre apenas seis instruções lineares do reset e não autoriza alegar recompilação integral ou gerar XEX.

14. **Avaliação NESRecomp (18/10/2026):** a pedido do proprietário, pausar a
    expansão do tradutor próprio até avaliar `tools/evaluate_nesrecomp.sh` e
    `docs/evidence/NESRECOMP_EVALUATION.md`. Usar a ROM canônica e o commit
    upstream fixado pelo harness. Não copiar/vendorizAR código ou jogo de
    terceiros sem revisão de licença; não usar o port SMB de terceiros como base.
    A licença upstream consultada é PolyForm Noncommercial 1.0.0.
15. A documentação NESRecomp descreve um interpretador de fallback. O objetivo
    final proíbe esse fallback no runtime; cobertura estática completa e ausência
    de execução interpretada precisam ser demonstradas antes de considerar
    adoção. Geração de C, CI verde ou build host não provam XEX nem jogabilidade.
16. Não versionar nem publicar o código gerado por este primeiro harness. A
    avaliação guarda-o em diretório temporário e registra apenas métricas/logs
    necessários. Não gerar XEX nem solicitar teste físico nesta fase.
