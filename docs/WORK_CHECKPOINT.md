# Checkpoint operacional — 2026-10-07 UTC (06/10 no Brasil)

## Ordem permanente de continuidade (atualizada na S002)

Todo novo chat/Work começa em [START_HERE.md](../START_HERE.md) e deve cumprir
[CONTINUITY_PROTOCOL.md](CONTINUITY_PROTOCOL.md). A cada avanço, registrar
resultados e tentativas. Antes da resposta final, atualizar este checkpoint,
[SESSION_LOG.md](SESSION_LOG.md) e [NEXT_STEPS.md](NEXT_STEPS.md), publicar
commits e confirmar HEAD remoto. O sucessor deve preservar e repetir o processo.

A sessão S002 só alterou documentação. Os resultados técnicos abaixo são da S001.
A tarefa ativa e os próximos comandos copiáveis estão em NEXT_STEPS.md.

## Retomar daqui

Repositório: PedroMarioaBros/SMNEStoX360, branch main.
Implementação desta sessão: commit `13e6249255c7f3a9b4dd1921bad360db4e3f9035`.
O commit 4c438b85fdb27b4deaf6cfec319e66ad2586b50c publicou a primeira versão
deste checkpoint, as evidências e o CI sintético.

**Marco: execução do PRG canônico no host, entrada NMI real em $8082,
quadros de título/fase e controles roteirizados. Ainda NÃO é port integral
validado nem jogo validado no Xbox 360.**

## Identidade e entrada

A ROM foi encontrada nos arquivos do proprietário, sem baixar ROM de terceiros.
Os hashes integrais ROM/PRG/CHR coincidiram com
[SMB_V026_CANONICAL_ROM.md](SMB_V026_CANONICAL_ROM.md).
Nenhuma nova ROM, PRG ou imagem de assets foi adicionada ao Git.
Nesta sessão o input local estava em `../rom/SMB_v026.nes`; não presumir que
esse caminho sobreviverá. Se ausente, localizar SMB_v026.nes nos arquivos do
proprietário e verificar hashes, ou pedir ao proprietário o arquivo.

## Auditoria inicial e progressão medida

HEAD recebido: `b8de2f29f36316f386a3ff4c54f91a8c76c08b0f`.

- NROM, RESET e timing PPU originais: compilados e executados com sucesso.
- Runner original: NÃO compilava; strings continham newlines literais dentro das
  aspas. Corrigido antes de qualquer medição.
- Após apenas corrigir o runner: 29.051 instruções, 94.210 ciclos CPU,
  frame 3, scanline 42, dot 261. Opcode bloqueador $99 em $8227; PC após fetch
  $8228. 24 opcodes encontrados, incluindo o bloqueador.
- CPU expandida, antes do NMI: 1.000.000 instruções, 3.006.944 ciclos,
  frame 100, scanline 253, dot 338, PC $8057; 30 opcodes. Era um laço de espera.
- NMI integrado: primeiro destino $8082 no frame 3; contador CPU 116.744
  **após** os 7 ciclos de entrada. Primeiro milhão: 27 NMIs, 84 opcodes;
  a espera seguinte dependia do sprite zero.
- Registradores e DMA: avançaram a inicialização, mas não resolveram sozinhos
  a espera por sprite zero.
- Compositor funcional: sprite zero deriva da sobreposição opaca de pixels
  CHR/background/sprite, com clipping. Nenhum bit de hit é forçado por um
  endereço ou condição específica do SMB.

## Resultados finais reproduzíveis (core 13e6249)

| Cenário | Instruções | Ciclos CPU | Frame PPU | Linha/dot | PC | Opcodes distintos | NMIs |
|---|---:|---:|---:|---|---|---:|---:|
| Sem entrada | 1.000.000 | 3.067.255 | 102 | 260/221 | $813D | 85 | 98 |
| Start + direita/pulo | 5.000.000 | 15.390.320 | 516 | 206/242 | $8057 | 99 | 510 |
| Mesmo roteiro, execução prolongada | 50.000.000 | 154.288.271 | 5.180 | 214/279 | $8057 | 100 | 5.174 |

Nenhum opcode bloqueou esses três cenários. Todos terminaram no limite de
instruções solicitado. PC $8057 é o laço entre NMIs, não evidência isolada
de travamento. O cenário longo registrou 4.997 eventos de sprite-zero hit.
Os números de frame são contadores zero-based, não confirmação de fidelidade
de todos os quadros.

Logs completos, incluindo hits por opcode e hashes de quadro:
- [Título](evidence/2026-10-07/title.txt)
- [Start/direita](evidence/2026-10-07/start-right.txt)
- [Execução prolongada](evidence/2026-10-07/long-run.txt)

O cenário de 5 milhões foi repetido: logs e 61.440 bytes de índices do quadro
foram idênticos. Uma execução com ASan/UBSan também gerou o mesmo log e quadro.
Isso demonstra repetibilidade desse cenário, NÃO equivalência com o NES.
O cenário de 50 milhões foi medido uma vez.

## Implementado e testado

- CPU: todos os 151 opcodes oficiais NMOS 6502 com aritmética Ricoh 2A03;
  page penalties, indireção com wrap de zero-page, JMP indirect page-wrap,
  stack, BRK/RTI, IRQ e NMI; unknown opcodes param.
- Operações RMW fazem write do valor original antes do modificado.
- ADC/SBC: 524.288 combinações de operandos/carry/flag D passaram.
- ASL/ROL/LSR/ROR: 10.240 casos de valor/carry/formas de endereço passaram.
- Testes direcionados: RESET, branches/page crossing, zero-page wrap,
  endereçamento indexado, JSR/RTS, stack wrap, BRK/RTI, IRQ, NMI, JMP bug,
  PPUSTATUS, arestas NMI e parada persistente em opcode não oficial.
- PPUCTRL/MASK/STATUS, OAMADDR/DATA, SCROLL/ADDR/DATA, latches v/t/fine-X,
  mirroring vertical, palette mirroring e masking de seis bits, CHR read-only,
  PPUDATA buffer e incremento 1/32.
- OAM DMA copia 256 bytes com wrap e acrescenta 513/514 ciclos.
- Controller: strobe alto lê A atual; serialização dos oito bits e retorno 1
  após oitavo bit.
- Renderização inicial: background, atributos, sprites, prioridade, flips,
  sprites 8x8/8x16, clipping, sprite zero e incremento vertical.
  Testes de render cobrem casos selecionados, não todas essas combinações.
- Compilação C99 com -Wall -Wextra -Werror; sete executáveis de testes passaram
  em configuração normal e ASan/UBSan.
- Wrapper recusou uma cópia deliberadamente alterada da ROM antes da execução.

**151 implementados não significa 151 instruções independentemente certificadas
em todos os modos e estados.** Testes externos de conformidade continuam necessários.

## Falhas observadas e limitações

1. PPU NÃO é cycle-perfect. Compositor faz acesso direto aos tiles, não pipeline
   de fetch/shift registers; snapshot por linha simplifica scroll, especialmente
   alterações durante a linha. Não modela dummy reads do CPU, microciclos de
   interrupções, atraso de polling IRQ após CLI/SEI/PLP ou janelas de supressão NMI.
2. Imagem de título foi inspecionada e contém MUNDO/TEMPO/MARIO/LUIGI.
   Imagem após Start/movimento mostra cenário de fase, sprites e Mario,
   mas o HUD não aparece no quadro final de 5 milhões. **Pendência gráfica
   explícita; não declarar esse quadro correto.** Diagnóstico inicial no frame
   500 encontrou scroll zero no topo e sprite-zero hit por volta da linha 31,
   seguido da mudança de scroll; falta comparar VRAM/pixels com referência.
3. Render não modela o bug de overflow de sprites, pipeline de avaliação,
   efeitos de escritas durante rendering, odd-frame skipped dot, paleta analógica
   ou color emphasis. PPUSTATUS races, reset/power-up e CPU open bus incompletos.
4. DMA é cópia instantânea + stall agregado, sem arbitragem ciclo a ciclo.
5. APU permanece placeholder. Nenhum áudio correto produzido. $6000-$7FFF
   ainda retorna zero; não foi comprovado hardware de expansão adicional.
6. Sem validação diferencial por quadro/estado contra execução NES de referência;
   alterações específicas da ROM não foram auditadas integralmente.
7. LeakSanitizer falhou no ambiente sob ptrace (/proc inacessível). Repetido com
   ASAN_OPTIONS=detect_leaks=0: AddressSanitizer e UBSan passaram. Não afirmar
   que leak detection foi validado.
8. Push por git HTTPS local não tinha credencial. Publicação foi feita pela
   conexão GitHub autorizada, com expected HEAD, e árvore remota conferida.
9. CI do commit 4c438b85fdb27b4deaf6cfec319e66ad2586b50c foi consultado e
   concluiu success: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37561513602.
   As medições da ROM acima são locais. Não transferir esse sucesso
   automaticamente para commits futuros.

## Reproduzir

```sh
sh tests/run_host_tests.sh
python3 tools/run_canonical.py /caminho/SMB_v026.nes
python3 tools/run_canonical.py /caminho/SMB_v026.nes \
  --instructions 5000000 --input tests/canonical_start_right.input \
  --frame build/gameplay.bin
python3 tools/run_canonical.py /caminho/SMB_v026.nes \
  --instructions 50000000 --input tests/canonical_start_right.input \
  --frame build/long-run.bin
```

Não mudar a ROM para fazer o runtime passar. O runner direto só verifica header
e tamanho: use o wrapper que verifica SHA-256 de ROM/PRG/CHR.
Quadro exportado: 256x240 índices NES, sem RGB; hash independente de paleta visual.
Roteiro `tests/canonical_start_right.input`: Start nos frames 100–101,
direita+A em 220–269, direita em 270–359, soltar depois.

## Próximo bloqueio e sequência

Prioridade imediata: comparar quadros/VRAM/scroll com uma referência usando
EXATAMENTE a mesma ROM; investigar HUD ausente e ajustar o pipeline PPU sem
patch de gameplay. Ampliar conformidade CPU com testes independentes.
Depois: entrada interativa host, áudio/APU e backend Xbox mínimo seguro.

Xbox: **nenhum novo XEX produzido ou testado nesta sessão.** Build histórico #16
teve Fatal Crash imediato. Estado de boot-test.xex não foi revalidado aqui.
Não ligar core novo ao vídeo antigo com endereços hardcoded.
A futura entrega deve embutir PRG/CHR canônicos no default.xex, sem ROM externa,
mas o host usa arquivo privado para validação nesta etapa.

## Referências técnicas consultadas

Somente semântica de hardware, nunca substituição do gameplay:
- https://www.nesdev.org/wiki/CPU_interrupts
- https://www.nesdev.org/wiki/PPU_programmer_reference
- https://www.nesdev.org/wiki/PPU_memory_map

Antes de finalizar a próxima sessão, atualizar este checkpoint com medidas
novas e limites reais e confirmar commits no GitHub.
