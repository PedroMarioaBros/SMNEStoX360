# Bancada host SDL2 — S007

O executável inicia diretamente SMB_v026, sem seletor e sem ROM externa.
O build verifica SHA-256 da ROM/PRG/CHR antes de embutir seus bytes exatos.
A lógica continua no PRG 6502, executado pelo core canônico. SDL só fornece
janela, apresentação de pixels e entrada. Esta bancada não é o backend Xbox.

## Compilar e executar (Linux)

Dependências: Python 3, compilador C99 e desenvolvimento SDL2 (libsdl2-dev).

```sh
python3 tools/build_host.py
build/host/smb-v026-host
```

Sem áudio por enquanto. Janela inicial 768x720, imagem lógica 256x240,
redimensionável com aspecto preservado e filtro nearest. Paleta RGB ilustrativa,
mesma do conversor PNG, sem color emphasis ou emulação analógica.
Ritmo nominal 60,0988 Hz; velocidade real depende do host, ainda não medida
em sessão manual. Perda de foco pausa a execução e limpa teclas retidas.
P pausa/retoma; Escape fecha. Não gera arquivos automaticamente.
`--dump caminho.bin` solicita explicitamente o quadro final ao fechar.

| Ação | Teclado jogador 1 | Teclado jogador 2 | Controle SDL |
|---|---|---|---|
| Direções | Setas | WASD | D-pad |
| NES A / pular | Z | G | A |
| NES B / correr/atirar | X | F | X |
| Start | Enter | Espaço | Start |
| Select | Shift direito | Tab | Back |

Dois gamepads podem ser conectados/removidos durante execução. O primeiro slot
livre recebe o dispositivo; direções opostas são canceladas. Gamepads físicos,
perda/retorno de foco, pausa e sessão manual longa ainda não foram validados.
O teste automatizado cobre entrada de teclado SDL do jogador 1 e apresentação.

## Teste reproduzível sem display

```sh
python3 tests/test_host_frontend.py
```

Usa SDL_VIDEODRIVER=dummy, executa em diretório vazio, injeta seis eventos SDL
(Start, direita e pulo) e apresenta 600 quadros. Confere cada pixel lógico da
última imagem por SDL_RenderReadPixels, comparando a textura em escala 3x,
e compara o dump com o hash independente binjnes da captura 599 da S006.
A política de entrada é a do adaptador de referência, após a captura anterior.
O roteiro tradicional run_canonical.py usa outra política (início do frame).
O teste não requer uma sessão gráfica nem um controle físico.

## Dependências nesta sessão

SDL runtime 2.30.0 disponível, mas sem headers/sdl2-config. apt-get falhou por
permissões de setgroups/seteuid; não foi elevada permissão nem instalado pacote.
Usados localmente os headers oficiais SDL, tag release-2.30.0, commit
859844eae358447be8d66e6da59b6fb3df0ed778 (licença zlib, fonte intacto).
SDL é dependência padrão de ambiente, não dado exclusivo do chat.
Alternativa reproduzível à instalação do pacote de desenvolvimento:

```sh
git clone --depth 1 --branch release-2.30.0 https://github.com/libsdl-org/SDL.git build/SDL
python3 tools/build_host.py --sdl-include build/SDL/include --sdl-library /lib/x86_64-linux-gnu/libSDL2-2.0.so.0
```

O caminho da biblioteca varia por sistema; prefira libsdl2-dev/sdl2-config.
Compilação local: -std=c99 -O2 -Wall -Wextra -Werror.
Fontes SDL foram usadas apenas como headers; runtime é a biblioteca do sistema.

## Entrega e limites

[Arquivos S007](evidence/S007/README.md) incluem binário Linux x86-64 com PRG/CHR
embutidos, header gerado, dump, hashes e logs. Recompilar é o caminho recomendado
em outro host. Esse binário exige SDL2 e bibliotecas do sistema; NÃO é XEX,
NÃO roda no Xbox nem no Android. Nenhum teste humano interativo foi alegado.
Sem alteração no core validado pela comparação S006. Próximo trabalho: APU.
