# Próxima ação — depois da S015 (09/10/2026)

Leia `START_HERE.md`, `AGENTS.md`, o protocolo de continuidade,
`WORK_CHECKPOINT.md`, `SESSION_LOG.md` e
`evidence/S015/README.md`. Não usar o build nathsou legado.
HEAD de entrada S015: `884c5db3defec68fbf58793fccbbc69a225584c2`.
S015 integrada pela PR #11:
https://github.com/PedroMarioaBros/SMNEStoX360/pull/11 ,
squash `bd9775a84d1b9451f61bc5d79bf1df0b2002952d`.
A ROM versionada `assets/canonical/SMB_v026.nes` é fonte
exclusiva do jogo; SHA-256 integral
`57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.

## Portas de saída S015

O XEX canônico mantém ROM/PRG/CHR intactos e usa
`src/canonical/` compilado para PowerPC. Os novos
`controller_map.[ch]` e `tests/test_controller_map.c`
verificam dois gamepads no barramento NES. O entrypoint
`src/platform/canonical_xex/main.c` usa as funções XAM
`XamInputGetState` e `XNotifyQueueUI` para tentar exibir
START e PASS ao processar 120 quadros. Sem framebuffer direto.

O validador `tools/verify_canonical_xex.py` exige PE
PowerPC, XEX2 e ROM íntegra de 40976 bytes realmente
incorporada no PE. Script do build coloca
`TESTE_XBOX_S015.md` dentro do artifact diagnóstico.
**Compilação não é execução no console**.
Conferir jobs verdes da versão final em `main`
antes de oferecer teste físico.

## Ação imediata do proprietário: teste S015 de boot

Abrir `docs/TESTE_FISICO_S015.md` e baixar do workflow
`canonical-xex.yml` o artifact verde
`smb360-CANONICAL-DIAGNOSTIC-xex` **da main**.
Executar `default.xex` somente em pasta isolada
de Xbox 360 desbloqueado; sem atualizar NAND/firmware.
Observar notificação `SMB360 S015 START - 120 frames`,
a notificação de `120 FRAMES PASS`, possíveis erros,
Fatal Crash, duração e retorno ao dashboard. Compartilhar
essas observações para distinguir boot de bloqueio XAM.
**Sem retorno físico, manter hardware como NOT_TESTED**.

## Tarefa ativa S016 — frontend gráfico seguro e boot auditável

1. Se houver resultado físico S015, anexá-lo ao diário,
   classificar START, PASS, erro, travamento ou resultado
   inconclusivo; nunca inventar teste físico.
2. Investigar aviso `lld-link /align without /driver`
   comparando configuração da toolchain e formato PE/XEX;
   não suprimi-lo com flags arbitrárias.
3. Encontrar API de renderização Xbox 360 compatível
   comprovada, com desenhos e testes em framebuffer
   alocado/gerenciado pela API. **Jamais** reutilizar
   endereços hardcoded de `src/platform/xex/video_fb.c`:
   o build antigo #16 gerou Fatal Crash.
4. Ligar pixels canônicos à apresentação segura e os
   controladores testados à sessão de jogo; pacing NTSC.
5. Depois ligar PCM `src/canonical/audio_pcm.c` a saída
   de áudio do console, verificando limites e underrun.
6. Em paralelo, projetar uma conversão estática e
   verificável de fluxo/opcodes 6502 da ROM original para
   PowerPC, visando a meta **sem interpretador**. Hoje
   `smb360_cpu6502_step` ainda interpreta instruções.

## Comandos verificáveis

```sh
git clone https://github.com/PedroMarioaBros/SMNEStoX360.git
cd SMNEStoX360
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
python3 tests/test_host_audio.py
```

Com toolchain OpenXeChain disponível:

```sh
bash scripts/build-canonical-xex.sh
python3 tools/verify_canonical_xex.py
(cd build-canonical-xex && sha256sum -c SHA256SUMS.txt)
```

Esperado: **14 suítes host** em normal e san, dois cenários
600/600 pixels e RAM, WAV determinístico, XEX2 e PE PowerPC
com ROM integral embutida. Teste físico é separado.
Publicar qualquer avanço, falha, documentação, evidência
e HEAD final no GitHub antes de encerrar a próxima sessão.
