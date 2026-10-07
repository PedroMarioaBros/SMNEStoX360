# Continuidade obrigatória — SMNEStoX360

1. Leia README.md, docs/SMB_V026_CANONICAL_ROM.md e docs/WORK_CHECKPOINT.md.
2. SMB_v026.nes é a fonte exclusiva do jogo. Nunca substituir PRG/CHR ou gameplay
   por uma recriação de terceiros. Não obter outra ROM para completar dados.
3. Leia HEAD e arquivos antes de editar. Edite → releia → compile com
   -Wall -Wextra -Werror → teste. Verifique escapes/newlines em C.
4. Rode sh tests/run_host_tests.sh. Execução canônica deve passar pelo wrapper
   tools/run_canonical.py (validação SHA-256 ROM/PRG/CHR).
5. Não publicar ROM, PRG ou outros novos dumps de assets. ROM é input privado.
6. Opcodes não oficiais devem parar. Não tratar como NOP para mascarar erro.
7. Não inventar métricas, commits ou sucesso de hardware. Separar implementado,
   compilado, teste sintético, validado na ROM e validado no Xbox.
8. Antes de finalizar cada sessão, atualize docs/WORK_CHECKPOINT.md com resultados,
   falhas, limitações, bloqueio e próximo passo; faça commits e confirme o HEAD remoto.
9. Atualizações curtas ao usuário durante o trabalho. Não pedir nova autorização
   para etapas rotineiras já autorizadas; teste físico depende do proprietário.
10. Build antigo #16: Fatal Crash no hardware. Backends antigos são históricos.
    Não usar seus endereços de vídeo hardcoded. Não declarar XEX2/CI como jogo funcional.
