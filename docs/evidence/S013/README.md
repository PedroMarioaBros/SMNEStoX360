# S013 — mixer não linear NES / PCM 48 kHz / WAV / SDL (09/10/2026)

## Origem e integridade do jogo

- HEAD inicial main: `6e158f17572e17bfa85a8543710a47fdd73497d1`.
- Branch: `work/s013-audio-pcm-20261009`.
- A única fonte do jogo continua sendo `assets/canonical/SMB_v026.nes`
  (40.976 bytes), PRG e CHR canônicos. Nenhum asset foi modificado.
- SHA-256 ROM:
  `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- SHA-256 PRG:
  `9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4`.
- SHA-256 CHR:
  `5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0`.

## Código introduzido

1. `src/canonical/audio_pcm.[ch]`: soma não linear dos DACs NES
   pulse1/pulse2 + triangle/noise/DMC pela fórmula NESdev (não soma
   linear). APU DMC usa 7 bits; outros canais 4 bits. Se um subgrupo
   tem volume zero, a fórmula evita divisão por zero.
2. Phase accumulator inteiro NTSC: 1.789.773 ciclos CPU por segundo,
   48.000 amostras PCM mono signed 16-bit por segundo. Sequência
   determinística por contagem de ciclos entre instruções.
   Filtros aproximados de 90 Hz e 440 Hz passa-altas e 14 kHz
   passa-baixas, coeficientes fixos; ganho 30.000 e clipping protegido.
   **Níveis da APU são coletados após cada instrução concluída,
   não a cada microciclo NES**. Essa é uma aproximação intencional.
3. `src/host/main.c`: `SDL_QueueAudio` alimenta dispositivo de
   áudio quando disponível; ausência de dispositivo não interrompe
   vídeo/gameplay. Buffer de amostras com limite de latência.
   Opção `--wav caminho.wav` grava WAV PCM 16-bit little-endian
   com cabeçalho RIFF e número exato de amostras. `--smoke`
   não abre dispositivo de áudio, mas aceita gravação WAV.
4. `tests/test_audio_pcm.c`: curva não linear e interação DMC,
   duração exata de 48.000 amostras por 1.789.773 ciclos, fragmentação
   de avanços idêntica, filtros determinísticos, amostras não nulas
   e ausência de saturação sintética. Runner passa de 12 para 13 suítes.
5. `tests/test_host_audio.py`: duas execuções do mesmo programa e
   ROM com roteiro de 600 quadros; verifica WAV, duração, sample count,
   amplitudes com sinais positivo/negativo, não silêncio e SHA-256
   igual. Arquiva `build/host/canonical-600frames.wav`. Workflow
   do GitHub publica esse WAV como artifact `canonical-s013-audio-wav`.

## Medição da ROM canônica (saída PCM, sem referência de áudio)

Captura no CI (frontend anterior ao passo adicional de arquivamento),
durante `--smoke --wav`:

- Amostras: **479.146**, mono, 48.000 Hz, signed 16 bits.
- Min/max: **−5.889 / +6.328**; limite signed 16-bit
  ±32.768, sem clipping neste cenário.
- Amostras não nulas: **159.921**.
- Duas capturas idênticas byte por byte (SHA-256 WAV):
  `3aa163954a75eaa13b9969c11f59fefd2da41ed8fc2d524c9edb6019b2b4d56c`.
- Quadro SDL 599 preservado com a mesma referência binjnes
  `fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc`.
- Workflow onde essas medições foram lidas e testes normais/san
  passaram: https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37891992640 .
- O WAV emitido **não** foi comparado com áudio binjnes
  independente; portanto valores e determinismo não significam
  fidelidade auditiva ao NES original.

### Falhas reais no desenvolvimento

Workflows `37891903477`, `37891908427`, `37891911591`
falharam inicialmente na nova fixture `test_audio_pcm.c:45`,
que supunha silêncio quando nenhum canal tivesse sido habilitado.
O triangle DAC do NES mantém inicialmente o nível 15; os primeiros
samples da saída filtrada não são obrigatoriamente nulos.
Corrigida a expectativa da fixture, não o mixer nem a ROM.
Além disso `.github/workflows/canonical-host.yml` ganhou
gatilho `tests/test_audio*` para garantir a repetição do teste
em alterações futuras. O próximo workflow `37891992640`
concluiu **success** nos quatro jobs.

### Reproduzir a partir de clone Linux com SDL2

```sh
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
ASAN_OPTIONS=detect_leaks=0 SANITIZE=1 sh tests/run_host_tests.sh
python3 tools/compare_reference.py --idle-frames 600 --input-frames 600
python3 tools/build_host.py
python3 tests/test_host_frontend.py
python3 tests/test_host_audio.py
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  build/host/smb-v026-host --smoke --wav build/host/demo.wav
```

## Limites e próximo trabalho

A saída PCM sintetizada pode ser tocada pelo SDL **em um host com
dispositivo de som funcional**, mas nesta sessão o teste do SDL é
dummy/headless: ninguém ouviu o jogo por alto-falantes. Não existe
comparação independente de waveform/IRQ/DMA com um NES real.
DMC steal de ciclos é aproximado por instrução; PPU não é
cycle-exact. Xbox: nenhum novo XEX canônico validado em hardware,
antigo build #16 teve Fatal Crash; workflow legado compilava
terceiros e ainda tem o bloqueio
https://github.com/PedroMarioaBros/SMNEStoX360/issues/8 .
O PRG canônico roda pelo interpretador 6502 em software, não
via compilação estática PowerPC sem emulador.

Próximo passo: S014 validação independente de APU e reprodução,
melhorias de áudio/timing, e avançar o backend Xbox canônico
seguro sem confundir ferramentas históricas com o port.

## Integração publicada

- PR #9: https://github.com/PedroMarioaBros/SMNEStoX360/pull/9
  incorporada em `main`, squash `64a7edaa76c14b069da283023bab38d1b64e1173`.
- Diferenças revisadas: 13 arquivos alterados, **zero** alterações em
  `assets/canonical/`. Teste da branch aprovado em `37892058096`.
- WAV disponível no workflow em Artifacts como
  `canonical-s013-audio-wav` (id 11598404861).
