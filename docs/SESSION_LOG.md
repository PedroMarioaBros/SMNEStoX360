# Histórico de sessões — acrescentar, não apagar

O checkpoint descreve o presente; este diário preserva avanços, falhas e decisões.
Cada nova sessão deve acrescentar uma entrada, seguindo CONTINUITY_PROTOCOL.md.
A identidade do commit que contém uma entrada pode ser consultada no histórico Git.

## S001 — 06/10/2026, execução canônica no Work

- HEAD recebido: b8de2f29f36316f386a3ff4c54f91a8c76c08b0f.
- Commits publicados:
  - 13e6249255c7f3a9b4dd1921bad360db4e3f9035 — CPU/NMI/PPU/testes.
  - 4c438b85fdb27b4deaf6cfec319e66ad2586b50c — checkpoint/evidências/CI.
- ROM, PRG e CHR verificados pelos hashes canônicos.
- Corrigido runner que não compilava; CPU expandida para 151 opcodes oficiais;
  NMI, registradores PPU, DMA, controle e compositor inicial implementados.
- Sete conjuntos de testes passaram; ADC/SBC 524.288 casos; shifts 10.240.
- Primeiro NMI em $8082, frame 3, contador CPU após entrada 116.744.
- Medição longa: 50.000.000 instruções, 154.288.271 ciclos, frame 5.180,
  5.174 NMIs, 100 opcodes distintos, sem opcode bloqueador.
- Repetição do cenário de 5 milhões: mesmo log e mesmo quadro.
- CI do commit 4c438b8 confirmado como success:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37561513602
- Falhas relevantes: strings C quebradas no HEAD; ausência de NMI e depois
  sprite-zero impediam avanço; LeakSanitizer bloqueado por ptrace; push HTTPS
  sem credencial. Soluções e limites em WORK_CHECKPOINT.md.
- Pendência observada: HUD ausente na imagem da fase; fidelidade não demonstrada.
- Xbox: nenhum XEX novo gerado/testado; build histórico #16 sofreu Fatal Crash.
- Evidências: docs/evidence/2026-10-07/.
- Próxima ação deixada: investigar HUD/PPU contra referência da mesma ROM.

## S002 — 06/10/2026, ordem permanente de continuidade

- Pedido: tornar obrigatório registrar todo avanço no GitHub, com próxima ação
  e próximo comando, e transmitir a obrigação a todo sucessor.
- HEAD de entrada confirmado: 4c438b85fdb27b4deaf6cfec319e66ad2586b50c.
- Escopo: somente documentação/processo; nenhuma alteração no runtime.
- Criados START_HERE.md, CONTINUITY_PROTOCOL.md, NEXT_STEPS.md e este diário.
- Reforçados AGENTS.md e README; atualizado checkpoint com ordem de leitura,
  obrigação de registro progressivo/encerramento e sucesso do CI já confirmado.
- Processo: ler estado remoto → continuar pendência → registrar avanços/tentativas
  → testar conforme escopo → atualizar estado/próximo comando → publicar
  → confirmar HEAD remoto → responder.
- Anti-repetição: não reiniciar etapas prontas nem apresentar reteste como avanço.
  Não criar loop de commits para incluir o hash de um documento nele próprio.
- Validação desta sessão: releitura, links locais e git diff --check.
  Testes de CPU/ROM NÃO foram reexecutados; as medidas acima pertencem à S001.
- Pendência técnica preservada: HUD ausente; próximo comando em NEXT_STEPS.md.
- Evidência da publicação desta entrada: histórico Git deste arquivo e HEAD remoto.
