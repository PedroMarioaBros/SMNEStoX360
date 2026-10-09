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

## S003 — 06/10/2026, arquivos completos e ROM no GitHub

- Ordem explícita do proprietário: incluir também a ROM e todos os arquivos
  do projeto no GitHub, sem depender de anexos ou estado do ChatGPT.
- HEAD de entrada: 99624149fbc0cffc00fbe88eee5e5a9274f03b47.
- Essa ordem substitui a proibição anterior de versionar ROM/PRG.
- ROM, PRG, CHR e manifest incluídos em assets/canonical/; hashes canônicos
  confirmados. .gitignore ganhou exceção específica para essa ROM.
- Outputs completos de build da S001 preservados em
  artifacts/session-S001-host.tar.xz, com inventário SHA-256 de arquivos e membros:
  binários host, logs, screenshots, quadros brutos, resultados repetidos/sanitizados
  e extração da ROM. Fontes temporárias também preservadas como histórico.
- Runner Python agora usa a ROM versionada por padrão. Novo verificador de
  integridade e guia REPRODUCIBILITY.md documentam retomada por clone limpo.
- AGENTS, protocolo, README, ponto de entrada, checkpoint e próximos comandos
  atualizados para não procurar ROM no ChatGPT.
- Verificação: exportação limpa dos arquivos do índice, sem copiar o antigo
  build nem usar ../rom; sete suítes passaram e cenário de 5 milhões gerou
  log e hash de quadro idênticos ao registro S001.
- Objetos binários enviados ao GitHub tiveram Git blob SHA conferido com o local.
- Sem alteração de gameplay. HUD ausente permanece como próxima tarefa.
- Nenhum XEX novo. Não extrapolar os resultados host para o Xbox.

## S004 — 06/10/2026, reprodução por clone limpo e diagnóstico inicial do HUD

- HEAD de entrada: 3f1a02e505d7a1a60a117240350abe4b776beb1e.
- Clone limpo do GitHub executou tools/verify_repository.py, todos os testes host e o roteiro canônico de 5 milhões; sem usar ../rom ou artefatos da sessão local.
- Resultado do clone limpo: ROM SHA-256 canônico; quadro reproduzido com SHA-256 f8f3847872da7e7768e9244513f789240f2eabdb64404a777a066948009659e5; diff do log contra a evidência S001 sem divergência.
- CI do commit S003 concluiu success: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37562667552
- Diagnóstico adicional no mesmo roteiro: frame 516, PC $8057, ppu_v=$632a, ppu_t=$000a, fine-X 3, PPUCTRL $90, PPUMASK $1e, 333 sprite-zero hits. A nametable 0 contém dados em linhas superiores, porém o quadro final tem topo visual vazio. Evidência bruta: docs/evidence/2026-10-07/inspect-5m.txt.
- Interpretação atual: existe estado gráfico de status, mas a composição funcional está aplicando scroll vertical/horizontal ao HUD; ainda não há correção publicada. Não afirmar que essa é a causa final até capturar a primeira divergência com uma referência ou um trace de writes PPU.
- Próxima ação permanece implementar/instrumentar separação de v/t e região fixa do HUD baseada na semântica de scroll da PPU, com teste sintético e comparação do mesmo roteiro. Não inserir condição específica para endereços do SMB.

## S005 — 07/10/2026 UTC, correção de timing e auditoria do HUD

- HEAD recebido: 11cd0245985577ea7d6ef3a3e47ff015e06f13ed. Clone limpo,
  hashes canônicos e sete suítes host confirmados antes de editar.
- Correção de diagnóstico: o quadro bruto anterior contém HUD visível.
  As afirmações de topo vazio nas S001–S004 ficam supersedidas por esta evidência.
  Não houve patch de gameplay/scroll para desenhar o HUD.
- Implementado o dot omitido dos frames ímpares NTSC com rendering ativo.
  Testes de 89.342/89.341 clocks, paridade com rendering desligado e trocas
  de PPUMASK no limite. Referência técnica: NESdev PPU frame timing/rendering.
- Falha durante desenvolvimento: a primeira fixture sintética não fornecia CHR
  e causou segmentation fault ao habilitar rendering. Corrigida para inicializar
  NROM com arrays PRG/CHR válidos. Não mascarar o acesso no runtime.
- Novos testes falham contra ppu_timing.c do HEAD antigo; falha esperada registrada.
- Criada conversão reproduzível dos índices para PNG com stdlib; PNG decodificado
  com Pillow preservou todos os índices. Cores ilustrativas, não validação NTSC.
- Medições novas: 5m/15.390.305 ciclos/frame 516/207:88/99 opcodes/510 NMIs;
  50m/154.288.005 ciclos/frame 5180/219:340/100 opcodes/5174 NMIs.
  PC $8057, sem opcode bloqueador. Primeiro NMI $8082/frame 3/ciclo 116.744.
  Quadros finais mantêm os hashes históricos. Não é aumento do recorde de 50m.
- Logs completos de opcode hits, imagem e comandos em docs/evidence/S005/.
- CI foi atualizado para comparar com o log S005, pois os contadores mudaram.
- Próximo passo: referência independente alinhada com a mesma ROM; sem
  certificação integral do gameplay, áudio ou Xbox. Nenhum XEX novo produzido.
- Validação final: sete suítes normais e ASan/UBSan passaram; execução sanitizada
  de 5m reproduziu integralmente log e quadro. Leak detection desabilitado.

## S006 — 07–08/10/2026, referência independente e controle 2

- HEAD: e99b80760d72387a8729b2fb9ddd3056cfa0bf22. Hashes canônicos e
  sete suítes confirmados antes das alterações.
- Obtido binjnes e2f5871a28ff189daa82b70be9324a42e2aaf9fd; licença MIT.
  Fonte usada preservada byte a byte em tests/reference/vendor, com manifest.
- Adaptador captura cada VBlank; binjnes usa evento dot 2, core usa fronteira de
  instrução após dot 1. Ordinais alinhados sem offset; entradas após captura
  anterior, antes do polling NMI, diferente do roteiro antigo por início de frame.
- Inicialmente: 600 idle e 6000 scripted com pixels iguais. RAM scripted divergia
  em 5895 capturas, primeira 103, endereços $06fd/$074b.
- Causa: reads $4017 devolviam apu_io, confundindo writes APU com controle 2.
  Implementado segundo controle e separação de leitura/escrita com teste sintético.
- Depois: 600/600 idle e 6000/6000 scripted com pixels e RAM integral iguais.
  57.751.826 instruções, 178.680.628 ciclos, frame 5999, linha/dot 241/6,
  5992 NMIs no ponto capturado. Maior contagem medida, não gameplay completo.
- Ferramenta compare_reference.py verifica ROM/PRG/CHR e fonte fixada, compila,
  executa e retorna erro se qualquer quadro ou byte de RAM divergir.
- Falha inicial: -Werror em upstream detectou format-security no disassembler.
  Exceção somente para esse warning upstream, fonte intacto. Tentativa subsequente
  sem binário de referência falhou; capturas refeitas depois da compilação válida.
- Host: sete suítes normais e sanitizadas passaram. Runner tradicional 5m normal
  e ASan/UBSan produziu log e quadro idênticos; leak detection desabilitado.
- CI atualizado para novo baseline e comparação curta 600+600. Resultado remoto
  ainda não consultado nesta entrada; execução local não prova CI remoto verde.
- Evidências completas compactadas com hashes em docs/evidence/S006, incluindo
  capturas antes/depois e logs. Nada único necessário fica só no chat.
- Próxima etapa: host interativo reutilizando core canônico e controles, preservando
  teste diferencial. APU e Xbox permanecem pendentes; nenhum XEX novo.

## S007 — 08/10/2026, host SDL2 com ROM embutida

- Entrada: edfdc149bb6d3a47cceac03a82f5bd345a5d9177; árvore limpa, hashes
  verificados. Core canônico mantido sem alterações.
- Implementados build com fingerprint obrigatório e PRG/CHR embutidos, frontend
  SDL2 com vídeo, teclado 2 jogadores, suporte a 2 gamepads/hotplug, pausa/foco.
- Dependências: runtime SDL2 2.30.0 presente; headers/pkg-config ausentes.
  apt-get falhou por permissões setgroups/seteuid. Sem escalada: headers obtidos
  do SDL oficial release-2.30.0, commit 859844eae358447be8d66e6da59b6fb3df0ed778.
  Documentada instalação padrão libsdl2-dev e alternativa de headers locais.
- Corrigida duplicação potencial de gamepads entre enumeração inicial e evento
  DEVICEADDED, verificando instance ID antes de abrir outro handle.
- Teste SDL em diretório vazio: 600 quadros, 5.794.218 instruções, 17.865.924
  ciclos, 593 NMIs, 101 opcodes; seis transições de teclado. Sem ROM externa.
- Readback SDL conferiu 61.440 pixels; quadro final bate binjnes S006/captura 599.
- Sete suítes host passaram. Smoke normal e ASan/UBSan passaram, dumps iguais;
  leak detection desabilitado. Nenhum teste manual/gamepad físico/tempo real.
- CI ganhou build SDL e smoke; resultado remoto não confirmado nesta entrada.
- Evidências e binário Linux x86-64 preservados em docs/evidence/S007, com hashes.
  O programa só grava dump quando --dump é solicitado. Sem arquivo automático.
- Não é produto Xbox: sem APU, XEX ou teste físico. Próxima ação: base temporal
  APU e primeiro canal com testes; frontend pronto para teste humano em Linux.

## S008 — 09/10/2026, início da APU Ricoh 2A03 e pulse 1

- HEAD de entrada em main: `f24d735559ca74eee7467224b5023402e64fdf69`.
  Continuidade feita em `work/s008-apu-pulse1-20261009` para manter
  mudanças não certificadas fora de main.
- Confirmado SHA-256 idêntico da ROM de 40.976 bytes e dos blocos PRG/CHR
  canônicos do proprietário. Nenhum gameplay, gráfico ou som foi substituído.
- Consultadas especificações NESdev de APU, pulse, sweep e envelope.
- Módulo `apu.[ch]` criado: relógio por CPU, frame counter 4/5 passos,
  pulse 1 com duty/timer/length/envelope/sweep, status parcial, IRQ flag
  interna e saída DAC 0..15. Integrado em `nrom.[ch]` e `machine.c`.
  APU recebe também os ciclos de interrupção e stall de DMA.
- Preservado contrato $4017: writes para frame sequencer e reads somente
  para o segundo controle. Teste existente preservado e expandido com
  asserções de alinhamento APU/CPU, inclusive DMA. Novo `test_apu_pulse.c`
  incorporado ao runner de testes.
- Compilação e execução **isoladas** da APU em C99 com
  `-Wall -Wextra -Werror` passaram; repetição ASan/UBSan passou, LeakSanitizer
  desativado. Foram executadas três funções de teste novas cobrindo ciclos
  e efeitos especificados. Não houve reprodução local do projeto completo:
  `git clone` falhou com `Could not resolve host: github.com`.
- CI GitHub Actions para `a41845a49415e04c6d71407bb6928615c14c7195`
  concluiu **success**:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888335327 .
  O commit `af36b563ee19543acd826bd89a768748e6ae0b5e`
  adicionou asserções de integração depois daquele run; deve ser validado
  pelo CI correspondente. Nenhum resultado de hardware foi medido.
- Limites: ainda sem PCM, mixer, canal pulse 2, triangle, noise, DMC,
  IRQ conectada à CPU ou precisão por microciclos. Clock agregado no final
  das instruções; APU $4015 parcial. Sem áudio audível / XEX novo.
- Evidência e comandos reproduzíveis: `docs/evidence/S008/README.md`.
  Próxima ação: conferir CI do HEAD, rodar/regredir comparação de 600+600
  pixels/RAM, corrigir divergências antes de revisão/merge.
