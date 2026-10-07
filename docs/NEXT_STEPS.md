# Próxima ação — atualizar em todo checkpoint

Última atualização: sessão S003, 06/10/2026 (America/Sao_Paulo).
Último core verificado: 13e6249255c7f3a9b4dd1921bad360db4e3f9035.
Estado: aguardando retomada técnica; a S003 tornou os arquivos e a ROM disponíveis no GitHub.

## Tarefa ativa: investigar HUD ausente no quadro da fase

A ROM chegou a 50 milhões de instruções sem opcode bloqueador e já gera
quadros. Não reiniciar implementação da CPU ou voltar à arquitetura antiga.
O problema observado é que o quadro de 5 milhões após Start/direita não
mostra o HUD. A causa ainda não foi confirmada. A medição S004 encontrou ppu_v=$632a, ppu_t=$000a, fine-X 3 e nametable com dados de status nas linhas superiores; o quadro final continua sem HUD. A hipótese atual é composição incorreta do scroll na região fixa do status.

## Próximos comandos

No checkout atualizado, depois de ler os documentos de START_HERE.md:

```sh
git status --short
git log -3 --oneline
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
```

Esse teste estabelece o baseline da nova sessão, não um marco novo.
A ROM já está versionada; use diretamente o caminho abaixo:

```sh
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes \
  --instructions 5000000 \
  --input tests/canonical_start_right.input \
  --frame build/hud-baseline.bin
```

Resultado esperado para core inalterado: 5.000.000 instruções; 15.390.320 ciclos;
frame 516; linha/dot 206/242; 510 NMIs; 99 opcodes distintos; sem opcode bloqueador.
SHA-256 do quadro:
`f8f3847872da7e7768e9244513f789240f2eabdb64404a777a066948009659e5`.

Depois da reprodução, entrar no diagnóstico — não ficar repetindo o baseline:

```sh
rg -n 'line_v|ppu_v|ppu_t|fine_x|sprite_zero|completed_pixels' src/canonical tests
```

Comparar VRAM, estado de scroll, seleção de pattern table e pixels das linhas
do HUD com uma implementação NES de referência executando a mesma ROM.
Registrar versão da referência, reset/timing inicial e roteiro de entrada;
alinhar os cenários antes de comparar. A referência é instrumento de validação,
não substituição do gameplay ou fonte de dados do jogo.
Ainda não foi selecionada/configurada uma referência para essa comparação;
esse é um trabalho pendente, não uma dependência já pronta.

Instrumentar os writes PPU e localizar a primeira divergência concreta. Começar pela captura de PPUCTRL, PPUSCROLL, PPUADDR e PPUDATA por frame, comparando v, t, fine_x, nametable e pixels nas linhas 0–31. Confirmar quando o jogo troca o scroll para a fase e como a PPU real mantém a região de status. A evidência S004 está em docs/evidence/2026-10-07/inspect-5m.txt.
Corrigir somente depois de demonstrar a causa; criar teste que capture a
regressão observada. Não forçar pixels, flags ou estado específico de SMB.

## Critério de conclusão

- Causa da divergência identificada com evidência.
- Correção relida, compilada e testada.
- Comparação do HUD no mesmo cenário e explicação das divergências restantes.
- Checkpoint, diário, logs e próxima ação atualizados e publicados.

## Dependências e caminhos alternativos

- Se a ROM não aparecer no checkout: conferir branch/commit e refazer o clone
  do GitHub; ela está em assets/canonical/SMB_v026.nes. Não buscar outra ROM.
- Sem referência disponível: registrar bloqueio, avançar diagnóstico sintético
  do PPU ou conformidade independente da CPU, deixando clara a validação faltante.
- Sem acesso de escrita GitHub: informar o bloqueio; não dizer que publicou.

## Fila posterior

1. Conformidade CPU independente e fidelidade PPU/scroll.
2. Entrada interativa host e APU/áudio.
3. Xbox: boot mínimo → loop → vídeo seguro → controle → core → áudio.
4. Embutir PRG/CHR verificados no default.xex e obter teste físico do proprietário.

Não gerar um XEX pelo backend antigo como se contivesse o runtime canônico.
Não marcar hardware como validado sem teste físico informado pelo proprietário.
