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

### Fechamento S008 — integração e verificação remota (09/10/2026)

- GitHub Actions do commit de código `af36b563ee19543acd826bd89a768748e6ae0b5e`:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37888390752,
  status **success**. Quatro jobs passaram: host normal, host sanitizado,
  frontend SDL e comparação diferencial com referência (600 idle e 600
  scripted, pixels e RAM). Este resultado substitui a pendência de CI
  registrada no início da sessão; não equivale a teste de áudio audível.
- Pull request #3:
  https://github.com/PedroMarioaBros/SMNEStoX360/pull/3,
  integrado à branch main em squash commit
  `f0af8dbf7236571760e03e6e36938ae61c44edab`.
  HEAD remoto conferido após merge.
- O código da S008 já faz parte da main. Próxima tarefa é pulse 2 e seus
  testes de forma independente; documento operacional atualizado em
  `docs/NEXT_STEPS.md`. Sem XEX novo, nenhum teste físico no Xbox.

## S009 — 09/10/2026, pulse 2 da APU e testes de regressão

- HEAD de entrada: `248c8758f5421c7ed1c7bbf230b8e509c79f5d83`.
  Trabalho em `work/s009-apu-pulse2-20261009`. Finalizado por
  [PR #4](https://github.com/PedroMarioaBros/SMNEStoX360/pull/4),
  merge/squash `652f4e2d7f71d26e6a4eb6f65d438aadd6f1c957`.
- Rom de 40976 bytes conferida localmente por SHA-256, incluindo PRG/CHR;
  valores sem mudança em SMB_V026_CANONICAL_ROM.md. Nenhum byte alterado.
- Generalizado `apu.[ch]`: canais pulse 1/2 independentes, duty, timer,
  envelope, comprimento, sweep e mute. Endereços $4004–$4007 adicionados,
  $4015 bit1 de status/enable, saída DAC do pulse 2. Sweep negativo
  difere entre canais: pulse1 subtrai change+1 e pulse2 subtrai change.
  Sem áudio PCM/mixer; não há nova execução no Xbox.
- Criado `tests/test_apu_pulse2.c`, incluindo quatro testes de
  independência de frequência/fase, reload/status, sweep, envelope
  e comprimento. Inserido em `tests/run_host_tests.sh` (normal/san).
  `test_ppu_registers.c` passou a verificar writes nos dois canais,
  $4015, controle 2 por $4017 e alinhamento de clocks.
- Desenvolvimento das fixtures: duas falhas reais de teste antes do
  sucesso; comprimento estava indexado como 254 em vez de 2 e saída
  no instante de duty index 7 foi presumida não nula. Corrigidos valores
  da fixture; nenhum patch artificial de gameplay/core.
- Localmente: ambos os binários de teste de APU passaram com
  `-std=c99 -Wall -Wextra -Werror` e repetidos com ASan/UBSan
  (`detect_leaks=0`). Clone GitHub local bloqueado por DNS; testes
  completos/ROM no CI, não fingir execução local completa.
- CI no commit de código `3f877fd7d3802df35897d303105b7980d56ecd7a`:
  [workflow 37889071037](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889071037),
  **success**, quatro jobs (host normal, host sanitizado, frontend SDL,
  referência diferencial). binjnes idle pixels/RAM = 600/600 e com
  controles pixels/RAM = 600/600. Testes do pulse 1 e pulse 2 PASS
  constam dos logs host. Não extrapolar para timing/som real.
- Evidências: `docs/evidence/S009/README.md`, workflow e diff da PR.
  Alvo final continua sem XEX canônico testado no Xbox. Próxima tarefa:
  triangle ($4008/$400A/$400B, linear/length/timer) com novos testes,
  regressão e evidências; comandos em `docs/NEXT_STEPS.md`.

## S010 — 09/10/2026, terceiro canal APU (triangle) e validação

- HEAD de entrada main: `030813421ed1aff21e5b6289392a48c9f2c17d1c`.
  Branch `work/s010-apu-triangle-20261009`; PR #5:
  https://github.com/PedroMarioaBros/SMNEStoX360/pull/5 .
- ROM do proprietário preservada, hashes ROM/PRG/CHR conferidos
  localmente no anexo; CI verificou assets no checkout GitHub.
- `src/canonical/apu.[ch]` estendido com triangle: $4008 linear,
  $400A/$400B período e comprimento, $4015 bit2, sequência 32 níveis,
  timer a CPU/1, linear no quarter frame e length no half frame.
  Output DAC retido durante gate; $400B não reinicia fase.
  Pulse1/2 mantidos, sem outra ROM ou gameplay alternativo.
- Adicionado `tests/test_apu_triangle.c` cobrindo sequência inteira,
  timer, gates, comprimento/halt, reload, status e separação pulse.
  `tests/test_ppu_registers.c` reforça independência dos registradores
  triangle e leitura de controle 2 em $4017.
  Workflow inclui `tests/test_apu*` na lista de mudanças que disparam CI.
- Falhas reais e corrigidas: workflows
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889678133
  e https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889702475
  falharam no teste `test_gates_hold_last_level`.
  O teste confundia o momento da escrita em $4008 com a efetiva
  parada após o próximo quarter frame. Corrigido no commit
  `95657fe2732472b72c6c4c6ff08de0392faff49a`;
  o runtime não foi manipulado para esconder a falha.
- CI do último commit de código/infra
  `73ca0151d153448e8d13cca07bdf395fd93618cd`:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37889762682
  **success**, quatro jobs: host normal, ASan/UBSan,
  binjnes reference e frontend SDL. 10 suites normais/san PASS.
  Comparador de ROM: idle 600/600 pixels e RAM; roteiro
  600/600 pixels e RAM, sem exclusão de memória.
  SDL readback 61.440 pixels PASS, captura 599 bate binjnes.
- O clone local via git HTTPS falhou por DNS; testes completos
  executados no GitHub Actions (não localmente). Apenas fingerprint
  da ROM do proprietário foi computado no ambiente local.
- Evidências e limites em `docs/evidence/S010/README.md`.
  Ainda não há áudio audível/PCM, nem noise/DMC/mixer/IRQ integrada,
  nem XEX canônico validado em hardware. A arquitetura continua
  CPU 6502 por software, sem tradução PowerPC nativa.
- Próximo passo documentado: noise com LFSR e envelope/length
  em `docs/NEXT_STEPS.md`; testar e registrar nova sessão.

### Fechamento S010 — publicação do código (09/10/2026)

- PR #5 foi integrada por squash na `main` no commit
  `dee528eabe2eb0f0c349d265d3ec256ba4ec91cf`.
- Resultado do CI é do commit de código `73ca0151d153448e8d13cca07bdf395fd93618cd`;
  commits posteriores até a integração foram somente documentação.
  Não transferir uma aprovação de CI entre versões distintas de código.
- Próxima etapa formal S011: canal noise, com comandos em `docs/NEXT_STEPS.md`.

## S011 — 09/10/2026, noise da APU 2A03

- HEAD de entrada: `c7e4cc7c7954d6fad528dd5a23a7a5b0c89fbf74`.
  Branch: `work/s011-apu-noise-20261009`. Continua o protocolo
  AGENTS.md/CONTINUITY_PROTOCOL.md e preserva a ROM do proprietário.
- ROM local SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
  PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
  CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
  Git HTTPS local ainda falha por DNS; conexão GitHub autorizada usada.
- Novo canal noise `smb360_apu_noise`, endereços $400C/$400E/$400F,
  tabelas NTSC de 16 períodos em CPU/2, LFSR 15 bits (seed1),
  feedback bits 0/1 ou 0/6 conforme mode, timer, length counter e
  envelope quarter-frame com loop. $4015 bit3 controla enable e
  expõe length. DAC digital 0..15 ainda sem reprodução audível.
- Criado `tests/test_apu_noise.c` com cinco funções de teste (todos
  os 16 períodos, gating e taps, envelope/length/IRQ flag, looping
  e independência pulse/triangle). `run_host_tests.sh` inclui 11
  suites; `test_ppu_registers.c` verifica acesso ao barramento,
  controlador 2 e separação das escritas APU.
- Falha observada no workflow
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890305871:
  `test_ppu_registers.c:88` assumiu noise LFSR=1 após inicializar
  `machine`. O RESET da CPU já consumiu 7 ciclos que avançaram
  o LFSR. Corrigida a fixture para comparar estado antes/depois de
  escritas nos registradores, sem alterar o core ou hashes canônicos.
  Commit da correção `19b5e2977aef8d98bea0c84c044d13318685b2a5`.
- GitHub Actions do código corrigido:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890367890
  **success**, quatro jobs verdes (host normal, ASan/UBSan,
  frontend SDL e referência binjnes). Os logs confirmam
  `test_apu_noise: PASS`, `test_ppu_registers: PASS`,
  verificação dos arquivos canônicos e 600/600 pixels e RAM
  nos dois roteiros. SDL readback 61.440 pixels PASS.
  Isso não prova fidelidade do sinal de áudio.
- Nenhum XEX canônico produzido/testado nesta etapa, nenhuma
  tradução nativa PPC ou saída PCM. Próximo passo: DMC, DMA/IRQ e
  validação de áudio, com instruções em `docs/NEXT_STEPS.md`.
  Evidência detalhada: `docs/evidence/S011/README.md`.

### Fechamento S011 — publicação remota (09/10/2026)

- Pull Request #6: https://github.com/PedroMarioaBros/SMNEStoX360/pull/6
  integrada por squash na main, commit `d8a9c0a60a2aa46b92276d1ece0c0558204dc2e3`.
- Código validado no commit `19b5e2977aef8d98bea0c84c044d13318685b2a5`:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890367890 .
  Todos os quatro jobs concluíram success. Commits documentais seguintes
  não modificaram o core. Não extrapolar para hardware/áudio real.
- Próxima tarefa S012 é DMC com DMA/IRQ e testes; comandos em NEXT_STEPS.md.

- Confirmação pós-merge: `canonical-host.yml` da main no commit
  `d8a9c0a60a2aa46b92276d1ece0c0558204dc2e3` concluiu
  **success** nos quatro jobs: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890597158 .
  Este sucesso é dos testes host, não prova execução funcional no console.

## S012 — 09/10/2026, DMC e primeiro DMA/IRQ da APU

- Entrada main `9293f007b29060efa2c7349f2ff272eeca04d60f`,
  branch `work/s012-apu-dmc-20261009`. ROM canônica 40976 bytes,
  SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
  PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
  CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
  Nenhum asset modificado.
- `apu.[ch]`: DMC de 7 bits, $4010-$4013, $4015 bit4
  (active), bit7 (IRQ); sample buffer independente do
  shifter, 16 períodos CPU NTSC, bits LSB-first, output
  DAC com ±2 e saturação 0..127, endereço $C000+n*64,
  tamanho 16*n+1, $FFFF→$8000, sample loop e IRQ no
  último byte lido. Frame IRQ já existia e agora pode
  ser encaminhada ao 6502 juntamente com DMC IRQ.
- `machine.c` passou a servir solicitações de DMA
  DMC do barramento NROM original, com leitura apenas
  do PRG autorizado e stall CPU fixo estimado de 4
  ciclos, atualizando clocks PPU/APU/CPU; IRQ
  atendida em limite de instrução usando o
  `smb360_cpu6502_irq` existente. **Microciclos,
  custo 1–4, colisão OAM e stalls de leitura
  não estão precisos** e devem ser refinados.
- Novo `tests/test_apu_dmc.c` com cinco cenários
  de status, interrupção, reg/IRQ, 16 velocidades,
  loop/wrap, buffers, saturação e prova sintética
  de leitura do PRG no barramento/serviço de IRQ
  e alinhamento CPU/APU após stall. Runner agora
  executa 12 suítes C99 em normal e san.
- CI último código `e1a68b1d7ded75cb968556fe425b5fdcb584e78d`:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890958510
  **success**, quatro jobs. 12/12 suites normal e
  ASan/UBSan PASS, incluindo `test_apu_dmc`,
  `test_ppu_registers`. Verificador dos assets PASS.
  Binjnes: idle 600/600 pixels + 600/600 RAM,
  input 600/600 pixels + 600/600 RAM;
  SDL dummy 61.440 pixels PASS, captura 599
  `fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc`.
  Não há comparador de áudio; os cinco canais
  possuem apenas níveis digitais, não reprodução PCM.
- Documentação: `docs/evidence/S012/README.md`.
  Sem nova validação no Xbox 360, sem XEX canônico
  funcional, sem tradução estática PPC do PRG.
- Próxima tarefa S013: mixer não linear, amostragem
  determinística PCM, saída SDL host e teste
  de waveform independente; após isso refinar
  DMA/IRQ ciclo a ciclo e backend Xbox.
  Comandos no `docs/NEXT_STEPS.md`.

### Fechamento S012 — integração remota

- PR #7 https://github.com/PedroMarioaBros/SMNEStoX360/pull/7
  integrada por squash na main: `af8f34487830f02b19170fb9b5fc107eaf360b33`.
- CI verde do último commit de código `e1a68b1d7ded75cb968556fe425b5fdcb584e78d`
  (run https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890958510 ).
  Commits documentais seguintes não alteraram runtime.
  Conferir CI pós-merge separadamente antes de afirmar aprovação dele.
- Próxima sessão S013: mixer + resampling/PCM/SDL e comparação sonora.

- Confirmação adicional: host CI da main no squash
  `af8f34487830f02b19170fb9b5fc107eaf360b33`
  concluiu `success`, quatro jobs
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37891237452 .
- Diagnóstico de falha preexistente do workflow Xbox legado:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37891237477 .
  SynthXEX chegou a criar `default.xex`, mas a próxima etapa
  exige `boot-test.xex` não produzido por `scripts/build-smb-xex.sh`.
  Também usa `nathsou/smb` antigo, não runtime canônico da ROM.
  Issue https://github.com/PedroMarioaBros/SMNEStoX360/issues/8
  aberta para correção preservando distinção entre código histórico
  e port da ROM. O mesmo defeito existia em S011
  (run 37890597151); não associar a uma regressão DMC.

## S013 — 09/10/2026, áudio PCM 48 kHz e arquivo WAV com ROM canônica

- Entrada main `6e158f17572e17bfa85a8543710a47fdd73497d1`;
  branch `work/s013-audio-pcm-20261009`. ROM original
  SMB_v026.nes, 40976 bytes, SHA-256
  `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
  PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
  CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
  Assets não alterados.
- Adicionado `src/canonical/audio_pcm.[ch]`: mistura
  NES não linear de pulses e triangle/noise/DMC;
  phase accumulator CPU NTSC 1.789.773/48.000,
  samples mono signed 16 bits, filtros aproximados
  90 Hz, 440 Hz e 14 kHz, saturação protegida.
  Saídas são capturadas **por instrução**, não
  nos microciclos da CPU, limite de fidelidade registrado.
- `src/host/main.c`: dispositivo áudio SDL opcional
  com `SDL_QueueAudio` e buffer com limite de latência,
  não bloqueando render se indisponível; opção `--wav`
  gera RIFF/WAVE de 16 bits little-endian.
- `tests/test_audio_pcm.c` cobre mixer não linear,
  exata contagem 48.000/1.789.773, determinismo por
  divisão dos ciclos, filtros e saturação;
  13 suítes C99 normal e ASan/UBSan no runner.
  `tests/test_host_audio.py` captura a ROM original
  600 frames duas vezes, verifica WAV/sha/valores,
  entrega `build/host/canonical-600frames.wav` ao
  artifact GitHub `canonical-s013-audio-wav`.
- CI `5666e3437e7073e781862991fa9d0b5fb4bd61ed`,
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892058096 :
  **completed/success**, quatro jobs: host normal,
  ASan/UBSan, reference binjnes, frontend SDL.
  13 suítes host passaram normal/san;
  gráficos/RAM idle 600/600 e input 600/600;
  SDL dummy readback 61.440 pixels PASS.
  WAV ROM: 479.146 samples, min −5.889,
  max +6.328, 159.921 não-zero e SHA-256
  `3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`,
  dois arquivos byte-identicos.
  Artefato #11598404861 confirmado na execução CI.
- Falha intermediária nos workflows 37891903477,
  37891908427, 37891911591: nova fixture PCM
  assumia silêncio total no reset, porém triangle
  DAC mantém nível inicial 15. Corrigido o teste,
  sem manipular runtime ou ROM. Workflow ganhou
  gatilho `tests/test_audio*`.
- Não foi verificada fidelidade com áudio binjnes
  ou ouvido com dispositivo real: SDL CI é dummy.
  Sem novo XEX canônico validado, IRQ/DMC DMA ainda
  aproximado, runtime 6502 ainda interpretado.
- Próxima tarefa S014: boot e build do core canônico
  no Xbox via OpenXeChain e PRG/CHR originais,
  sem reutilizar gameplay `nathsou` histórico;
  depois validar waveform com referência.
  Issue Xbox #8 permanece:
  https://github.com/PedroMarioaBros/SMNEStoX360/issues/8 .
  Instruções: `docs/NEXT_STEPS.md`;
  evidências: `docs/evidence/S013/README.md`.

### Encerramento e merge S013

- PR #9 https://github.com/PedroMarioaBros/SMNEStoX360/pull/9
  integrada à main por squash: `64a7edaa76c14b069da283023bab38d1b64e1173`.
- Último código/CI testado na branch `5666e3437e7073e781862991fa9d0b5fb4bd61ed`, run
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892058096
  (quatro jobs verdes). Após esse commit houve somente documentação.
- Próxima S014: XEX próprio de `src/canonical`, não build legado nathsou.

- Validação pós-merge: run
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37892331535
  (`main`, commit `64a7edaa76c14b069da283023bab38d1b64e1173`)
  **completed/success** nos quatro jobs. Captura WAV arquivada
  em `canonical-s013-audio-wav`, id 11599295620.
- O workflow Xbox legado `37892331507` reportou `failure`
  preexistente, issue #8, não confundir com CI do runtime canônico.

## S014 — 09/10/2026, primeiro XEX do core canônico

- HEAD `main` de entrada:
  `cdcda47f9952afa92b09228a4da100a463ef97c0`.
  Branch `work/s014-canonical-xex-20261009`.
  ROM/PRG/CHR originais mantidos, hashes:
  ROM `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
  PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
  CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- Criado `src/platform/canonical_xex/main.c` com
  instância real `smb360_machine`, inicialização com
  PRG/CHR embutidos, loop de 120 quadros/timeout,
  diagnóstico de PC/ciclos/FNV de pixels por stdout,
  sem framebuffer MMIO, áudio ou input legado.
  `embedded_rom.S` incbin incorpora os 40.976
  bytes exatos da ROM canônica.
- `scripts/build-canonical-xex.sh` roda hashes obrigatórios
  e compila todos os `src/canonical/*.c` com
  OpenXeChain PowerPC para `canonical.exe` e XEX2
  `default.xex`, manifest e sums. Workflow isolado
  `.github/workflows/canonical-xex.yml` usa cache
  do compilador pinado, sem dependência
  `nathsou/smb`, e publica artifact por 30 dias.
- Primeira tentativa CI `37958041952` falhou com
  `main.c:2:60: '/*' within block comment [-Werror,-Wcomment]`.
  Corrigido comentário; nenhuma mudança em gameplay
  ou testes; commit `75eb79a35b6c60ff8ed4d4e17a88404af89c2325`.
- CI XEX nesse commit: run
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958120419
  **completed/success**, LLVM target `ppc32-unknown-xbox360`,
  `SynthXEX> XEX built`, magic XEX2, checksums e
  manifest de DIAGNÓSTICO aprovados.
  Artifact confirmado `smb360-CANONICAL-DIAGNOSTIC-xex`
  id 11630285368. Aviso real de linker:
  `/align specified without /driver; image may not run`.
  Não afirmar boot no console.
- Host CI depois do acréscimo de triggers:
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958266320,
  commit `0217fc98606e491c4458855e7f413b581474d36e`:
  **success**, quatro jobs, 13 suítes normal/san,
  pixels/RAM binjnes idle 600/600 e input 600/600;
  SDL 61.440 pixels PASS e mesmo WAV SHA256 da S013.
- XEX NOVO É HEADLESS: programa tem código para
  avançar o Mario original 120 quadros, mas o código
  ainda não foi EXECUTADO em console real, e sem
  vídeo/áudio/controles não é jogável.
  O PRG 6502 continua interpretado em software
  C compilado para PPC; não atende o objetivo de
  port sem interpretador/tradução direta PPC.
- Documentação `docs/evidence/S014/README.md`,
  checkpoint atualizado, próxima S015 registrada
  em `docs/NEXT_STEPS.md`: validar boot físico,
  vídeo seguro, input e arquitetura nativa.

### Fechamento S014 — publicação na main

- PR #10 https://github.com/PedroMarioaBros/SMNEStoX360/pull/10
  integrada por squash em `b3a0aa56d6b45be588cec0ebfd936f1cb070ce07`.
- CI do HEAD final do PR `577e14041ab7de586b62cedf4f864d132d610043`:
  Xbox canônico https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958685203
  `completed/success`, artifact `smb360-CANONICAL-DIAGNOSTIC-xex`
  #11630856127; host https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958685211
  `completed/success`, quatro jobs normal/ASan-UBSan/reference/frontend.
- Não declarar boot no Xbox nem jogo jogável. Próxima S015 registrada.

- Pós-merge S014 (commit `b3a0aa56d6b45be588cec0ebfd936f1cb070ce07`):
  novo XEX canônico https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798039
  **success**, artifact `smb360-CANONICAL-DIAGNOSTIC-xex`
  id 11629836328; host https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798074
  **success** nos quatro jobs. Histórico `Compilar SMB Xbox 360`
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798128
  continua **failure** conhecido (issue #8), separado do build canônico.
  Compilação não implica boot ou port sem interpretação da CPU.


## Organização documental — 09/10/2026 (não é S015)

- Pedido do proprietário: organizar todos os projetos Xbox 360, compilações na nuvem e sua situação real sem apagar histórico ou quebrar o jogo.
- Criado `docs/STATUS_PARA_PEDRO.md` com entrada simples, distinção canônico/legado e tarefa S015, vinculada ao [painel central](https://github.com/PedroMarioaBros/OpenXeChain-X360-Builder/blob/main/docs/PAINEL_PMCN_XBOX360.md).
- README principal passa a apontar para o resumo e distinguir XEX2 de diagnóstico de jogo executável/jogável.
- `.github/workflows/xbox360-cloud-build.yml`: renomeado para **LEGADO SMB Xbox 360 nathsou (manual, NAO OFICIAL)** e limitado ao gatilho `workflow_dispatch`, para não executar automaticamente em cada push/PR. Código legado, scripts e logs preservados.
- Motivo da falha antiga confirmado por logs da execução [37958798128](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798128): `build-smb-xex.sh` produziu `default.xex`, porém a validação exigiu também `boot-test.xex`, que o script não produz. A rota tem ainda histórico de Fatal Crash em hardware, não deve ser reparada por engano como caminho oficial.
- **Não foram modificados**: `assets/canonical/`, `src/`, `scripts/`, `canonical-xex.yml` ou qualquer arquivo de jogo. Nenhum novo XEX/boot/hardware testado nesta organização.
- Próxima tarefa técnica permanece **S015**: segurança de boot/linker, diagnóstico gráfico e controle da linha canônica, conforme `docs/NEXT_STEPS.md`. Verificação após commits: conferir links e HEAD remoto; não interpretar eventuais CI de documentação como teste físico.

## S015 — 09/10/2026, notificações XAM, controles e auditoria do XEX

- Entrada main `884c5db3defec68fbf58793fccbbc69a225584c2`
  (reorganização documental anterior sem modificar jogo).
  Branch `work/s015-observable-boot-20261009`.
  ROM `SMB_v026.nes` 40976 bytes e PRG/CHR preservados:
  hashes ROM `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`,
  PRG `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`,
  CHR `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.
- `src/canonical/controller_map.[ch]` mapeia Xbox XInput
  para 8 botões NES e cancela direções opostas. Novo
  `tests/test_controller_map.c` cobre 8 botões,
  combinações e leitura serial real de duas portas
  $4016/$4017. Runner C99 passa a 14 suítes normal/san.
- `src/platform/canonical_xex/main.c` usa
  `XamInputGetState` para ambos gamepads e
  `XNotifyQueueUI` para START/PASS/erro do diagnóstico,
  UTF-16 estático e encerramento após 120 quadros;
  evita vídeo MMIO antigo. O SDK xecorelib
  lista XamInputGetState @401 e XNotifyQueueUI @656.
  Compile/link não prova toast visível em hardware.
- `tools/verify_canonical_xex.py` valida saída
  PE PowerPC machine 0x01f2, 4 seções e presença
  da ROM canônica completa byte por byte, offset PE
  0x16c00 no build testado; também XEX2 e
  hashes SHA-256. Run de código
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37965972564
  success. Não há ROM de terceiros no executável.
- A primeira atualização de `build-canonical-xex.sh`
  alterou manifesto para
  `video_backend=XAM_NOTIFICATIONS_ONLY` mas o workflow
  ainda verificava `NONE_SAFE_HEADLESS`, causando
  falha posterior na checagem (run 37966195621).
  Compilação e verificação PE tinham passado;
  corrigida a verificação do CI.
- `docs/TESTE_FISICO_S015.md` oferece instruções
  objetivas para o proprietário testar somente uma
  cópia isolada no console e relatar START/PASS,
  Fatal Crash, travamento ou retorno. Esse documento
  acompanha artifact `smb360-CANONICAL-DIAGNOSTIC-xex`.
- `lld-link` ainda avisa que `/align` sem `/driver`
  pode impedir execução; não há teste físico que
  relacione o aviso a crash. Não alterar flags às cegas.
- Limites: sem gameplay exibido no Xbox, sem áudio
  hardware, sem teste físico e sem tradução 6502→PPC
  estática. XEX é **diagnóstico não jogável**;
  port nativo sem interpretador não atingido.
- Próxima ação: após merge e CI verde da main,
  notificar o proprietário de que o teste S015 está
  pronto, com link Actions e guia; em paralelo S016
  gráfico Xbox seguro e proposta de tradução PPC.
  Evidência em `docs/evidence/S015/README.md`.

### Fechamento S015 — integração no GitHub

- PR #11 https://github.com/PedroMarioaBros/SMNEStoX360/pull/11
  integrada na main por squash
  `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`.
- Commit final do código na branch
  `94a01f0af3798984d5ff88d16800690db10a64fc`:
  XEX canônico CI `37966684272` `success`;
  host CI `37966684264` `success` nos quatro jobs.
  XEX artifact id `11633472546`, contendo guia físico.
- CI pós-merge está em runs `37966878615` (XEX)
  e `37966878630` (host); conferir resultado
  dos dois antes de solicitar teste físico.

## S016 — 12/10/2026: S015 Fatal Crash real, isolamento de boot

- HEAD de entrada `main`:
  `2a7a2f6af127adbbb8a025b801d2612fdba55561`.
  Branch `work/s016-isolate-fatal-crash-20261012`.
- **Hardware:** o proprietário executou `default.xex`
  extraído de S015 e relatou **Fatal Crash imediatamente**,
  com necessidade de reiniciar o Xbox 360. Arquivos no ZIP:
  `BUILD_MANIFEST.txt`, `canonical.exe`,
  `default.xex`, `SHA256SUMS.txt` e
  `TESTE_XBOX_S015.md`. Nenhuma mensagem/PC do crash
  disponibilizado. Estado correto S015:
  `HARDWARE_TESTED=FAIL_FATAL_CRASH`, não "não testado".
  A observação foi publicada na
  https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .
  S015 não deve ser reexecutado.
- XEX identificado: commit S015
  `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`,
  CI compilação https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37966878615 ,
  artifact #11634357018, SHA256 do `default.xex`
  `48a9071765d3b9db8cfe9cfb91423b5f437ce990d262e380cd7bdd01dea7d077`
  (528384 B). Build verde não significa boot.
- Criados `boot_kernel_only.c` e `boot_canonical_no_xam.c`:
  diagnósticos separados (sem ROM/gameplay e com ROM
  `SMB_v026.nes` original/120 quadros respectivamente),
  sem funções XAM diretas do S015, sem printf e sem
  framebuffer MMIO. Ambiguidades de boot reduzidas
  com duas etapas de teste físico *condicional*.
- Novo `scripts/build-xex-boot-isolation.sh` e workflow
  `xex-boot-isolation.yml` produzem e verificam
  dois PE PowerPC/XEX2 com manifestos e SHA,
  ROM íntegra na variante 02 e ausente na 01.
- PRIMEIRO CI `38204175107` falhou por assertiva
  incorreta que exigia zero `xam.xex`. Artefatos PE
  foram inspecionados: **até o kernel-only importa
  `xam.xex` e `XamLoaderLaunchTitle` pelo CRT**.
  Não é possível afirmar ausência de XAM; somente
  ausência de `XNotifyQueueUI`/`XamInputGetState`
  explicitamente chamados. Evidência e hipótese
  atualizadas, sem mascarar o CRT.
- CI `38204354807`: ambos compilaram, PE e ROM
  PASS, mas a etapa de manifesto teve `grep`
  malformado. Edições subsequentes produziram
  YAML duplicado acidentalmente; refeito com
  verificação de igualdade byte-a-byte e reproduzido.
- CI **verde** (workflow isolado):
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204528510 ,
  commit `c0e430469bd45ac567d78d8279bda37d42258e8d`,
  `completed/success`. Novo artifact S016 distinto do
  default S015; ainda sem teste físico no Xbox.
- Instruções e avisos preservados em
  `docs/TESTE_FISICO_S016.md`,
  `docs/evidence/S016/README.md`,
  `START_HERE.md`, checkpoint e NEXT_STEPS.
  Documento S015 marcado historicamente como
  **NÃO EXECUTAR**. Não altera ROM/PRG/CHR.
- Próximo: testar a hipótese de startup/CRT/ABI
  e importação/synthxex; somente solicitar a
  etapa 01 do **novo** S016 após CI main verde
  e aceitação do proprietário, etapa 02 apenas
  se primeira funcionar. Sem garantia de que
  novo XEX não crashe; parar no primeiro Fatal Crash.
  A CPU 6502 segue interpretada, sem PPC estático.

### S016 — publicação, PR #13 e estado físico

- PR #13 https://github.com/PedroMarioaBros/SMNEStoX360/pull/13
  merge squash `cf10bc38eb1dbf564a83438d60f07dace96766a7`.
- CI da branch `fe340af22cf40689580b774a164d475078799195`:
  isolamento https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649408
  `success`, artifact #11726049878; host 4 jobs
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649340
  `success`; XEX canônico
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38204649496
  `success`.
- CI da main após merge está nos runs
  `38204726453` (S016), `38204726543` (host) e
  `38204726479` (canônico); verificar separadamente.
- S015 continua **FATAL CRASH CONFIRMADO**, portanto nenhum
  novo XEX deve ser vendido como jogo/boot validado.
  S016 é apenas isolamento controlado **não testado em hardware**.

### Continuidade S016 — 12/10/2026: desbloqueio por software

- O proprietário esclareceu que o console **não possui RGH/JTAG físico**: usa desbloqueio **por software**, descrito como a versão mais recente. O **nome exato do patcher não foi informado**. Não converter essa informação em certeza de FreeMyXe/XeUnshackle sem confirmação.
- Consulta a https://consolemods.org/wiki/Xbox_360:Bad_Update : BadUpdate/ABadAvatar é um método plausível, normalmente temporário. FreeMyXe e XeUnshackle são alternativas de patcher/ambiente, não sinônimos do exploit inicial.
- Compatibilidade variável com XEX devkit/retail: o guia FreeMyXe de 2025 (https://github.com/FreeMyXe/FreeMyXe/wiki/Guidance-for-Homebrew-developers) menciona restrição, mas o README atual (https://github.com/FreeMyXe/FreeMyXe) afirma suporte a XEX devkit-encrypted; XeUnshackle (https://github.com/Byrom90/XeUnshackle) descreve ambos os formatos. Não assumir incompatibilidade nem converter o executável sem inspecionar a versão e cabeçalho.
- A causa do Fatal Crash S015 continua **não comprovada**; 01/02 S016 **não testados fisicamente**. Próximo passo: identificar patcher pelo nome exibido no console e então estudar imports/criptografia compatíveis. Atualizada a issue #12. Não solicitar reteste agora.

### Confirmação adicional — 12/10/2026, ABadAvatar automático

- O proprietário esclareceu inequivocamente que o método de entrada do desbloqueio por software é **ABadAvatar**, descrito como a versão mais recente que possui e com disparo automático. Não classificar mais ABadAvatar como mero candidato não confirmado.
- O README primário (https://github.com/shutterbug2000/ABadAvatar) diz expressamente que ABadAvatar não inclui XEX e precisa do XEX de pós-exploit em `BadUpdatePayload`, frequentemente FreeMyXe ou XeUnshackle. A identidade e versão desse **segundo componente** ainda são desconhecidas; FreeMyXe/XeUnshackle não devem ser atribuídos sem evidência.
- Issue de Falha Real #12 ampliada com o ambiente de software identificado e próximo diagnóstico de compatibilidade: https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 . Nenhuma mudança de código ou ROM e nenhum teste físico nesta interação. Não recomendar novo executável enquanto patcher/loader não identificado. O Fatal Crash imediato S015 segue confirmado; S016 não foi testado em hardware.

## S017 — 12/10/2026: Executável PowerPC Xbox OS sem Newlib CRT

- O proprietário solicitou prosseguir sem mais bloqueio por tipo de desbloqueio e produzir **executável real Xbox 360**. ABadAvatar é confirmado, patcher não precisa travar a engenharia, mas a compatibilidade universal não pode ser garantida sem teste físico.
- Novo código `native_crtless_probe.c`: PE PowerPC Xbox com entrada explícita `smb360_entry`, sem runtime C padrão, sem GPU/MMIO, sem ROM e com três importações exatas `ExTerminateThread`, `KeDelayExecutionThread`, `XamLoaderTerminateTitle`. Faz ~5s de espera e pede encerramento normal do título via XAM.
- Segundo código `native_crtless_canonical.c` + `native_crtless_memory.c`: executa `src/canonical/` e ROM canônica original 40976B por até 120 quadros com watchdog, usando o mesmo ponto de entrada sem CRT. Não tem imagem/áudio nem é jogo jogável; CPU 6502 segue interpretada.
- `scripts/build-crtless-xex.sh`, `tools/verify_crtless_xex.py` e `xex-crtless.yml` criados: verificação exata dos imports, Xbox PE32 PowerPC 0x01f2, XEX2, entrypoint PE/XEX, ROM canônica embutida exatamente uma vez em 02, checksums separados. Não usa código nathsou nem vídeo MMIO antigo.
- **Falha investigada no próprio SynthXEX:** CI `38206298625` retornou `Out of memory` ao converter PE canônico, embora o PE tivesse sido compilado. O mapeador de PE upstream faz `realloc(buffer, rawSize)` e trata retorno NULL como OOM até para rawSize=0; o PE CRT-less tinha `.data` BSS-only raw size 0. Variável inicializada e usada cria `.data` válida, sem mudar ROM/gameplay. Workaround e erro preservados em `docs/evidence/S017/README.md`.
- **CI dos dois XEX S017:** https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206437946 `completed/success`, PE 2048/68608 B, XEX 200704/397312 B, importações exatas, hashes. Testes host e compilação canônica da branch no commit `439ab0ce46f497a220f803cb34e3ac805e6a148e` `success`.
- **Físico:** S015 `Fatal Crash` confirmado. Ambos os novos S017 permanecem **hardware UNTESTED**. O aviso de LLD sobre `/align` permanece; XEX2 estruturalmente correto pode ainda falhar. Roteiro `docs/TESTE_FISICO_S017.md`: sonda 01 primeiro em pasta isolada, 02 apenas se 01 funcionar; interromper no primeiro crash. Não alterar NAND/firmware/exploit.

### S017 — conclusão PR #14 e entrega de arquivos ao proprietário

- PR [#14](https://github.com/PedroMarioaBros/SMNEStoX360/pull/14)
  merge squash em `40825ff3530d426b05d3e23b4ab02b8d82f12ebe`.
- S017 CI da main para XEX CRT-free: [run 38206754775](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754775)
  **completed/success**, build canônico regressão
  [run 38206754778](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206754778)
  **completed/success**. Branch código final [run 38206678772](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38206678772) **success** e artifact #11728050710.
- ZIP S017 baixado para o usuário e validado: 01 `default.xex` 200704 B SHA-256
  `e1e6eedc3722f442d02b4054247726927956e2681609b673c2f42f67cbcf5404`;
  02 `02_CANONICAL_CRTLESS/default.xex` 397312 B SHA-256
  `cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee`. ROM original exata incorporada uma vez no PE 02.
- Entregues links ZIP e arquivos individuais ao proprietário; **não equivale a console testado**. A ausência de vídeo e áudio é intencional neste diagnóstico. A CPU 6502 segue interpretada em C, não estática PPC.
- S015 permanece Fatal Crash físico. Precisa-se de retorno físico da Etapa 01 S017 antes da 02; não mexer em NAND/desbloqueio. Se 01 falhar, investigar PE/XEX/ABI/cabeçalhos do SynthXEX e não insistir com a 02.

## S018 — 13/10/2026: resultado físico S017 e diagnóstico observável

- O proprietário informou testado S017 **01 minimal**:
  tela preta e congelamento do console, sem Fatal Crash,
  dashboard original ou Aurora acessível. **FALHA.**
- S017 **02 ROM original**: tela preta sem erro visível e
  **reabertura/reinício automático do Aurora**. Não sabemos
  se rodou 120 frames, se terminou antecipadamente ou se
  houve recuperação automática. NÃO classificar como PASS.
  Issue #12 atualizada e nova issue #15 com fatos separados.
- A observação expôs impossibilidade de distinguir sucesso
  ou erro no S017 pelo sintoma. `native_crtless_status.c`
  mantém entrypoint nativo sem Newlib e ROM exata,
  adicionando notificações XAM START/ROM_INVALID/CPU_STOP/
  FRAME_TIMEOUT/120_FRAMES_PASS e contador da PPU como
  condição, 120 incrementos e watchdog por quadro.
  Não usar MMIO direto de vídeo, `printf`, dependências
  de filesystem ou nathsou. ROM SHA original intacta.
- `tools/verify_crtless_xex.py` estende com
  `--allow-notifications`, conservando modo anterior
  sem flag. Importações esperadas são 3 APIs de S017
  mais `XNotifyQueueUI`; PE PowerPC/XEX/ROM conferidos.
  Workflow S018 `xex-status.yml`, script
  `build-xex-s018-status.sh`, guia e evidência S018.
- Primeiro CI branch código S018
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38415557401
  **completed/success**. Novo pacote ainda NÃO testado
  no Xbox; aviso XAM pode ser bloqueado ou instável,
  logo ausência de notificações inconclusiva.
- O caminho de encerramento `XamLoaderTerminateTitle`
  e fallback `ExTerminateThread`+loop é suspeita
  e deve ser isolado; não afirmar que foi a causa do
  travamento S017. A CPU NES continua interpretada C99
  em PowerPC e o Mario ainda não aparece no Xbox.
  Não repetir testes S015/S017 antigos.

### S018 — merge #16, builds principais aprovados, artifact entregue

- PR [#16](https://github.com/PedroMarioaBros/SMNEStoX360/pull/16)
  squash `f94f8acd553777ea198d617ced63d924c9cde3a4`.
- CI do HEAD de implementação pós-merge:
  XEX status https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357552
  **success**, host
  https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/38416357745
  **success**, S017 regressão `38416357481` e
  canônico regressão `38416357713` **success**.
- Artifact S018 `smb360-S018-STATUS-NATIVE-XEX`
  id #11824027270, ZIP 92383 bytes; contém
  `default.xex` 397312 bytes SHA-256
  `c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`.
  Descompactado e verificado por SHA256SUMS, manifest S018
  e cabeçalho `XEX2`; entregue ao proprietário com
  link individual e ZIP em chat.
- Continuidade do hardware: S017 01 travou sem Fatal Crash,
  S017 02 relançou Aurora sem erro/sem prova de frames.
  S018 **ainda não foi testado no console**. Nenhuma
  promessa de execução universal ou Mario gráfico.

## S019 — 17/10/2026: S018 causou Fatal Crash no Xbox REAL

- **Novo relato físico literal:** proprietário executou S018:
  **"S018 resultou em Fatal Crash Detected"**.
  Resultado S018 agora `HARDWARE_TESTED_FAIL_FATAL_CRASH`,
  e não "não testado" (estado dos docs antigos).
  Sem horário de ocorrência, stack trace ou relato de
  notificações, portanto NÃO inventar "imediato" ou "nenhuma
  mensagem apareceu". O XEX S018 NÃO deve ser repetido.
  Issue [#15](https://github.com/PedroMarioaBros/SMNEStoX360/issues/15)
  atualizada.
- Histórico: S015 Fatal Crash; S017 01 tela preta/freeze;
  S017 02 tela preta/reabertura do Aurora sem comprovação
  de que 120 quadros rodaram; S018 Fatal Crash.
  CI verde S018 `38416357552` demonstrou APENAS build,
  nunca execução em hardware.
- Comparação nova feita com **artifacts efetivamente
  entregues ao dono do Xbox**:
  S017 #11728050710, estágio
  `02_CANONICAL_CRTLESS/default.xex`, SHA256
  `cef79db5efa58cc07a6e4083b7e026b5d050d1c36eab0c27c5db61e95a34edee`;
  S018 #11824027270 `default.xex`, SHA256
  `c1e5e5846e77ec1b4374720160ac597f9d16331287f91378f113d6e18732aeb0`.
  Os PEs possuem 68608 bytes, XEX2 397312 bytes,
  entry `0x82015900`, mesmo mapa de seções e
  ROM `SMB_v026.nes` íntegra exatamente uma vez.
  ROM 40976 B SHA256
  `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- **Diferencial comprovado na import table**: S017 02
  importa `ExTerminateThread`,
  `KeDelayExecutionThread`,
  `XamLoaderTerminateTitle`. S018 importa essas três
  MAIS `XNotifyQueueUI`. O S018 a chama no início
  de `smb360_entry`, antes do teste de ROM/core.
  Ordinal 656 consta em `OpenXeChain/xecorelib/xam.def`.
  Evidência em [S019](evidence/S019/README.md).
- **Hipótese, não causa confirmada:** uso direto de
  `XNotifyQueueUI` pode estar associado ao Fatal Crash,
  consistente com S015, mas também há diferenças na
  lógica de frames, imports, formato/código e pode falhar
  antes de `smb360_entry`. Não há debug trace físico.
- Decisão: não entregar nova S019 XEX e não sugerir
  reteste S018. Investigar ABI/carregador XEX/erro na
  importação ou retorno do título em ambiente offline;
  sem mexer em NAND, Aurora ou ABadAvatar. A identidade
  específica de FreeMyXe/XeUnshackle não bloqueia
  a engenharia, como solicitado pelo proprietário.
  O Mario ainda não renderiza vídeo/áudio Xbox nem é
  port estático PPC. ROM/PRG/CHR intactos.

### S019 — prevenção de reenvio acidental da S018

- Modificado `.github/workflows/xex-status.yml` para deixar de publicar
  `smb360-S018-STATUS-NATIVE-XEX` após o Fatal Crash confirmado no Xbox.
  O build estático continua como regressão, com aviso explícito
  `::warning::S018 is RETIRED`. Histórico dos artifacts preservado,
  nenhuma ROM ou código de CPU alterado e nenhum novo XEX solicitado
  ao proprietário. Dossiê `docs/evidence/S019/README.md`.


## S020 — 18/10/2026, ordem explícita de recompilação estática

- O proprietário esclareceu que o objetivo do SMNEStoX360 não é um emulador de NES nem um porte pronto de terceiros: é partir da ROM canônica SMB_v026.nes, analisar/desmontar o PRG 6502, desenvolver a tradução estática para C (ou representação equivalente compilável), compilar para PowerPC/Xenon e entregar execução nativa no Xbox 360.
- Inspecionados no HEAD remoto os arquivos README.md, START_HERE.md, AGENTS.md, docs/WORK_CHECKPOINT.md, docs/NEXT_STEPS.md, docs/STATUS_PARA_PEDRO.md, docs/SMB_V026_CANONICAL_ROM.md, docs/REPRODUCIBILITY.md e o registro S015. A ROM canônica está versionada: 40.976 bytes, SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- Confirmado que o runtime atual interpreta o PRG 6502 em C e que o XEX de diagnóstico não tem vídeo/áudio jogável; não existe comprovação de recompilação estática nem jogo jogável no Xbox. Histórico de hardware permanece: S015/S018 Fatal Crash; S017 01 congelou e S017 02 retornou ao Aurora sem prova de sucesso. Não retestar.
- Criado `docs/NATIVE_RECOMPILATION_POLICY.md` com as regras permanentes, restrições contra adotar porte SMB de terceiros como base, separação entre referência host e produto nativo, etapas de análise, critérios de conclusão e limites de testes físicos.
- Atualizados AGENTS.md, START_HERE.md, README.md, docs/NEXT_STEPS.md e docs/WORK_CHECKPOINT.md para que sucessores encontrem a ordem ao abrir o repositório. O próximo trabalho é inspeção offline do código/branches e prova de conceito incremental de tradução estática 6502→C; não gerar XEX automaticamente.
- Validação desta sessão: leitura do conteúdo remoto e escrita documental via GitHub API. Não foram executados compilação, testes host, análise binária nova, CI ou teste físico nesta sessão; não alegar tais resultados.
- Próxima ação: confirmar HEAD remoto e conteúdo dos documentos alterados; depois inspecionar os arquivos reais do tradutor/runtime e implementar a menor prova de conceito estática com teste diferencial reproduzível.


## S021 — 18/10/2026: POC-001 de tradução estática ROM→C

- Branch de trabalho criada: `work/static6502-poc-20261018`, baseada no HEAD remoto de `main`.
- Adicionado `tools/static6502_poc.py`: verificação SHA-256 e layout da ROM canônica, extração do PRG e tradução direta de seis instruções reais de $8000-$8009 para C. Não há dispatch loop/opcode interpreter no C gerado. Opcode ainda não suportado interrompe a tradução.
- Self-test integrado a `tests/run_host_tests.sh`: compila o C gerado com `-std=c99 -Wall -Wextra -Werror -pedantic`; verifica A=$10, X=$FF, S=$FF, flags e callback de escrita para $2000; testa rejeição de $AD em $800A.
- Teste executado nesta sessão numa cópia local equivalente do tradutor, usando `/mnt/data/SMB_v026.nes` com SHA-256 canônico: PASS na compilação C e asserções. O arquivo já publicado na branch ainda precisa da execução CI exata; não declarar CI aprovada até haver run associado.
- Criado `docs/evidence/STATIC_RECOMPILER_POC_001.md` com escopo, comando, resultado e limitações.
- Limite: seis instruções lineares somente. Sem recompilação integral, branches, equivalência com o core, PPU/APU nativas, build PPC/XEX ou validação Xbox. Nenhum XEX gerado e nenhum teste físico pedido.
- Próximo passo: expandir a tradução estática para blocos básicos e branches, adicionar testes por opcode e comparação diferencial, validar a branch com `python3 tools/verify_repository.py` e `sh tests/run_host_tests.sh`, depois avaliar PR.


### S021 — validação CI e integração em main

- PR [#18](https://github.com/PedroMarioaBros/SMNEStoX360/pull/18) aprovada pelo próprio CI de host e integrada por squash em `d0822da58767ec8ee078752694e6d679445089dd`.
- CI do HEAD completo da PR: [run 39183794667](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39183794667), `completed/success`, head `386774b5aa05b79a24c38a92fb4b2911f45e5965`. O run de host associado ao commit que continha o tradutor e o self-test também passou: `39183681786`.
- O resultado é somente validação host de uma POC de seis instruções; nenhum XEX ou teste Xbox foi feito. Próxima ação registrada em NEXT_STEPS: expandir para blocos básicos/branches e comparar estado contra o runtime de referência.

## Avaliação NESRecomp — 18/10/2026

- O proprietário autorizou pausar a expansão do tradutor estático próprio e
  testar a viabilidade do NESRecomp genérico com a ROM canônica exata.
- Consultada a documentação upstream: https://github.com/mstan/nesrecomp .
  Ela descreve tradução estática 6502→C, backend de ciclos, runner NES e mapper
  0 suportado. Também documenta um interpretador de fallback, que é incompatível
  com o requisito final se for executado no produto.
- Revisada a licença upstream `PolyForm Noncommercial 1.0.0`: pesquisa e hobby
  pessoal não comercial são permitidos; uso comercial não está concedido.
  Não vendorizamos nem copiamos código upstream nesta etapa.
- Criados `tools/evaluate_nesrecomp.sh`, workflow CI e
  `docs/evidence/NESRECOMP_EVALUATION.md`. O script verifica SHA-256, baixa um
  commit upstream fixo, compila o gerador e tenta gerar C em diretório temporário.
- Pin: `a39d9cdbec0ac98b1430ddb53747b5ed8f8cac87`.
- Tentativa local de `git clone https://github.com/mstan/nesrecomp.git` falhou
  por DNS: `Could not resolve host: github.com`. Não houve build nem geração
  local; o workflow Actions deverá fornecer a evidência automatizada.
- Código C gerado não é commitado nem enviado como artifact nesta fase. Não foi
  adotado o port SMB de terceiros e não foi gerado XEX nem realizado teste físico.
- A PR #19 da POC própria fica aberta e sem merge enquanto a avaliação estiver
  pendente. Próxima ação: verificar o resultado real do workflow, corrigir erros
  do harness e registrar cobertura/limitações antes de decidir a arquitetura.

- Primeira CI falhou imediatamente porque o workflow chamou um script Bash com `sh` (`set: Illegal option -o pipefail`). Corrigido o workflow para invocar `bash`; isso é falha do harness, não do NESRecomp. A avaliação precisa rodar novamente.

- CI de avaliação concluiu success: [run 39186504796](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186504796); métricas completas no [run 39186469481](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186469481).
- NESRecomp compilou no runner Linux e reconheceu a ROM canônica: 2×16 KiB PRG, mapper 0, NMI `$8082`, RESET `$8000`, IRQ `$FFF0`.
- Resultado de geração cycle-accurate: 1.305 instruções compiladas; 1 ficou para interpretador por cruzar fronteira de banco 4 KiB ou `$FFFF`; 5 arquivos C, 545.655 bytes, 14.397 linhas. Saída ficou em temp e não foi commitada.
- Conclusão: geração estática substancial para a ROM exata é viável. Ainda não é aceitável como arquitetura final enquanto existir fallback interpretado. Próxima ação: localizar o alvo que sobrou, tentar cobri-lo estaticamente, depois compilar e comparar o runner host. Sem XEX e sem teste físico.

### Próximo ciclo — diagnóstico do tail do NESRecomp (19/10/2026)

- O run mais recente da PR #20, [39186568820](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39186568820), concluiu `success` no commit de avaliação. A geração e a verificação da ROM continuam reproduzíveis.
- Inspecionada a lógica do backend cycle no pin upstream: o contador `tails` mede posições visitadas (`seen`) mas não compiladas (`!is_insn`), quando a instrução não cabe no slot de 4 KiB. O log atual não revela o endereço individual.
- A ROM canônica contém bytes `DE 42 E2` em CPU `$9FFF`; `DE` é DEC absoluto,X de 3 bytes e atravessaria o slot. Esse é o candidato mais forte para o único tail, mas ainda não é prova de que seja a entrada visitada pelo analisador. Não afirmar endereço confirmado até adicionar diagnóstico temporário ao build do CI.
- Comentário de continuidade deixado na PR #20 (comentário ID 6260695162): próxima alteração proposta imprime apenas bank/slot/endereço/opcode/comprimento para cada `seen && !is_insn`, sem guardar nem publicar C gerado. O diagnóstico deve acontecer na cópia temporária do upstream e manter o pin fixo.
- Próximo gate: confirmar o endereço com instrumentação, verificar se existe forma genérica de compilá-lo estaticamente sem manter fallback; depois compilar runner host e executar testes determinísticos. Nenhum XEX ou teste físico.


### S023 — NESRecomp tail diagnostic confirmed (19/10/2026)

- Added `tools/nesrecomp-tail-diagnostic.patch`, a minimal temporary patch applied only to the pinned upstream checkout under the CI temp directory. It prints only bank, slot, address, opcode and length for `seen && !is_insn` entries; generated C remains temporary, not uploaded/committed.
- Harness `tools/evaluate_nesrecomp.sh` now applies the patch and checks that diagnostic-record count equals NESRecomp's reported interpreter-tail count.
- First diagnostic CI attempt failed because the first patch hunk had no surrounding context; this was a harness/patch-format issue, not a NESRecomp generation failure. The patch was corrected to include context and rerun.
- Successful diagnostic run: [39207665461](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/39207665461), head `33c694c163d4586a50c84e1d9d2387ad2878e218`, status success. It reported exactly one tail: bank 7, slot 7, CPU `$FFFF`, opcode `$FF`, length 3. Count matched the compiler summary; output metrics stayed 5 C files / 545,655 bytes / 14,397 lines.
- Confirmed input context: PRG vector bytes at `$FFFA-$FFFF` are `82 80 00 80 F0 FF`; IRQ vector `$FFFE-$FFFF` equals `$FFF0`. Cycle generator seeds NMI/RESET/IRQ vectors; its decoder path from the IRQ-vector seed eventually reaches `$FFFF`, where a three-byte instruction would wrap. This strongly suggests false code discovery into vector/table bytes; it is not proof of a gameplay-reachable instruction.
- Correction to the previous log hypothesis: `$9FFF` was not the fallback; confirmed diagnostic target is `$FFFF`.
- Next gate: investigate a generic/configurable way to prevent the bogus IRQ-vector target from being decoded as code; verify the title's actual IRQ usage before disabling/excluding that path; then determine how to build a runtime with no 6502 interpreter at all, compile/run the generic runner on host, and compare deterministic states/frames to the independent reference. No XEX, no hardware test, no third-party SMB port adoption.
