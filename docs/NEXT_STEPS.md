# Próxima ação — S007, 08/10/2026

Leia START_HERE.md e cumpra o protocolo permanente. Não reinventar core/ROM.

## Retomar o host

```sh
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/build_host.py
python3 tests/test_host_frontend.py
build/host/smb-v026-host
```

SDL2 dev necessário; fallback local documentado em HOST_FRONTEND.md.
Smoke esperado: 600 quadros, 5.794.218 instruções, 17.865.924 ciclos,
593 NMIs, 101 opcodes, 61.440 pixels readback PASS. Execução manual continua
pendente; não confundir smoke dummy com gameplay humano ou teste Xbox.

## Próximo trabalho técnico: começar APU

```sh
cat src/canonical/machine.c src/canonical/nrom.c
rg -n 'apu_io|4015|4017|irq' src/canonical tests
```

1. Inspecionar acessos de áudio da ROM e referências primárias da APU Ricoh 2A03.
2. Definir clock CPU/APU, frame sequencer e integração de registradores sem
   regredir controller 2 ($4017 READ versus WRITE).
3. Implementar primeiro canal pulse com testes sintéticos para timer/duty,
   length counter/envelope e mute. Não declarar áudio completo com só um canal.
4. Comparar comportamento/saída com referência, preservando origem PRG/CHR.
   Depois pulse 2, triangle, noise, DMC e mixer/resampling, conforme necessidade.
5. Antes de publicar mudanças no core, rodar comparação curta:

```sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
```

Esperado: pixels/RAM iguais em ambos os cenários. Qualquer diferença exige
investigação; não trocar hashes esperados para esconder regressões.
Atualize checkpoint, diário, evidências e este arquivo; publique e confirme HEAD.

Xbox permanece alvo final: boot mínimo → loop → vídeo seguro → input/core →
áudio → XEX com assets embutidos → teste físico. Não reutilizar endereços fixos
do backend antigo. Build #16 teve Fatal Crash. Nenhum XEX canônico validado.
