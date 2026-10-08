# S006 — validação independente e correção de controle 2

Entrada: e99b80760d72387a8729b2fb9ddd3056cfa0bf22. Implementação no commit que
adiciona estes arquivos. Reproduzir: `python3 tools/compare_reference.py`.

- comparison.json: 600 idle e 6000 scripted, pixels e todos os bytes de RAM iguais.
- before-fix.json: pixels iguais, RAM diferente em 5895/6000 antes da correção.
- captures.tar.xz: capturas integrais de ambos os motores, logs CSV e compilação;
  inclui RAM/log do core anterior. Extraia em build, não sobre fontes.
- capture-hashes.json: SHA-256 de cada membro e do arquivo compactado.
- host-tests.txt / sanitizer-tests.txt: sete suítes bem-sucedidas.
- start-right.txt: runner tradicional 5m com hashes e hits de opcode.

Referência, licença, alinhamento, entradas e limites em
../../../tests/reference/README.md. Primeira diferença de RAM: captura 103,
$06fd e $074b, ambos FF no core versus 00 na referência. Não houve patch de RAM:
a correção genérica da leitura serial de $4017 eliminou todas as diferenças.
O segundo controle está solto no roteiro diferencial; testes sintéticos também
cobrem botões ativos, strobe, retenção, shift independente e writes APU.

Maior captura: 57.751.826 instruções, 178.680.628 ciclos, frame 5999, linha/dot
241/6, 5992 NMIs. O adaptador não exporta opcode hits; eles estão disponíveis
no runner tradicional de 5m (99 opcodes). Não inferir cobertura não medida.
Comparação não usa offsets, exclusões de RAM ou dados de outra ROM.

Compilação/execução local confirmadas; ASan/UBSan de testes e runner 5m passaram.
Não houve teste Xbox nem novo XEX. CI remoto não confirmado neste relatório.
