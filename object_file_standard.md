# Proposta de Padrão para Arquivos Objeto (.o)

Esta especificação define o formato padrão para os arquivos objeto gerados pelo **Montador** (Passo 2) e consumidos pelo **Ligador/Carregador** no **ProjetoPS**.

Para fins acadêmicos e facilidade de depuração, o formato proposto é baseado em **texto plano (ASCII)** estruturado por seções marcadas com palavras-chave em maiúsculo. Isso simplifica a depuração visual e a implementação dos parsers em C++.

---

## 1. Estrutura Geral do Arquivo

O arquivo objeto `.o` é composto por 5 seções principais ordenadas:

1. **`HEADER`**: Metadados sobre o módulo.
2. **`EXTDEF`**: Tabela de símbolos definidos localmente que são exportados (públicos).
3. **`EXTREF`**: Tabela de símbolos externos referenciados pelo módulo.
4. **`REALOC`**: Lista de offsets dentro do código que contêm endereços internos e precisam ser realocados.
5. **`CODE`**: Bytes de código de máquina codificados em hexadecimal.

Linhas vazias e linhas iniciadas com `;` (comentários) devem ser ignoradas pelo leitor.

---

## 2. Descrição Detalhada das Seções

### Seção 1: HEADER
Contém as configurações básicas do módulo.
- `MODULE: <nome>` — Nome identificador único do módulo.
- `SIZE: <tamanho>` — Tamanho total do segmento de código/dados do módulo em bytes (representado em decimal ou hexadecimal `0x`).

Exemplo:
```text
HEADER
MODULE: modulo_math
SIZE: 24
```

### Seção 2: EXTDEF (Definições Externas)
Define os símbolos criados neste módulo que outros arquivos podem acessar.
- Formato: `<NomeDoSimbolo> <OffsetLocal>`
- O `OffsetLocal` (16 bits) é relativo ao início deste módulo (começando em `0x0000`).

Exemplo:
```text
EXTDEF
SOMA 0x0002
SUBTRAI 0x0014
```

### Seção 3: EXTREF (Referências Externas)
Identifica quais símbolos estão declarados em outros arquivos e em quais partes deste código eles estão sendo usados.
- Formato: `<NomeDoSimbolo> <Offset1> <Offset2> ...`
- Os offsets (16 bits) representam as posições exatas na seção de código deste módulo onde o endereço de 16 bits do símbolo externo deve ser embutido (substituindo placeholders `00 00`).

Exemplo:
```text
EXTREF
MULTIPLICA 0x0006
PRINT_HEX 0x000F 0x001A
```

### Seção 4: REALOC (Realocação)
Lista todos os offsets dentro da seção de código do módulo que contêm endereços absolutos locais (ex: o operando de instruções como `JP`, `CALL` ou `LD` apontando para labels internos do módulo). Quando o ligador mover o módulo no espaço de memória, o offset de carregamento do módulo deve ser somado a essas posições.
- Formato: Uma lista de inteiros em cada linha representando o offset.

Exemplo:
```text
REALOC
0x000B
0x001F
```

### Seção 5: CODE
Os bytes reais de instrução (código compilado) representados em strings hexadecimais de duas letras separadas por espaços para legibilidade.
- Formato: Bytes em hexadecimal.

Exemplo:
```text
CODE
3E 05 06 0A CD 00 00 80 76
```
*(Nota: No exemplo de `CODE`, a instrução na posição `0x0004` é `CD 00 00` correspondente a um `CALL` a um símbolo externo. O endereço está zerado (`00 00`), aguardando que o Ligador aplique o patch de `EXTREF` na posição `0x0005` e `0x0006`).*

---

## 3. Exemplo Completo de um Arquivo Objeto (`math.o`)

```text
; Arquivo objeto para modulo_math
HEADER
MODULE: math
SIZE: 15

EXTDEF
SOMA 0x0008

EXTREF
DIVIDE 0x0002

REALOC
0x000B

CODE
3E 05 CD 00 00 06 0C C3 0F 00 76
```

---

## 4. Vantagens Deste Modelo para o Grupo
- **Fácil Compartilhamento**: Cada integrante sabe o que deve escrever/ler.
- **Divisão Clara de Responsabilidades**:
  - **Matheus/Lucas/Celina** escrevem este arquivo no Montador.
  - **João/Celina** leem `SIZE` da seção `HEADER` para calcular o mapa de memória.
  - **Hiago** lê as linhas de `EXTDEF` para alimentar a GTS.
  - **Inácio** lê `EXTREF` e aplica os valores resolvidos nas posições apontadas da seção `CODE`.
  - **Augusto/Pedro** leem `REALOC` e aplicam a realocação aos bytes correspondentes da seção `CODE`.
