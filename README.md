# SMB360 — SMB_v026 no Xbox 360

**Novo chat ou Work: leia [START_HERE.md](START_HERE.md) antes de trabalhar.**
O registro de cada avanço, da próxima ação e do próximo comando no GitHub é
obrigatório para toda sessão e todo sucessor.

O produto final pretendido é um `default.xex` que inicia diretamente a ROM
SMB_v026 integrada ao executável, sem seletor de ROM ou interface de emulador.
**Ainda não existe um XEX validado dessa arquitetura canônica.**

## Fonte de verdade

A ROM do proprietário é a fonte exclusiva de PRG, CHR, dados e comportamento.
O runtime em `src/canonical/` executa seu PRG 6502 original.
Nenhuma recriação de SMB de terceiros é usada nesse caminho.

- ROM SHA-256: `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`
- NROM-256, 32 KiB PRG, 8 KiB CHR, mirroring vertical.
- [Identidade canônica](docs/SMB_V026_CANONICAL_ROM.md)
- [Checkpoint e limitações atuais](docs/WORK_CHECKPOINT.md)

## Arquivos completos no GitHub

A ROM está em [assets/canonical/SMB_v026.nes](assets/canonical/SMB_v026.nes),
com PRG, CHR e manifest na mesma pasta, conforme autorização expressa do
proprietário na S003. Não é necessário recuperar anexos no ChatGPT.
[REPRODUCIBILITY.md](docs/REPRODUCIBILITY.md) lista arquivos, dependências,
inventário dos outputs históricos e comandos para retomar de um clone limpo.

## Compilar e testar no host

Requisitos: compilador C99 e Python 3. Não requer SDK Xbox para estes testes.

```sh
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes
python3 tools/run_canonical.py assets/canonical/SMB_v026.nes \
  --instructions 5000000 --input tests/canonical_start_right.input \
  --frame build/gameplay.bin
```

O wrapper valida SHA-256 integral, PRG e CHR antes de executar.
O runner C direto valida tamanho/header, mas não SHA-256: use o wrapper.
`frame.bin` contém 256 × 240 índices de paleta de 6 bits do último quadro
completo, sem header; não é imagem RGB nem referência comprovada de fidelidade.
As entradas de controle são eventos `frame máscara_hex` na ordem
A, B, Select, Start, Up, Down, Left, Right (bits 0 a 7).
Os frames são contados a partir de zero.

Para ASan/UBSan:

```sh
SANITIZE=1 sh tests/run_host_tests.sh
```

Em ambientes sob ptrace que impedem LeakSanitizer, acrescente
`ASAN_OPTIONS=detect_leaks=0` tanto aos testes quanto à execução da ROM.
Isso mantém AddressSanitizer e UndefinedBehaviorSanitizer ativos.

## Estado e Xbox

CPU: 151 opcodes oficiais implementados; não é uma certificação cycle-perfect.
PPU funcional inicial, NMI, DMA e controle disponíveis no host.
Desde S008–S011, a APU possui contador de quadros, pulse1, pulse2,
triangle e noise com testes host. **Ainda não produz áudio audível**:
faltam DMC, IRQ/DMA completos, mixer, resampling e saída PCM.
A S011 preservou quadros e RAM nos cenários 600+600 frames da
referência independente, sem demonstrar fidelidade de waveform
de áudio. Os canais possuem apenas níveis DAC individuais 0–15. A arquitetura atual executa o PRG 6502 original
em runtime de software, não é ainda tradução nativa PowerPC.
O checkpoint separa implementação, testes sintéticos, execução da ROM e hardware.

Os backends `src/platform/`, integrações `nathsou` e scripts Xbox antigos
são históricos e **não estão ligados ao runtime canônico**. Não construir um
jogo novo por esses caminhos sem a migração deliberada.
O build antigo #16 sofreu Fatal Crash imediato no Xbox 360 e não é funcional.

A retomada Xbox deve seguir: boot mínimo → loop → vídeo seguro → controle →
runtime canônico → áudio. Não reutilizar endereços de framebuffer hardcoded.
