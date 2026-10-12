# S016 — INVESTIGAÇÃO DE FATAL CRASH — PROVAS DE BOOT ISOLADAS

> **O S015 FALHOU NO XBOX 360 REAL. NÃO VOLTE A EXECUTAR O XEX S015.**
> Este documento apresenta um plano para um eventual segundo
> teste controlado; executáveis S016 só devem ser disponibilizados
> depois do workflow específico S016 PASS. Nem mesmo PASS do CI
> garante boot no Xbox. Testes podem causar novo Fatal Crash.
>
> **Nenhum destes arquivos é um jogo jogável.**

## Resultado S015 preservado

O proprietário baixou `smb360-CANONICAL-DIAGNOSTIC-xex`,
incluindo `BUILD_MANIFEST.txt`, `canonical.exe`,
`default.xex`, `SHA256SUMS.txt` e `TESTE_XBOX_S015.md`.
Ao executar o `default.xex`, obteve **Fatal Crash imediato**
e precisou reiniciar o console. Isso é falha de hardware observada,
não falha de teste sintético. GitHub issue
https://github.com/PedroMarioaBros/SMNEStoX360/issues/12 .

Não sabemos se `main` iniciou: `printf` e XAM são chamados
logo no começo, e o linker ainda advertia sobre `/align`
sem `/driver`. A causa continua **indeterminada**.

## O que S016 fez diferente

Em vez de alterar tudo no mesmo XEX, separou dois programas
independentes e com importações estritamente minimizadas:

**Etapa 01 — `01_KERNEL_ONLY/default.xex`**

- Compilação Xbox OS via OpenXeChain + SynthXEX.
- Apenas `main`, importação de `KeDelayExecutionThread` para
  aguardar aproximadamente **5 segundos** e retornar.
- **Sem ROM NES, sem core 6502, sem chamadas diretas de notificação/
  controle XAM, sem `printf`, sem memória GPU, sem áudio e sem gamepad.**
  Atenção: o runtime de inicialização da toolchain ainda importa
  `xam.xex`/`XamLoaderLaunchTitle`, observado no PE produzido.
- Se sofrer Fatal Crash, é improvável que a lógica do Mario,
  CPU NES ou XAM do S015 sejam necessárias para reproduzir
  o problema; investigar headers, imports, formato XEX,
  proteção/encriptação e compatibilidade do console/toolchain.
- Se retornar ao gerenciador após aproximadamente cinco segundos,
  há evidência física de que *esse* XEX minimalista inicia e
  termina; não prova ainda que o runtime canônico funciona.

**Etapa 02 — `02_CANONICAL_NO_XAM/default.xex`**

- Usa o mesmo `src/canonical/` da máquina NES, ROM autorizada
  de **40976 bytes** embutida e CPU 6502 interpretada,
  **sem chamadas diretas XAM**, sem `printf`, sem framebuffer e sem áudio.
  A dependência XAM da biblioteca de inicialização permanece.
- Executa, no máximo, 120 quadros; tem limites de
  1.000.000 de instruções por quadro; após conclusão
  espera cinco segundos e retorna.
- **Somente cogitar executar se a ETAPA 01 tiver funcionado.**
  Se falhar apenas aqui, a diferença está no código/dados
  incorporados ou no maior binário; não prova automaticamente
  bug na CPU em vez de linker/carregamento.

## Instruções de instalação, apenas após disponibilização e aceite

Baixar o artifact específico
`smb360-S016-BOOT-ISOLATION-NOT-PLAYABLE` do workflow
`S016 Xbox boot isolation (NOT S015)` na branch `main`.
Descompactar e **não substituir** o jogo, os arquivos
do Aurora ou o `default.xex` antigo: existem **dois**
subdiretórios de teste distintos. Copiar e executar
separadamente, pelo navegador de arquivos do
Xbox desbloqueado, na ordem estrita acima.

1. Começar somente por `01_KERNEL_ONLY/default.xex`.
   Registrar se volta ao menu após ~5 segundos, se
   dá Fatal Crash imediato ou se a tela congela.
2. Se e somente se a etapa 01 concluir normalmente,
   considerar a `02_CANONICAL_NO_XAM/default.xex`.
   Pode demorar mais que cinco segundos por processar
   120 frames antes de aguardar. Registrar retorno,
   crash imediato/tardio ou travamento.
3. Se houver **qualquer Fatal Crash**, parar. Não
   repetir o mesmo XEX. Não alterar NAND, firmware,
   exploit ou `launch.ini` para testar.
4. Não haverá notificações na tela: isto é **intencional**
   para isolar a XAM. A única observação é a duração
   e o retorno ao gerenciador ou crash.

**Resultado de console ainda PENDENTE em S016.**
O proprietário poderá informar, se souber, RGH/JTAG/FreeMyXe,
gerenciador Aurora/XeXMenu, e se outros homebrews XEX
funcionam. Não pedir dados privados, credenciais ou flash de NAND.

## Validação automática e limites

- Ambos os XEX devem ter cabeçalho `XEX2`, PE PowerPC
  `machine=0x01f2`, **nenhuma** importação `xam.xex`.
- Apenas a etapa 02 deve incluir ROM completa
  `SMB_v026.nes` idêntica byte a byte ao arquivo versionado:
  SHA-256 `57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047`.
- A etapa 01 **não inclui a ROM** de propósito.
- Sem teste físico do proprietário não declarar boot.
- O aviso do linker `/align specified without /driver`
  pode continuar; um XEX2 estruturalmente válido
  ainda pode ser incompatível com o console. A documentação
  upstream do SynthXEX o descreve como software em estágio
  inicial, sem garantias de estabilidade.

Próximo comando de desenvolvimento:
`bash scripts/build-xex-boot-isolation.sh` após configurar
`OPENXECHAIN_PREFIX`, validar CI na branch e na main;
investigar imports, PE, XEX e mecanismo de dashboard
antes de pedir novo teste.
