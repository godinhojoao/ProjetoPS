# Loader

Módulo responsável por carregar código já montado na memória da VM. São
dois loaders, um por formato de entrada:

| Loader         | Entrada | Realoca? | Arquivos                       |
| -------------- | ------- | -------- | ------------------------------ |
| `BinaryLoader` | `.bin`  | não      | `src/loader/binary_loader.*`   |
| `ObjectLoader` | `.o`    | sim      | `src/loader/object_loader.*`   |

O `BinaryLoader` não interpreta nada: só transfere bytes do disco para
`Memory`. Use-o quando o programa é carregado sempre no mesmo endereço.

O `ObjectLoader` lê o `.o` realocável (formato em
[`object_file_standard.md`](./object_file_standard.md)), copia o código
para um endereço de carga qualquer e ajusta os endereços absolutos —
é o **Cenário A** da realocação. Veja a seção
[Loader de arquivo objeto](#loader-de-arquivo-objeto-o) abaixo.

## Formato do `.bin`

Sequência crua de bytes (`uint8_t`), sem cabeçalho, na mesma ordem em que
serão gravados em memória. Cada byte é uma palavra de 8 bits (opcode ou
operando) conforme a tabela de instruções em [`vm.md`](./vm.md).

Exemplo (programa de 4 bytes):

| Mnemônico    | Bytes        | Endereço |
| ------------ | ------------ | -------- |
| `LD A, 0x42` | `0x3E 0x42`  | 0x0000   |
| `ADD A, B`   | `0x80`       | 0x0002   |
| `HLT`       | `0x76`       | 0x0003   |

## API

```cpp
class BinaryLoader
{
public:
    uint32_t load(const std::string& path, Memory& memory, uint16_t startAddr = 0x0000);
};
```

- `path`      — caminho do `.bin`.
- `memory`    — referência à memória da VM (64 KB).
- `startAddr` — endereço inicial de gravação. Padrão `0x0000` (início do
  segmento de código, conforme o layout em `vm.md`).
- Retorno: número de bytes carregados; `0` se houve falha.

Casos de falha (retornam 0 e imprimem erro em `stderr`):
- arquivo não existe ou não pôde ser aberto;
- arquivo vazio;
- conteúdo não cabe em `65536 - startAddr` bytes.

## Como usar

### Pela VM

```cpp
VM vm;
if (!vm.load("programa.bin")) return 1;
vm.run();
```

### Pela linha de comando

```sh
make run ARGS=programa.bin
```

Ou diretamente:

```sh
./vm programa.bin
```

Se nenhum `.bin` for passado, a VM roda sem programa carregado (útil
enquanto o fetch-decode-execute não está pronto).

## Como testar

Suíte standalone (não depende de CPU/decoder/executor):

```sh
make test-loader
```

Cobre quatro casos: load em `0x0000`, load com offset, arquivo
inexistente, e overflow de memória.

## Loader de arquivo objeto (`.o`)

O montador/ligador emite código com endereços **relativos** ao início do
módulo, mais uma seção `REALOC` listando onde esses endereços estão. O
`ObjectLoader` carrega esse módulo em qualquer endereço e soma o endereço
de carga aos operandos apontados por `REALOC`.

Exemplo — `CALL 0x0008` montado no offset `0x0002` do módulo:

```text
CODE    3E 05 CD 08 00 ...     ; operando 0x0008 em little-endian
REALOC  0x0003                 ; offset do operando dentro do CODE
```

Carregado em `0x0100`, o operando vira `0x0108` e o `CALL` acerta o alvo.
Bytes fora da tabela `REALOC` (opcodes, imediatos como o `0x05` do
`LD A, 5`) não são tocados — por isso a tabela existe: não dá pra
distinguir endereço de dado só olhando os bytes.

### API

```cpp
struct ObjectFile
{
  std::string moduleName;              // HEADER: MODULE
  uint16_t declaredSize;               // HEADER: SIZE
  std::vector<uint8_t> code;           // seção CODE
  std::vector<uint16_t> relocOffsets;  // seção REALOC
};

class ObjectLoader
{
public:
  bool parse(const std::string& path, ObjectFile& obj);
  static bool relocate(ObjectFile& obj, uint16_t loadAddr);
  uint32_t load(const std::string& path, Memory& memory, uint16_t loadAddr = 0x0000);
};
```

`load` = `parse` + `relocate` + cópia para a memória. Retorna a quantidade
de bytes gravados, ou `0` em qualquer falha (com erro em `stderr`):

- arquivo inexistente, sem seção `CODE`, ou `CODE` vazia;
- `SIZE` menor que o tamanho real do `CODE`;
- byte hexadecimal ou offset malformado (via `Shared::tryParseNumber`, que
  não lança exceção — ler um `.o` corrompido não pode derrubar a VM);
- offset de `REALOC` apontando fora do `CODE`;
- endereço realocado passando de `0xFFFF`;
- código não cabe em `65536 - loadAddr` bytes.

As seções `EXTDEF`/`EXTREF` são reconhecidas e ignoradas: quem resolve
símbolo externo é o ligador (`src/linker/extref_resolver.*`). Campos
desconhecidos no `HEADER` também são ignorados, pra o `.o` poder ganhar
metadados sem quebrar loaders antigos.

A realocação acontece no buffer, **antes** da cópia para a memória.
`Memory::write` recusa escrita abaixo de `codeEnd`, então corrigir bytes
já gravados dependeria da ordem em que a VM chama `setCodeEnd`.

### Pela VM

`VM::loadObject` carrega, realoca e aponta o `PC` para o início do módulo:

```cpp
VM vm;
if (!vm.loadObject("programa.o", 0x0100)) return 1;
vm.run();   // PC começa em 0x0100
```

### Como testar

```sh
make test-object-loader
```

Dez casos: parse do `.o`, carga em `0x0000` (realocação identidade),
realocação em `0x0100`, offset fora do código, estouro de 16 bits, hex
inválido, hex com lixo no fim, `SIZE` inconsistente, arquivo inexistente e
overflow de memória.

## Pipeline

```
.bin no disco                     .o no disco (CODE + REALOC)
   │                                 │
   ▼                                 ▼
[ BinaryLoader ]                  [ ObjectLoader ]
   │ escreve em Memory               │ realoca operandos de REALOC
   │ a partir de startAddr           │ e escreve a partir de loadAddr
   └────────────┬────────────────────┘
                ▼
          [ VM loop ]    Fetch (usa PC) → Decode → Execute
```

## Pendências fora do escopo do loader

- Quem **gera** a seção `REALOC` é o montador/ligador (Cenário B): ele
  precisa identificar os opcodes com operando absoluto (`JP`, `CALL`,
  `LD` de/para endereço) e anotar os offsets. O `ObjectLoader` só consome
  essa tabela — se ela vier incompleta, o programa realocado quebra em
  runtime e não na carga.
- `Linker::writeObj`/`readObj` (`src/linker/linker.cpp`) ainda escrevem um
  formato binário próprio (magic `Z80L`), que não carrega `EXTDEF`/`EXTREF`
  e não é lido por ninguém. O contrato do grupo é o `.o` texto de
  `object_file_standard.md`; esse código binário deve ser migrado ou
  removido pelo owner do ligador.
- `Shared::parseAddress` chama `std::stoul` sem `try/catch`: um offset
  malformado num `.o` faz o `extref_resolver` lançar `std::invalid_argument`
  e derrubar o processo. O loader usa `Shared::tryParseNumber` justamente
  por isso; o ligador deveria migrar também.
- O decoder (`src/instruction_set/decoder.*`) é quem interpreta os bytes
  carregados — responsabilidade de outro módulo.
