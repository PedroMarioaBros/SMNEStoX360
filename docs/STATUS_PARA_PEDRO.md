# Super Mario Bros. NES → Xbox 360 — situação para o proprietário

**Fotografia de 09/10/2026.** Para iniciar qualquer trabalho técnico, siga obrigatoriamente [START_HERE.md](../START_HERE.md), [AGENTS.md](../AGENTS.md), [docs/WORK_CHECKPOINT.md](WORK_CHECKPOINT.md) e [docs/NEXT_STEPS.md](NEXT_STEPS.md). Este resumo não substitui o histórico técnico.

[⬅ Painel central Xbox 360](https://github.com/PedroMarioaBros/OpenXeChain-X360-Builder/blob/main/docs/PAINEL_PMCN_XBOX360.md) · [Código](https://github.com/PedroMarioaBros/SMNEStoX360) · [GitHub Actions](https://github.com/PedroMarioaBros/SMNEStoX360/actions)

## Em uma frase
**O código canônico do jogo já foi compilado pelo GitHub na forma de `default.xex`, mas esse arquivo é apenas de diagnóstico: NÃO é um Super Mario Bros. jogável no Xbox 360.**

## Entenda as duas linhas de compilação

| Linha | Uso atual | Prova |
| --- | --- | --- |
| **CANÔNICA** — `.github/workflows/canonical-xex.yml`, `scripts/build-canonical-xex.sh`, `src/canonical/` e `src/platform/canonical_xex/` | **É a rota oficial de desenvolvimento.** Preserva ROM `SMB_v026.nes`; compilação PowerPC + XEX2 válida; sem backend Xbox de vídeo/áudio/controle | [Execução concluída com sucesso](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798039) e [provas S014](evidence/S014/README.md) |
| **LEGADA** — `.github/workflows/xbox360-cloud-build.yml`, `scripts/build-smb-xex.sh`, `src/platform/xex/`, `nathsou` | História experimental; antigo build teve Fatal Crash. Workflow recente falha ao exigir `boot-test.xex` não produzido pelo script. Não usar essa saída como jogo oficial | [Falha isolada da rota antiga](https://github.com/PedroMarioaBros/SMNEStoX360/actions/runs/37958798128) |

O `XEX2` canônico **não foi testado fisicamente** no console. Seu cabeçalho e hashes foram verificados, não seu boot. Os testes do host passaram com cenários controlados (CPU, PPU, controle, áudio PCM), mas não garantem fidelidade total nem reprodução no hardware.

## Caminho exato até jogo jogável

1. **Boot mínimo** do XEX canônico e investigação do alerta de linker `/align specified without /driver`.
2. **Imagem segura** por API comprovada do Xbox (não copiar endereços hardcoded da rota antiga).
3. **Controle Xbox 360**, leitura de botões e sincronização de frames.
4. **Áudio Xbox**, conectando a saída PCM do runtime canônico.
5. **Teste real de jogabilidade** pelo proprietário, com logs e identificação do pacote.
6. **Depois**, avaliar a exigência separada de tradução estática de código 6502→PPC; no momento o PRG original roda em interpretador C.

**Próxima tarefa registrada:** S015, diagnóstico de boot e backend de vídeo seguro, em [NEXT_STEPS.md](NEXT_STEPS.md). Não começar um terceiro motor nem importar uma ROM diferente.

## O que você pode fazer pelo celular
Abra [Actions](https://github.com/PedroMarioaBros/SMNEStoX360/actions). O nome certo para procurar é **Canonical NES Xbox 360 diagnostic XEX**. Uma execução verde comprova compilação e verificações automáticas; o artifact `smb360-CANONICAL-DIAGNOSTIC-xex` não é pacote jogável.

Nenhuma instalação de ferramentas no seu PC é necessária para disparar esse build na nuvem; testes físicos dependerão do seu Xbox desbloqueado.

## Para novo chat/Work
> Continue `PedroMarioaBros/SMNEStoX360` seguindo `START_HERE.md`, `AGENTS.md` e `docs/CONTINUITY_PROTOCOL.md`. Preserve a ROM canônica, a linha S014 e as evidências. Execute a próxima S015, registre o resultado mesmo se falhar e atualize `WORK_CHECKPOINT.md`, `SESSION_LOG.md` e `NEXT_STEPS.md` antes de encerrar. Não declarar XEX de diagnóstico como jogo jogável.
