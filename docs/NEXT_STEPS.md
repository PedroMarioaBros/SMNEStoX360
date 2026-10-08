# Próxima ação — S006, 08/10/2026

Cumpra START_HERE.md, AGENTS.md e CONTINUITY_PROTOCOL.md em toda sessão.

## Baseline atual

```sh
git status --short
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/compare_reference.py
```

Esperado: idle pixels/RAM 600/600; input pixels/RAM 6000/6000. Fonte fixada
binjnes MIT já está versionada; não buscar outra ROM ou recriação SMB.
Essa reprodução é verificação, não marco novo. Ver tests/reference/README.md
para a política de entrada/captura, diferente do runner tradicional.

```sh
python3 tools/run_canonical.py --instructions 5000000 --input tests/canonical_start_right.input --frame build/gameplay.bin > build/gameplay.txt
diff -u docs/evidence/S006/start-right.txt build/gameplay.txt
python3 tools/frame_to_png.py build/gameplay.bin build/gameplay.png
```

Esperado: 5m instruções, 15.391.133 ciclos, frame 516, linha/dot 214/185,
99 opcodes, 510 NMIs, sem parada. Não usar baseline S005 para CI do core S006.
O diagnóstico de HUD ausente foi retirado na S005. S006 corrigiu controle 2,
eliminando uma divergência real de RAM sem mudar os pixels do roteiro medido.

## Trabalho seguinte: bancada host interativa

Preparar frontend host mínimo para jogar a ROM canônica manualmente, reaproveitando
machine/PPU/controller1/controller2. O host é bancada, o produto final continua Xbox.

```sh
rg -n 'SDL|OpenXeChain|boot-test|xenon|framebuffer' Makefile* scripts src .github/workflows
pkg-config --modversion sdl2
cat src/canonical/machine.h src/canonical/nrom.h
```

1. Inspecionar dependências disponíveis e convenções de build. Criar frontend
   separado do backend Xbox antigo; vídeo a partir de completed_pixels e input
   pelas APIs canônicas. Nada de gameplay externo.
2. Validar fingerprint antes de carregar; manter interface direta, sem seletor.
3. Documentar teclas/mapeamento e comandos de compilação. Não fingir teste
   interativo se o ambiente não tiver display/controlador; oferecer smoke headless.
4. Reexecutar testes e comparação curta (`--idle-frames 600 --input-frames 600`)
   apenas se alterar o core. Publicar código, resultados, checkpoint e próxima ação.

Depois: APU/áudio por canais com testes → Xbox boot/loop → vídeo seguro →
input/core → áudio → default.xex com PRG/CHR embutidos → teste físico.
Pode investigar boot Xbox em paralelo sequencial com host, mas nunca reutilizar
endereços hardcoded ou chamar build #16 funcional: sofreu Fatal Crash.

## Limites ainda existentes

Comparação confirma dois cenários específicos, não todo o jogo. Ainda faltam
áudio, microtiming PPU/CPU, open bus, power-up/races, outros roteiros e execução
real no Xbox. Recorde medido: 57.751.826 instruções na captura diferencial longa.
