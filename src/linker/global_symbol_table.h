#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <stdexcept>

// Representa um símbolo exportado (EXTDEF) lido de um arquivo .o
struct ExtDef {
    std::string symbolName;
    uint16_t relativeAddress; // endereço relativo ao início do módulo
};

// Exceção específica do Ligador para erros como símbolos duplicados
class LinkerException : public std::runtime_error {
public:
    explicit LinkerException(const std::string& message) : std::runtime_error(message) {}
};

class GlobalSymbolTable {
public:
    /**
     * @brief  Registra os símbolos EXTDEF de um módulo na tabela global,
     *         calculando o endereço absoluto de cada um.
     *         Endereço absoluto = moduleStartAddress + relativeAddress.
     *
     * @param  moduleName         Nome do módulo exportador (para mensagens de erro).
     * @param  moduleStartAddress Endereço base onde o módulo foi alocado na memória.
     * @param  definitions        Lista de EXTDEFs lidas do arquivo .o.
     * @throws LinkerException    Se um símbolo já existe na tabela (duplicata global).
     */
    void registerModuleSymbols(
        const std::string& moduleName,
        uint16_t moduleStartAddress,
        const std::vector<ExtDef>& definitions
    );

    /**
     * @brief  Busca o endereço absoluto de um símbolo na tabela global.
     *
     * @param  symbolName  Nome do símbolo procurado.
     * @param  outAddress  Referência onde o endereço resolvido será armazenado.
     * @return             true se encontrou, false se o símbolo não existe na tabela.
     */
    bool lookupSymbol(const std::string& symbolName, uint16_t& outAddress) const;

    /**
     * @brief  Lê a seção EXTDEF de um arquivo .o no formato padrão do projeto.
     *         Ignora comentários (linhas com ';') e linhas vazias.
     *         Para ao encontrar outra seção (EXTREF, REALOC, CODE, HEADER).
     *
     * @param  objectFilePath  Caminho do arquivo .o a ser parseado.
     * @return                 Lista de ExtDef extraídas da seção EXTDEF.
     */
    static std::vector<ExtDef> parseExtDefsFromFile(const std::string& objectFilePath);

    /**
     * @brief  Retorna uma cópia do mapa interno (símbolo -> endereço absoluto).
     *         Útil para exibição na interface Qt ou depuração.
     *
     * @return Mapa com todos os símbolos registrados e seus endereços.
     */
    std::unordered_map<std::string, uint16_t> getTable() const;

    /**
     * @brief  Limpa toda a tabela de símbolos para uma nova rodada de ligação.
     */
    void clear();

    /**
     * @brief  Imprime a tabela global de símbolos na saída padrão (depuração).
     */
    void printTable() const;

private:
    // símbolo -> endereço absoluto na memória da VM (16 bits)
    std::unordered_map<std::string, uint16_t> symbolTable;
};
