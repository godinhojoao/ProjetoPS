#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

// Representa uma linha já parseada do código assembly
struct AsmLine {
    std::string label;      // rótulo (sem ':'), vazio se não houver
    std::string operation;  // mnemônico (ex: LD, ADD, HALT...)
    std::string operands;   // tudo após o mnemônico (ex: "A, B")
    uint16_t    address;    // endereço de memória desta instrução
    uint8_t     size;       // tamanho em bytes desta instrução
    int         lineNumber; // número da linha original (para erros)
};

class Assembler {
public:
    /**
     * @brief  Executa o primeiro passo do montador de dois passos.
     *         Lê o código ASM (já expandido pelo MacroProcessor),
     *         calcula o endereço de cada instrução e monta a tabela de símbolos.
     *         Suporta o conjunto de instruções Z80 da primeira etapa e todos os
     *         modos de endereçamento: imediato, direto, indireto (HL/IX/IY),
     *         indexado (IX+d / IY+d) e implícito.
     *
     * @param  asmCode       Código assembly como string (uma instrução por linha).
     * @param  startAddress  Endereço inicial do programa (padrão 0x0000).
     * @return true se o primeiro passo foi concluído sem erros; false caso contrário.
     */
    bool firstPass(const std::string& asmCode, uint16_t startAddress = 0x0000);

    //futuramente vai existir uma secondPass
    std::string secondPass();

    //fução q executa os 2 passos. implementar certinho dps
    std::string assemble(const std::string& asmCode);

    /**
     * @brief  Retorna a tabela de símbolos construída no primeiro passo.
     *         Mapeamento: nome_do_rótulo -> endereço de memória.
     */
    const std::unordered_map<std::string, uint16_t>& getSymbolTable() const;

    /**
     * @brief  Retorna as linhas parseadas e anotadas com endereços e tamanhos.
     */
    const std::vector<AsmLine>& getLines() const;

    /**
     * @brief  Retorna a lista de erros encontrados no primeiro passo.
     */
    const std::vector<std::string>& getErrors() const;

    /**
     * @brief  Imprime a tabela de símbolos na saída padrão.
     */
    void printSymbolTable() const;

private:
    std::unordered_map<std::string, uint16_t> symbolTable;
    std::vector<AsmLine> lines;
    std::vector<std::string> errors;

    /**
     * @brief  Faz o parse de uma linha assembly, separando rótulo,
     *         operação e operandos. Remove comentários e espaços extras.
     *
     * @param  line       Linha de texto a ser parseada.
     * @param  lineNumber Número da linha no fonte (para mensagens de erro).
     * @return AsmLine preenchida (endereço e tamanho ainda não definidos).
     */
    AsmLine parseLine(const std::string& line, int lineNumber) const;

    /**
     * @brief  Calcula o tamanho em bytes de uma instrução Z80 a partir do
     *         mnemônico e dos operandos, conforme o conjunto de instruções
     *         definido na primeira etapa do trabalho.
     *
     * @param  operation  Mnemônico (ex: "LD", "ADD", "JP").
     * @param  operands   Operandos da instrução (ex: "A, B", "(IX+5)").
     * @return Número de bytes que a instrução ocupa na memória; 0 se não reconhecida.
     */
    uint8_t calcSize(const std::string& operation, const std::string& operands) const;
};
