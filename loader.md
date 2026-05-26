# Loader

Módulo responsável por carregar um arquivo binário (`.bin`) — já montado
pelo assembler — direto na memória da VM. O loader não interpreta nada:
só transfere bytes do disco para `Memory`.

Arquivos: `src/loader/binary_loader.h`, `src/loader/binary_loader.cpp`.

## Formato do `.bin`

Sequência crua de bytes (`uint8_t`), sem cabeçalho, na mesma ordem em que
serão gravados em memória. Cada byte é uma palavra de 8 bits (opcode ou
operando) conforme a tabela de instruções em [`vm.md`](./vm.md).

Exemplo (programa de 4 bytes):

| Mnemônico    | Bytes        | Endereço |
| ------------ | ------------ | -------- |
| `LD A, 0x42` | `0x3E 0x42`  | 0x0000   |
| `ADD A, B`   | `0x80`       | 0x0002   |
| `HALT`       | `0x76`       | 0x0003   |

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

## Pipeline

```
.bin no disco
   │
   ▼
[ Loader ] ──► escreve em Memory a partir de startAddr
   │
   ▼
[ VM loop ]    Fetch (usa PC) → Decode → Execute
```

## Pendências fora do escopo do loader

- O `VM::run()` atual ainda é um stub e sobrescreve `mem[0x0000]` com
  `0x42` como demo. Quando o owner do módulo VM implementar o ciclo
  fetch-decode-execute essa escrita sairá e o byte do programa carregado
  vai prevalecer.
- O decoder (`src/instruction_set/decoder.*`) é quem interpreta os bytes
  carregados — responsabilidade de outro módulo.
