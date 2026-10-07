# Ordem permanente de continuidade — todas as sessões

Estabelecida explicitamente pelo proprietário em 06/10/2026, às 23:24
(America/Sao_Paulo). Aplica-se a qualquer chat, Work ou agente que continuar
PedroMarioaBros/SMNEStoX360.

**Todo avanço, alteração ou tentativa relevante deve ficar registrado no GitHub.
Toda sessão deve deixar o próximo passo e o próximo comando explícitos.
Cada sucessor deve cumprir e preservar esta mesma ordem.**

## Fonte persistente e responsabilidades

- START_HERE.md: porta de entrada.
- AGENTS.md: regras permanentes para os agentes.
- WORK_CHECKPOINT.md: fotografia atual e evidências técnicas.
- NEXT_STEPS.md: uma tarefa ativa, critério de conclusão, comandos e dependências.
- SESSION_LOG.md: histórico cronológico; acrescentar entradas, sem apagar tentativas.
- evidence/: resultados textuais reproduzíveis, identificados por cenário e versão.
- Histórico Git: conteúdo exato de cada alteração e seus commits.

Os quatro documentos em docs/ complementam-se. Se houver divergência, conferir
código, HEAD e evidências; registrar a correção, não escolher silenciosamente
a versão mais conveniente. Instruções novas do proprietário prevalecem.

## Ao abrir uma sessão

1. Obter HEAD remoto, branch e estado local. Preservar alterações de terceiros.
2. Ler START_HERE.md e os arquivos indicados, inclusive a última entrada do diário.
3. Identificar exatamente a tarefa ativa e o último resultado verificado.
4. Usar assets/canonical/SMB_v026.nes do próprio checkout e validar fingerprint.
   Executar python3 tools/verify_repository.py para conferir assets e arquivo histórico.
5. Comunicar brevemente o ponto de retomada e começar a tarefa pendente.
6. Não refazer descobertas, implementações ou medições concluídas como se fossem
   progresso novo. Repetições para baseline/regressão são permitidas, identificadas
   como verificação e justificadas por uma necessidade concreta.

## A cada avanço ou tentativa relevante

Registrar, sem esperar o encerramento de uma sessão longa:

- o que mudou e por quê; arquivos e decisões de arquitetura;
- o que foi apenas implementado, compilado, testado sinteticamente, validado
  com a ROM ou validado no Xbox, separadamente;
- comandos executados, entradas/cenários, resultados exatos e evidências;
- falhas, hipóteses descartadas e tentativas que não resolveram o problema;
- limitações e dependências ainda existentes;
- estado atual da tarefa e a ação imediatamente seguinte.

Agrupar alterações coerentes em commits pequenos e publicá-los depois de
releitura e validação adequada. Não exigir commit por linha de código.
Em trabalhos longos, atualizar checkpoints intermediários: é proibido deixar
todo o progresso apenas na conversa ou em arquivos transitórios.

Se uma experiência ainda quebrar testes, não publicá-la como implementação
validada em main. Usar branch de trabalho para preservar a experiência quando
necessário, registrando branch, falha e como retomar. Nunca perder o diagnóstico.

Atualização explícita do proprietário na S003 (06/10/2026 às 23:30):
ROM, PRG, CHR e todos os arquivos do projeto necessários à continuidade devem
estar no GitHub. Essa ordem substitui a restrição anterior de não versionar ROM.
Guardar evidências, ferramentas e outputs únicos; outputs volumosos podem ficar
em arquivos compactados versionados, acompanhados de inventário e hashes.
Não deixar arquivos necessários apenas no ChatGPT ou no ambiente temporário.
Credenciais, tokens e URLs assinadas não são arquivos do projeto e não devem
ser publicados. As ferramentas padrão de compilação são dependências documentadas.

## Antes de encerrar ou responder com resultado final

1. Reler o diff e os arquivos editados.
2. Executar a validação apropriada ao escopo:
   - código: compilar/testar conforme AGENTS.md;
   - documentação isolada: verificar links, consistência e git diff --check;
     não anunciar testes de runtime novos se não houve execução.
3. Atualizar WORK_CHECKPOINT.md preservando evidências e limites.
4. Acrescentar uma entrada em SESSION_LOG.md.
5. Atualizar NEXT_STEPS.md com tarefa ativa, próximo comando copiável,
   resultado esperado, critério de conclusão e bloqueios.
6. Fazer commit e publicar no GitHub pela conexão autorizada.
7. Consultar novamente HEAD/branch remoto e confirmar conteúdo/árvore publicada.
   Commit apenas local não conta como salvo no GitHub.
8. Somente então responder ao proprietário com commits, resultados, pendências
   e ponto de retomada.

Esta é a definição de encerramento normal. Se publicação ou teste estiver
bloqueado, preservar o que for possível e informar explicitamente o que NÃO
foi salvo ou validado, com o erro e a ação necessária. Nunca ocultar falha para
satisfazer a regra de encerramento.

## Identificação sem dependência circular

O diário registra o HEAD de entrada e os commits de implementação já conhecidos.
O commit que contém a própria entrada é encontrado por:

```sh
git log -1 --format='%H %s' -- docs/SESSION_LOG.md
```

Não fazer uma sequência infinita de commits só para escrever o hash do próprio
commit dentro dele. A confirmação do HEAD publicado fica também na resposta
ao proprietário. Novas evidências de CI devem apontar ao run e commit corretos;
um run anterior verde não prova um commit novo.

## Preservação para o próximo sucessor

Não remover ou enfraquecer este protocolo numa limpeza de documentação.
Se o proprietário alterar o processo, registrar a alteração e sua data.
Todo sucessor deve entregar os mesmos documentos atualizados para que o seguinte
continue de onde o trabalho parou.
