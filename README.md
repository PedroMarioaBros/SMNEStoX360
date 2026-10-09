# SMB360 — SMB_v026 no Xbox 360

## 🧭 Situação atual sem linguagem técnica

**[LEIA AQUI — STATUS PARA PEDRO](docs/STATUS_PARA_PEDRO.md)** · [Painel central dos projetos Xbox 360](https://github.com/PedroMarioaBros/OpenXeChain-X360-Builder/blob/main/docs/PAINEL_PMCN_XBOX360.md)

**Caminho oficial:** `canonical-xex.yml` + `src/canonical/` + ROM `SMB_v026.nes`. A CI gera um XEX2 de **diagnóstico sem vídeo/áudio/controles**, ainda **não testado no Xbox físico**. A antiga compilação `xbox360-cloud-build.yml` usa `nathsou`, não é a entrega canônica e suas falhas não anulam a CI canônica. Consulte [S014](docs/evidence/S014/README.md) e [próxima etapa S015](docs/NEXT_STEPS.md).

---


**Novo chat ou Work: leia [START_HERE.md](START_HERE.md) antes de trabalhar.**
O registro de cada avanço, da próxima ação e do próximo comando no GitHub é
obrigatório para toda sessão e todo sucessor.

O produto final pretendido é um `default.xex` que inicia diretamente a ROM
SMB_v026 integrada ao executável, sem seletor de ROM ou interface de emulador.
**Ainda não existe um XEX dessa arquitetura validado no Xbox físico como jogo jogável.**

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
Desde S008–S012, a APU possui contador de quadros, pulse1, pulse2,
triangle, noise e DMC com testes host. **A S013 adicionou mixagem
não linear NES, PCM mono 48 kHz/16 bits, reprodução SDL opcional
e gravação WAV determinística**. A ROM canônica produziu
479.146 amostras em 600 quadros, com captura arquivada na CI.
O código agora passa 13 suítes host. Não houve audição em um
dispositivo físico nem comparação com áudio de referência
independente. DMC mantém DMA/IRQ aproximados (stall fixo por
instrução), não cycle-perfect; os cinco canais continuam 0–15
em pulse/triangle/noise e 0–127 no DMC. A arquitetura atual executa o PRG 6502 original
em runtime de software, não é ainda tradução nativa PowerPC.
O checkpoint separa implementação, testes sintéticos, execução da ROM e hardware.

Os backends antigos `src/platform/xex/`, integrações `nathsou`
e `scripts/build-smb-xex.sh` são históricos e **não fazem parte do
runtime canônico**. S014 acrescentou `src/platform/canonical_xex/`,
`scripts/build-canonical-xex.sh` e um workflow separado
`canonical-xex.yml` que compila o core C99 canônico inteiro para PPC,
incorpora a ROM `SMB_v026.nes` completa e produz XEX2 verificado.
É somente um executável **headless de diagnóstico**, sem imagem/áudio/input
no console e ainda NÃO testado fisicamente em hardware. Ver
[prova técnica S014](docs/evidence/S014/README.md).
O build antigo #16 sofreu Fatal Crash imediato no Xbox 360 e não é funcional.

A retomada Xbox deve seguir: boot mínimo → loop → vídeo seguro → controle →
runtime canônico → áudio. Não reutilizar endereços de framebuffer hardcoded.
