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
