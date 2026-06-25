#include "assembler.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// funcoes auxiliares
static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

// Remove espaços e tabulações no início e fim da string
static std::string trimSpaces(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Remove comentários inline (tudo a partir de ';')
static std::string stripComment(const std::string& s) {
    size_t pos = s.find(';');
    return (pos == std::string::npos) ? s : s.substr(0, pos);
}

// Retorna true se o token é um registrador de 8 bits do Z80
static bool isRegister(const std::string& token) {
    std::string t = toUpper(token);
    return (t == "A" || t == "B" || t == "C" ||
            t == "D" || t == "E" || t == "H" || t == "L");
}

// Retorna true se o operando usa endereçamento indexado: (IX+d) ou (IY+d)
static bool isIndexed(const std::string& token) {
    std::string t = toUpper(token);
    return (t.find("IX") != std::string::npos ||
            t.find("IY") != std::string::npos) &&
            t.find('(') != std::string::npos;
}

// Retorna true se o operando é indireto via HL: (HL)
static bool isHLIndirect(const std::string& token) {
    return toUpper(token) == "(HL)";
}

// parser de linha
AsmLine Assembler::parseLine(const std::string& rawLine, int lineNumber) const {
    AsmLine parsed;
    parsed.lineNumber = lineNumber;
    parsed.address    = 0;
    parsed.size       = 0;

    std::string line = trimSpaces(stripComment(rawLine));
    if (line.empty()) return parsed;

    // Verifica se a linha começa com um rótulo (token terminado em ':')
    // O rótulo pode estar sozinho na linha ou seguido de uma instrução
    std::string remaining = line;

    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        std::string possibleLabel = trimSpaces(line.substr(0, colonPos));

        // Rótulo válido não pode conter espaços
        bool validLabel = !possibleLabel.empty() &&
                          possibleLabel.find(' ')  == std::string::npos &&
                          possibleLabel.find('\t') == std::string::npos;

        if (validLabel) {
            parsed.label = possibleLabel;
            remaining = trimSpaces(line.substr(colonPos + 1)); // resto após ':'
        }
    }

    if (remaining.empty()) return parsed; // linha só com rótulo

    // Separa mnemônico dos operandos pelo primeiro espaço ou tabulação
    size_t spacePos = remaining.find_first_of(" \t");
    if (spacePos == std::string::npos) {
        // Só mnemônico, sem operandos (ex: NOP, HALT, RET)
        parsed.operation = toUpper(trimSpaces(remaining));
    } else {
        parsed.operation = toUpper(trimSpaces(remaining.substr(0, spacePos)));
        parsed.operands  = trimSpaces(remaining.substr(spacePos + 1));
    }

    return parsed;
}

// calculo de tamanho de instrucao
// Conjunto de instruções conforme definido na etapa 1 do trabalho.
// Referência de tamanhos: Z80 CPU User Manual (Zilog).
//
// Modos de endereçamento suportados:
//   Implícito      -> NOP, HALT, RET, PUSH, POP, INC, DEC  (1 byte)
//   Imediato       -> LD r, n  (2 bytes)
//   Direto         -> JP nn, CALL nn  (3 bytes)
//   Indireto HL    -> LD r,(HL) / LD (HL),r  (1 byte)
//   Indireto IX/IY -> LD r,(IX+d) / LD r,(IY+d)  (3 bytes)
//   Indexado       -> LD (IX+d),r / ADD A,(IX+d)  (3 bytes)
uint8_t Assembler::calcSize(const std::string& operation, const std::string& operands) const {
    if (operation == "NOP"  ||
        operation == "HALT" ||
        operation == "RET") {
        return 1;
    }

    // PUSH rp / POP rp — registrador de par (BC/DE/HL/AF) -> 1 byte
    if (operation == "PUSH" || operation == "POP") return 1;

    // INC r / DEC r -> 1 byte
    if (operation == "INC" || operation == "DEC") return 1;

    // LD: tamanho depende do modo de endereçamento
    //   LD r, r'     -> 1 byte  (registrador para registrador)
    //   LD (HL), r   -> 1 byte  (indireto via HL)
    //   LD r, (HL)   -> 1 byte  (indireto via HL)
    //   LD r, n      -> 2 bytes (imediato de 8 bits)
    //   LD r, (IX+d) -> 3 bytes (indexado: prefixo DD + opcode + deslocamento)
    //   LD r, (IY+d) -> 3 bytes (indexado: prefixo FD + opcode + deslocamento)
    //   LD (IX+d), r -> 3 bytes (indexado)
    //   LD (IY+d), r -> 3 bytes (indexado)
    //   LD A, (nn)   -> 3 bytes (direto: 0x3A + nn low + nn high)
    //   LD (nn), A   -> 3 bytes (direto: 0x32 + nn low + nn high)
    if (operation == "LD") {
        size_t comma = operands.find(',');
        std::string dest = trimSpaces(operands.substr(0, comma));
        std::string src  = (comma != std::string::npos)
                           ? trimSpaces(operands.substr(comma + 1)) : "";

        // Endereçamento indexado (IX+d) ou (IY+d) em qualquer lado -> 3 bytes
        if (isIndexed(dest) || isIndexed(src)) return 3;

        // Endereçamento direto: "(nn)" onde nn é um endereço de 16 bits
        // (não é um registrador de par nem IX/IY)
        auto isDirectAddr = [](const std::string& op) -> bool {
            if (op.empty() || op[0] != '(') return false;
            std::string inner = toUpper(trimSpaces(op.substr(1, op.size() - 2)));
            return inner != "HL"  && inner != "BC" &&
                   inner != "DE"  && inner != "AF" &&
                   inner.find("IX") == std::string::npos &&
                   inner.find("IY") == std::string::npos;
        };
        if (isDirectAddr(dest) || isDirectAddr(src)) return 3;

        // Imediato de 8 bits (LD r, n): fonte não é registrador nem indireção
        if (!src.empty() && !isRegister(src) && src[0] != '(') return 2;

        // LD r,r' | LD (HL),r | LD r,(HL) -> 1 byte
        return 1;
    }

    // ADD A, r | SUB r | AND r | OR r | XOR r | CP r
    //   com registrador ou (HL) -> 1 byte
    //   com (IX+d) / (IY+d)    -> 3 bytes
    //   com imediato n          -> 2 bytes
    if (operation == "ADD" || operation == "SUB" || operation == "AND" ||
        operation == "OR"  || operation == "XOR" || operation == "CP") {
        size_t comma = operands.find(',');
        // ADD tem "A, r" então a fonte é após a vírgula; SUB/AND/etc. é direto
        std::string src = (comma != std::string::npos)
                          ? trimSpaces(operands.substr(comma + 1))
                          : trimSpaces(operands);

        if (isIndexed(src))    return 3; // (IX+d) / (IY+d)
        if (isHLIndirect(src)) return 1; // (HL)
        if (isRegister(src))   return 1; // registrador simples
        return 2;                        // imediato de 8 bits
    }

    // JP nn | CALL nn -> 3 bytes (endereçamento direto: opcode + nn low + nn high)
    if (operation == "JP" || operation == "CALL") return 3;

    // JR offset -> 2 bytes (opcode + deslocamento signed de 8 bits)
    if (operation == "JR") return 2;

    // Instrução não reconhecida -> 0 (será reportado como erro)
    return 0;
}

// primeiro passo
bool Assembler::firstPass(const std::string& asmCode, uint16_t startAddress) {
    lines.clear();
    symbolTable.clear();
    errors.clear();

    std::istringstream stream(asmCode);
    std::string rawLine;
    int lineNumber = 0;
    uint16_t lc = startAddress; // contador de localização (LC)

    while (std::getline(stream, rawLine)) {
        lineNumber++;

        AsmLine asmLine = parseLine(rawLine, lineNumber);

        // Ignora linhas vazias ou só com comentário
        if (asmLine.operation.empty() && asmLine.label.empty()) continue;

        // Define o endereço desta linha como o valor atual do LC
        asmLine.address = lc;

        // Registra o rótulo na tabela de símbolos com o endereço atual do LC
        if (!asmLine.label.empty()) {
            if (symbolTable.count(asmLine.label)) {
                errors.push_back("Linha " + std::to_string(lineNumber) +
                                 ": rotulo duplicado '" + asmLine.label + "'");
                return false;
            }
            symbolTable[asmLine.label] = lc;
        }

        // Linha só com rótulo (sem instrução): não avança o LC
        if (asmLine.operation.empty()) {
            lines.push_back(asmLine);
            continue;
        }

        // Calcula o tamanho da instrução e avança o contador de localização
        uint8_t size = calcSize(asmLine.operation, asmLine.operands);
        if (size == 0) {
            errors.push_back("Linha " + std::to_string(lineNumber) +
                             ": instrucao desconhecida '" + asmLine.operation + "'");
            return false;
        }

        asmLine.size = size;
        lc += size;

        lines.push_back(asmLine);
    }

    return errors.empty();
}

std::string Assembler::secondPass() {
    //gera binário e retorna o path
    return "";
}

//retorna o path do binário gerado
std::string Assembler::assemble(const std::string& asmCode) {
    firstPass(asmCode, 0);
    secondPass();
    return "";
}

// getters
const std::unordered_map<std::string, uint16_t>& Assembler::getSymbolTable() const {
    return symbolTable;
}

const std::vector<AsmLine>& Assembler::getLines() const {
    return lines;
}

const std::vector<std::string>& Assembler::getErrors() const {
    return errors;
}

// saida de depuracao
void Assembler::printSymbolTable() const {
    std::cout << "\n=== Tabela de Simbolos ===\n";
    if (symbolTable.empty()) {
        std::cout << "(vazia)\n";
        return;
    }
    for (const auto& [label, address] : symbolTable) {
        std::cout << "  " << label << " -> 0x"
                  << std::hex << std::uppercase << address
                  << std::dec << "\n";
    }
    std::cout << "==========================\n";
}
