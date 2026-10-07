# Retomada exclusivamente pelos arquivos do GitHub

Na S003, o proprietário determinou explicitamente que a ROM também fosse
incluída no GitHub. Esta decisão substitui a restrição anterior. O repositório
contém agora os inputs e outputs existentes necessários à continuidade.
Não é preciso acessar a conta ChatGPT, este chat ou seus anexos.

## Clone limpo

Dependências host: Git, Python 3 (somente biblioteca padrão), compilador C99,
shell POSIX. Para abrir o arquivo histórico, tar com suporte xz.
Os toolchains são programas de ambiente, não dados exclusivos do ChatGPT.

~~~sh
git clone https://github.com/PedroMarioaBros/SMNEStoX360.git
cd SMNEStoX360
python3 tools/verify_repository.py
sh tests/run_host_tests.sh
python3 tools/run_canonical.py --instructions 5000000 \
  --input tests/canonical_start_right.input --frame build/gameplay.bin
~~~

O argumento de ROM é opcional e usa assets/canonical/SMB_v026.nes por padrão.
Para verificar a reprodução contra o registro atual da S005 (os logs S001 são históricos):

~~~sh
python3 tools/run_canonical.py --instructions 5000000 \
  --input tests/canonical_start_right.input --frame build/gameplay.bin \
  > build/reproduced.txt
diff -u docs/evidence/S005/start-right.txt build/reproduced.txt
~~~

## Mapa dos arquivos

| Conteúdo | Caminho |
|---|---|
| ROM exata, 40.976 bytes | assets/canonical/SMB_v026.nes |
| PRG exato, 32.768 bytes | assets/canonical/SMB_v026.prg |
| CHR exato, 8.192 bytes | assets/canonical/SMB_v026.chr |
| Hashes e vetores da extração | assets/canonical/manifest.json |
| Código atual e histórico de alterações | src/, include/, histórico Git |
| Testes, eventos de controle e runner | tests/ |
| Ferramentas e scripts de build | tools/, scripts/ |
| Outputs da S001, inclusive binários de teste | artifacts/session-S001-host.tar.xz |
| Inventário de bytes e SHA-256, incluindo membros do arquivo | artifacts/inventory.json |
| Logs de execução e diagnóstico | docs/evidence/2026-10-07/ e docs/evidence/S005/ |
| Quadros brutos e PNG da S005 | docs/evidence/S005/frames.tar.xz e start-right.png |
| Conversor de quadro para PNG | tools/frame_to_png.py |
| Fontes temporárias preservadas do desenvolvimento S001 | docs/evidence/2026-10-07/source/ |
| Continuidade | START_HERE.md, AGENTS.md, docs/WORK_CHECKPOINT.md, NEXT_STEPS.md, SESSION_LOG.md |

O arquivo histórico preserva o diretório build/ da S001: executáveis Linux de
testes (com sanitizers), runner, trace, ROM extraída, manifest, logs, quadros
brutos e screenshots título/fase. Inclui resultados repetidos e sanitizados.
Os executáveis são históricos, específicos do host, NÃO XEX. Para continuar,
recompilar a partir do código atual; não é necessário executar os binários antigos.

Extrair em diretório separado para não sobrepor builds atuais:

~~~sh
mkdir -p build/history-S001
tar -xJf artifacts/session-S001-host.tar.xz -C build/history-S001
~~~

As fontes em evidence/.../source/ preservam scripts de edição usados na S001
e uma sonda pontual de PPU. São registros históricos, não etapas do build:
não reaplicar expand_cpu.py, add_nmi.py ou ppu_regs.py ao HEAD atual.
O caminho ../rom/ usado na sonda original é histórico; a ROM hoje está
em assets/canonical/. Nenhuma etapa de retomada depende desses scripts.

## O que ainda não existe

Não há XEX novo do core canônico, APU implementada ou referência NES já
configurada para a comparação diferencial. Armazenar todos os arquivos
existentes não significa que esses marcos foram concluídos.
O próximo trabalho técnico é configurar comparação independente. O diagnóstico
de HUD ausente foi retirado na S005 após conferir os pixels reais.

## Regra de preservação

Todo arquivo novo necessário para retomar, incluindo diagnósticos e resultados,
deve ser publicado no GitHub. Manter código/assets diretamente no Git; outputs
podem ser compactados com inventário e hashes. Atualizar os comandos e o diário.
Caches e novos builds descartáveis podem ser regenerados pelos scripts, mas
nenhuma evidência única ou input deve ficar apenas no ambiente ChatGPT.
Não publicar credenciais ou tokens.
