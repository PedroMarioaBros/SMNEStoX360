# Próxima ação — após S011 (09/10/2026)

Ao iniciar: START_HERE.md → AGENTS.md → docs/CONTINUITY_PROTOCOL.md →
docs/WORK_CHECKPOINT.md → docs/NEXT_STEPS.md → docs/SESSION_LOG.md.
Usar **exclusivamente** `assets/canonical/SMB_v026.nes`; confirmar SHA-256
da ROM/PRG/CHR antes de executar. Não trocar assets ou gameplay.

## Estado de partida

S011 foi integrada pela PR #6
https://github.com/PedroMarioaBros/SMNEStoX360/pull/6
(commit squash `d8a9c0a60a2aa46b92276d1ece0c0558204dc2e3`).
O canal noise da APU 2A03 foi implementado em `src/canonical/apu.[ch]`,
com os dois pulses e triangle anteriores. Código de trabalho integrado a
`tests/run_host_tests.sh` com uma décima primeira suíte:
`tests/test_apu_noise.c` (5 cenários, 16 períodos NTSC).
CI do código S011:
https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37890367890
(commit `19b5e2977aef8d98bea0c84c044d13318685b2a5`).
Conferir conclusão exata no workflow e status do PR antes de reusar.
Evidência e falha de fixture: docs/evidence/S011/README.md.
Não recontar regressões anteriores como novo progresso.

## Tarefa S012 — implementar DMC com segurança

1. Ler `src/canonical/apu.[ch]`, `nrom.[ch]`, `machine.c`,
   `cpu6502.[ch]`, tests e docs da última sessão.
2. Definir DMC ($4010–$4013 e $4015 bit4), tabela NTSC de 16
   períodos, DAC de 7 bits ($4011), registros de endereço e tamanho
   de amostra, shifter de 8 bits, controle de buffer, loop, IRQ.
   Acesso de amostra deve ler **somente o PRG canônico** pelo barramento
   em $C000–$FFFF; nunca importar outra ROM.
3. Projetar e testar o DMA da DMC (stalls CPU e interação com OAM DMA),
   IRQ de DMC e frame IRQ e interrupções no 6502. **Não ativar
   interrupções no core sem testes reais**; parte do timing hoje é por
   instrução e exige atenção à ordem de leitura/escrita.
4. Testes sintéticos para ler amostras, wrap $FFFF→$8000, shifts,
   saturação do DAC 0..127, enable/disable, loop/IRQ, períodos
   NTSC, comprimento, endereço; ausência de opcodes mascarados.
5. Validar build C99 `-Wall -Wextra -Werror`, suíte normal e ASan/UBSan,
   execução canônica pelo wrapper, comparação binjnes 600+600 de
   quadros/RAM. Ampliar comparação independente para saída e/ou estado
   de APU antes de alegar **áudio NES fiel**.

Comandos copiáveis no clone limpo:

```sh
git clone https://github.com/PedroMarioaBros/SMNEStoX360.git
cd SMNEStoX360
git log -1 --oneline
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes \
  --instructions 5000000 --input tests/canonical_start_right.input \
  --frame build/s012-baseline.bin
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
```

Resultado esperado como baseline: 11 suítes sintéticas normais/san
aprovadas, 600/600 quadros E 600/600 RAM no idle e no roteiro,
SDL readback 61.440 pixels aprovado. Não é validação da saída sonora.
Se clone local falhar por DNS, usar workflows específicos de CI
e atribuir resultados a commits exatos. Registrar falhas e evidências.

Depois do DMC: implementar mixer não-linear, resampling e PCM no host SDL;
confrontar saída de áudio com binjnes ou outra referência usando a mesma ROM.
Xbox: boot mínimo → loop → vídeo seguro → input → core → áudio →
`default.xex` com PRG/CHR embutidos → validação física pelo proprietário.
Build #16 antigo deu Fatal Crash. Não usar endereços hardcoded antigos.

**Ainda não é tradução de código 6502 para PowerPC nativo**:
o runtime implementa em software a execução do PRG 6502 original.
Não descrever como port sem emulação nem atribuir percentual global
sem critério medido. Registrar este limite e próximo comando no GitHub.
