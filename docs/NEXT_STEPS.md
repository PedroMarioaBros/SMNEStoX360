# Próxima ação — S005, 07/10/2026 UTC

Leia START_HERE.md e cumpra CONTINUITY_PROTOCOL.md. Código, ROM e evidências
estão no GitHub. Atualize checkpoint, diário e este arquivo antes de encerrar.

## Estado e comandos imediatos

S005 implementou odd-frame skip NTSC e corrigiu o diagnóstico documental de
HUD ausente: o HUD já aparecia no quadro anterior. Não refazer uma correção de
scroll baseada nessa premissa. O recorde continua 50 milhões de instruções.

```sh
git status --short
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/run_canonical.py --instructions 5000000 --input tests/canonical_start_right.input --frame build/gameplay.bin > build/gameplay.txt
diff -u docs/evidence/S005/start-right.txt build/gameplay.txt
python3 tools/frame_to_png.py build/gameplay.bin build/gameplay.png
```

Esperado: 5.000.000 instruções, 15.390.305 ciclos CPU, frame 516, linha/dot
207/88, PC $8057, 99 opcodes, 510 NMIs, 333 sprite-zero hits, nenhuma parada.
Hash do quadro: f8f3847872da7e7768e9244513f789240f2eabdb64404a777a066948009659e5.
Essa repetição é baseline, não avanço novo.

## Próximo trabalho técnico: validação independente

Há uma avaliação de binjnes já descrita em `.github/workflows/binjnes-eval.yml`.
Inspecione esse workflow e o código da referência antes de adaptar seu tester:

```sh
cat .github/workflows/binjnes-eval.yml
rg -n 'frame|input|completed_pixels|nmi' tests/run_canonical_rom.c src/canonical
```

A referência prevista nesse workflow é binji/binjnes, commit
`e2f5871a28ff189daa82b70be9324a42e2aaf9fd`. Ainda NÃO foi usada nesta sessão
para validar nossos quadros. Ela serve apenas como instrumento independente;
PRG/CHR/gameplay continuam exclusivamente da ROM canônica.

1. Compilar o tester de referência e verificar a licença/versão usada.
2. Executar a ROM canônica verificada nos dois runtimes. Alinhar reset,
   frames de entrada e instante de captura; não comparar por número arbitrário
   de instruções entre runtimes com sincronização diferente.
3. Salvar índices de pixels/VRAM, localizar a primeira divergência e escrever
   teste de regressão para a causa comprovada antes de corrigir o core.
4. Publicar adaptador, comandos, evidências, limitações e próximos passos.

Não declarar o quadro correto apenas porque tem HUD. A região sob TEMPO,
scroll, sprite-zero e interrupções ainda precisam de comparação independente.
Sem referência disponível, avançar conformidade sintética da CPU/PPU e registrar
explicitamente o que não foi validado.

Depois: entrada interativa host → APU → Xbox boot mínimo/loop/vídeo seguro →
controle/core → áudio. Não usar o backend antigo com endereços de vídeo fixos.
Nenhum novo XEX foi produzido; build histórico #16 teve Fatal Crash.
