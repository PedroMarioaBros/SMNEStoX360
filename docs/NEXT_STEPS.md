# Próxima ação — S008, 09/10/2026

Leia START_HERE.md / AGENTS.md / CONTINUITY_PROTOCOL.md antes de agir.
ROM canônica e o PRG/CHR não devem ser substituídos.

## Tarefa ativa: validar e consolidar a base APU em branch

Branch: `work/s008-apu-pulse1-20261009`, derivada do main
`f24d735559ca74eee7467224b5023402e64fdf69`.

O módulo de áudio agora acompanha ciclos CPU (inclusive NMI e DMA).
Pulse 1 implementado sinteticamente, ainda sem PCM nem mixer, outros canais
não implementados. Testes sintéticos locais e CI do commit
`a41845a49415e04c6d71407bb6928615c14c7195` passaram; o CI precisa
confirmar o HEAD definitivo da branch após a asserção de integração.

Com um clone limpo, executar:

```sh
git fetch origin
git switch work/s008-apu-pulse1-20261009
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Requisitos: C99, Python 3, SDL2 dev e dependências de testes de referência
incluídas no repositório. O comparador esperado continua **600/600 pixels
e RAM idênticos em ambos os cenários**. Testes unitários devem passar e
contagem `m.bus.apu.cpu_cycles == m.cpu.cycles` deve persistir inclusive
após DMA. Conferir também job CI e commit exatos antes de interpretar sucesso.

Caso a comparação divirja, investigar acesso $4015, timing por instrução
e alterações no frame sequencer. Não ajustar hashes para esconder falha.
Documentar evidência (diferença de byte/frame) e preservar correção em branch.

Somente após validação completa e review, decidir merge da branch para main.
Depois implementar canal pulse 2 e demais unidades APU (triangle/noise/DMC,
mixagem, resampling para SDL), além da integração de IRQ, testes diferenciais
de áudio e reprodução audível. Não declarar fidelidade de áudio neste estágio.

## Alvo final Xbox 360

Boot mínimo → loop → vídeo seguro → entrada → runtime canônico → áudio →
XEX autossuficiente → teste no console. O antigo build #16 teve Fatal Crash.
Nenhum XEX da arquitetura atual foi validado em hardware. Não reutilizar
endereços hardcoded de vídeo.

Em toda sessão atualizar WORK_CHECKPOINT.md, SESSION_LOG.md e este arquivo;
fazer commits, publicar e verificar o HEAD remoto.
