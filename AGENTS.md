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
