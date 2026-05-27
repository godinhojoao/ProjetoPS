## NOMENCLATURA

- ### Classes e Structs
**PascalCase** (class Cpu, struct RegisterAblubleble)

- ### Funções, métodos e variáveis
**camelCase** (uint8_t readMemory, uint16_t programCounter)

- ### Constantes
**SCREAMING_SNAKE_CASE** (MEMORY_SIZE, NOP_OPCODE)

- ### Arquivos
**snake_case** (instructions_set.cpp) 

## INDENTAÇÃO / ESPAÇAMENTOS 
- Identação de 4 espaços. (Mudar na IDE pra usar 4 espaços em vez de tab (Vscode é no canto inferior direito))

- Chaves vão em linhas próprias (K&R). 
Exemplo:
void exemplo() {
...
}

- Espaço ao redor de operadores. 
Exemplo: 
int total = a + b;


- Use uma linha em branco pra separar blocos dentro de uma função e funções. 
Exemplo:
void Exemplo() {
  int a = 0;
  int b = 2;

  int c = a + b;
}

void Exemplo2() {
}

## TIPOS DE DADOS
Como o Z80 tem processador de 8 bits e barramento de 16 bits, não vamos utilizar tipo int (com exceção de interface a princípio). Vamos utilizar tipos específicos da biblioteca <cstdint>.
uint8_t, int8_t, uint16_t, int16_t, uint32_t, bool. 

## COMENTÁRIOS 
- Procure sempre comentar. Comente o porquê e dê uma breve explicação se necessário.

- As funções públicas nos arquivos .h devem ter documentação da seguinte forma:

/**
 * @brief  Faz xyz
 *
 * @param  address  Endereço de 16 bits.
 * @return          Byte armazenado naquele endereço.
 */
uint8_t read(uint16_t address);

## COMMITS
- Commitar da forma: **{tipo}({escopo}): {descrição}**

Exemplo: feat(cpu): implementa função x

- Procure fazer commits menores e constantes.
- Não façam commits grandes com mais de uma função. 
- Cuide o que for commitar, não adicione arquivos desnecessários. 
- Antes de commitar, dê pull e se puder afetar outro módulo do trabalho, verifique antes com atenção. 
---
**Tipos:**
- *feat*        Funcionalidade
- *fix*         Correção de bug
- *refactor*    Reorganização de código sem alterar lógica
- *test*
- *docs*        Alteração em documentação/comentários
- *style*       Ajuste de formação sem alterar lógica
- *chore*       Configurações, build, arquivos auxiliares
- *wip*         Trabalho em andamento

**Escopos:**
- cpu
- memory
- stack
- instructions
- flow
- gui
- docs
- project
---