#include "assembler.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include "../shared/shared.h"

// funcoes auxiliares
static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
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

// auxiliares segunda passada

// obtém código do registrador
static uint8_t getRegCode(const std::string& reg) {
    std::string r = toUpper(Shared::trim(reg));
    if (r == "B") return 0b000;
    if (r == "C") return 0b001;
    if (r == "D") return 0b010;
    if (r == "E") return 0b011;
    if (r == "H") return 0b100;
    if (r == "L") return 0b101;
    if (r == "(HL)") return 0b110;
    if (r == "A") return 0b111;
    return 0; 
}

// obtém código do registrador par
static uint8_t getRegPairCode(const std::string& reg) {
    std::string r = toUpper(Shared::trim(reg));
    if (r == "BC") return 0b00;
    if (r == "DE") return 0b01;
    if (r == "HL") return 0b10;
    if (r == "AF" || r == "SP") return 0b11; 
    return 0;
}

// extrai deslocamento de endereço indexado
static int8_t extractOffset(const std::string& op, std::vector<std::string>& errors, int lineNum) {
    size_t plus = op.find('+');
    size_t minus = op.find('-');
    size_t closeParen = op.find(')');
    if (closeParen == std::string::npos) closeParen = op.length();

    try {
        // verifica se tem '+'
        if (plus != std::string::npos) {
            std::string valStr = Shared::trim(op.substr(plus + 1, closeParen - plus - 1));
            return (int8_t)std::stoi(valStr, nullptr, 0);
        }
        // verifica se tem '-'
        if (minus != std::string::npos) {
            std::string valStr = Shared::trim(op.substr(minus + 1, closeParen - minus - 1));
            return -(int8_t)std::stoi(valStr, nullptr, 0);
        }
    } catch (...) {
        errors.push_back("Linha " + std::to_string(lineNum) + ": Deslocamento de índice inválido '" + op + "'");
    }
    return 0;
}

// resolve endereço/valor
static uint16_t resolveValue(const std::string& op, const std::unordered_map<std::string, uint16_t>& symTab, std::vector<std::string>& errors, int lineNum) {
    if (symTab.count(op)) return symTab.at(op);
    try {
        if (op.size() > 2 && op[0] == '0' && (op[1] == 'x' || op[1] == 'X')) {
            return (uint16_t)std::stoi(op, nullptr, 16);
        }
        return (uint16_t)std::stoi(op, nullptr, 0);
    } catch (...) {
        errors.push_back("Linha " + std::to_string(lineNum) + ": Símbolo indefinido ou valor inválido '" + op + "'");
        return 0;
    }
}
// Fim auxiliares

// parser de linha
AsmLine Assembler::parseLine(const std::string& rawLine, int lineNumber) const {
    AsmLine parsed;
    parsed.lineNumber = lineNumber;
    parsed.address    = 0;
    parsed.size       = 0;

    std::string line = Shared::trim(rawLine);
    if (line.empty()) return parsed;

    // Verifica se a linha começa com um rótulo (token terminado em ':')
    // O rótulo pode estar sozinho na linha ou seguido de uma instrução
    std::string remaining = line;

    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        std::string possibleLabel = Shared::trim(line.substr(0, colonPos));

        // Rótulo válido não pode conter espaços
        bool validLabel = !possibleLabel.empty() &&
                          possibleLabel.find(' ')  == std::string::npos &&
                          possibleLabel.find('\t') == std::string::npos;

        if (validLabel) {
            parsed.label = possibleLabel;
            remaining = Shared::trim(line.substr(colonPos + 1)); // resto após ':'
        }
    }

    if (remaining.empty()) return parsed; // linha só com rótulo

    // Separa mnemônico dos operandos pelo primeiro espaço ou tabulação
    size_t spacePos = remaining.find_first_of(" \t");
    if (spacePos == std::string::npos) {
        // Só mnemônico, sem operandos (ex: NOP,HLT, RET)
        parsed.operation = toUpper(Shared::trim(remaining));
    } else {
        parsed.operation = toUpper(Shared::trim(remaining.substr(0, spacePos)));
        parsed.operands  = Shared::trim(remaining.substr(spacePos + 1));
    }

    return parsed;
}

// calculo de tamanho de instrucao
// Referência de tamanhos: Z80 CPU User Manual (Zilog).
//
// Modos de endereçamento suportados:
//   Implícito      -> NOP,HLT, RET, PUSH, POP, INC, DEC  (1 byte)
//   Imediato       -> LD r, n  (2 bytes)
//   Direto         -> JP nn, CALL nn  (3 bytes)
//   Indireto HL    -> LD r,(HL) / LD (HL),r  (1 byte)
//   Indireto IX/IY -> LD r,(IX+d) / LD r,(IY+d)  (3 bytes)
//   Indexado       -> LD (IX+d),r / ADD A,(IX+d)  (3 bytes)
uint8_t Assembler::calcSize(const std::string& operation, const std::string& operands) const {
    // DIRETIVAS DE MONTAGEM (PSEUDO-INSTRUÇÕES)
    if (operation == "ORG" || operation == "EQU" || operation == "END") {
        return 0; // Não geram código de máquina diretamente
    }
    if (operation == "DB" || operation == "CONST")  // define byte
    {
        if (operands.empty()) return 0;
        uint8_t count = 1;
        for (char c : operands) {
            if (c == ',') count++;
        }
        return count; // 1 byte para cada operando separado por vírgula
    }

    // define tamanho para DW(define word)
    if (operation == "DW") {
        if (operands.empty()) return 0;
        uint8_t count = 1;
        for (char c : operands) {
            if (c == ',') count++;
        }
        return count * 2; // 2 bytes para cada valor de 16 bits
    }
    
    // define tamanho para DS(define space)
    if (operation == "DS" || operation == "SPACE") {
        if (operands.empty()) return 0;
        try {
            int size = std::stoi(operands, nullptr, 0);
            return size > 0 ? static_cast<uint8_t>(size) : 0;
        } catch (...) {
            // Se for uma constante definida anteriormente por EQU, busca na tabela
            if (symbolTable.count(operands)) {
                return static_cast<uint8_t>(symbolTable.at(operands));
            }
            return 0;
        }
    }

    // instruções de 1 byte
    if (operation == "NOP"  ||
        operation == "HLT" ||
        operation == "RET") {
        return 1;
    }

    //PUSH rp / POP rp — registrador de par (BC/DE/HL/AF) -> 1 byte
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
        std::string dest = Shared::trim(operands.substr(0, comma));
        std::string src  = (comma != std::string::npos)
                           ? Shared::trim(operands.substr(comma + 1)) : "";

        // Endereçamento indexado (IX+d) ou (IY+d)
        if (isIndexed(dest) || isIndexed(src)) {
            // LD (IX+d), n ocupa 4 bytes (DD 36 d n)
            if (isIndexed(dest) && !isRegister(src) && src != "(HL)" && !src.empty() && src[0] != '(') {
                return 4;
            }
            return 3;
        }

        // Endereçamento direto: "(nn)" onde nn é um endereço de 16 bits
        // (não é um registrador de par nem IX/IY)
        auto isDirectAddr = [](const std::string& op) -> bool {
            if (op.empty() || op[0] != '(') return false;
            std::string inner = toUpper(Shared::trim(op.substr(1, op.size() - 2)));
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
                          ? Shared::trim(operands.substr(comma + 1))
                          : Shared::trim(operands);

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

        // Se for a diretiva ORG, atualiza o LC imediatamente antes de processar rótulos/endereços
        // porque ORG define o endereço de todos os rótulos seguintes
        if (asmLine.operation == "ORG") {
            try {
                lc = resolveValue(asmLine.operands, symbolTable, errors, lineNumber);
            } catch (...) {
                errors.push_back("Linha " + std::to_string(lineNumber) + ": Endereço de ORG inválido '" + asmLine.operands + "'");
                return false;
            }
        }

        // Define o endereço desta linha como o valor atual do LC
        asmLine.address = lc;

        // Registra o rótulo na tabela de símbolos
        if (!asmLine.label.empty()) {
            if (symbolTable.count(asmLine.label)) {
                errors.push_back("Linha " + std::to_string(lineNumber) +
                                 ": rotulo duplicado '" + asmLine.label + "'");
                return false;
            }
            
            // Se for EQU, associa o rótulo ao valor do operando
            if (asmLine.operation == "EQU") {
                try {
                    symbolTable[asmLine.label] = resolveValue(asmLine.operands, symbolTable, errors, lineNumber);
                } catch (...) {
                    errors.push_back("Linha " + std::to_string(lineNumber) + ": Valor inválido para EQU '" + asmLine.operands + "'");
                    return false;
                }
            } else {
                symbolTable[asmLine.label] = lc;
            }
        }

        // Se for END, encerra a primeira passada aqui
        if (asmLine.operation == "END") {
            lines.push_back(asmLine);
            break;
        }

        // Linha só com rótulo (sem instrução): não avança o LC
        if (asmLine.operation.empty()) {
            lines.push_back(asmLine);
            continue;
        }

        // Calcula o tamanho da instrução/diretiva e avança o contador de localização
        uint8_t size = calcSize(asmLine.operation, asmLine.operands);
        if (size == 0 && asmLine.operation != "EQU" && asmLine.operation != "ORG") {
            errors.push_back("Linha " + std::to_string(lineNumber) +
                             ": instrucao ou diretiva desconhecida '" + asmLine.operation + "'");
            return false;
        }

        asmLine.size = size;
        lc += size;

        lines.push_back(asmLine);
    }

    return errors.empty();
}

std::string Assembler::secondPass() {
    std::vector<uint8_t> output;

    uint16_t initialAddress = lines.empty() ? 0 : lines.front().address;

    for (const auto& line : lines) {
        if (line.operation.empty()) continue;

        std::string op = line.operation;
        std::string args = line.operands;
        
        // Trata lacunas de endereçamento (gaps) geradas por ORG na memória preenchendo com zeros
        uint16_t expectedAddress = initialAddress + output.size();
        if (line.address > expectedAddress) {
            uint16_t padding = line.address - expectedAddress;
            for (uint16_t i = 0; i < padding; ++i) {
                output.push_back(0x00);
            }
            expectedAddress = line.address; // atualiza expected
        } else if (line.address < expectedAddress && op != "ORG" && op != "EQU" && op != "END") {
            // Se o endereço da linha atual for menor que o esperado por conta de um ORG para trás que sobrescreve memória
            errors.push_back("Linha " + std::to_string(line.lineNumber) + 
                             ": Endereco de memoria sobreposto ou ORG invalido detectado.");
            continue;
        }

        // --- PROCESSAMENTO DE DIRETIVAS DE MONTAGEM ---
        if (op == "ORG" || op == "EQU") {
            continue; // Apenas diretivas de controle, ignoradas no output binário direto
        }
        if (op == "END") {
            break; // Finaliza a montagem
        }
        if (op == "DB" || op == "CONST") {
            std::stringstream ss(args);
            std::string token;
            while (std::getline(ss, token, ',')) {
                uint16_t val = resolveValue(Shared::trim(token), symbolTable, errors, line.lineNumber);
                output.push_back(static_cast<uint8_t>(val & 0xFF));
            }
            continue;
        }
        if (op == "DW") {
            std::stringstream ss(args);
            std::string token;
            while (std::getline(ss, token, ',')) {
                uint16_t val = resolveValue(Shared::trim(token), symbolTable, errors, line.lineNumber);
                output.push_back(val & 0xFF);
                output.push_back((val >> 8) & 0xFF);
            }
            continue;
        }
        if (op == "DS" || op == "SPACE") {
            uint16_t size = resolveValue(args, symbolTable, errors, line.lineNumber);
            for (uint16_t i = 0; i < size; ++i) {
                output.push_back(0x00);
            }
            continue;
        }

        size_t comma = args.find(',');
        std::string arg1 = (comma != std::string::npos) ? Shared::trim(args.substr(0, comma)) : Shared::trim(args);
        std::string arg2 = (comma != std::string::npos) ? Shared::trim(args.substr(comma + 1)) : "";

        if (op == "NOP") {
            output.push_back(0x00);
        } else if (op == "HLT") {
            output.push_back(0x76);
        } else if (op == "RET") {
            output.push_back(0xC9);
        } else if (op == "PUSH" || op == "POP") {
            uint8_t base = (op == "PUSH") ? 0b11000101 : 0b11000001;
            uint8_t pp = getRegPairCode(arg1);
            output.push_back(base | (pp << 4));
        } else if (op == "INC" || op == "DEC") {
            uint8_t base = (op == "INC") ? 0b00000100 : 0b00000101;
            uint8_t reg = getRegCode(arg1);
            output.push_back(base | (reg << 3));
        } else if (op == "LD") {
            bool arg1Idx = isIndexed(arg1);
            bool arg2Idx = isIndexed(arg2);
            
            if (arg1Idx || arg2Idx) {
                std::string idxStr = arg1Idx ? arg1 : arg2;
                std::string regStr = arg1Idx ? arg2 : arg1;
                bool isIX = (toUpper(idxStr).find("IX") != std::string::npos);
                uint8_t prefix = isIX ? 0xDD : 0xFD;
                
                if (arg1Idx) { 
                    if (isRegister(regStr) || regStr == "(HL)") {
                        uint8_t r = getRegCode(regStr);
                        output.push_back(prefix);
                        output.push_back(0b01110000 | r); 
                        output.push_back((uint8_t)extractOffset(idxStr, errors, line.lineNumber));
                    } else {
                        output.push_back(prefix);
                        output.push_back(0x36);
                        output.push_back((uint8_t)extractOffset(idxStr, errors, line.lineNumber));
                        output.push_back((uint8_t)resolveValue(regStr, symbolTable, errors, line.lineNumber));
                    }
                } else { 
                    uint8_t r = getRegCode(arg1);
                    output.push_back(prefix);
                    output.push_back(0b01000110 | (r << 3)); 
                    output.push_back((uint8_t)extractOffset(idxStr, errors, line.lineNumber));
                }
            } else {
                auto isDirectAddr = [](const std::string& opr) -> bool {
                    if (opr.empty() || opr[0] != '(') return false;
                    std::string inner = toUpper(Shared::trim(opr.substr(1, opr.size() - 2)));
                    return inner != "HL" && inner != "BC" && inner != "DE" && inner != "AF";
                };

                if (isDirectAddr(arg1)) { 
                    output.push_back(0x32);
                    std::string addrStr = Shared::trim(arg1.substr(1, arg1.size() - 2));
                    uint16_t addr = resolveValue(addrStr, symbolTable, errors, line.lineNumber);
                    output.push_back(addr & 0xFF);
                    output.push_back((addr >> 8) & 0xFF);
                } else if (isDirectAddr(arg2)) { 
                    output.push_back(0x3A);
                    std::string addrStr = Shared::trim(arg2.substr(1, arg2.size() - 2));
                    uint16_t addr = resolveValue(addrStr, symbolTable, errors, line.lineNumber);
                    output.push_back(addr & 0xFF);
                    output.push_back((addr >> 8) & 0xFF);
                } else if (!isRegister(arg2) && arg2 != "(HL)") { 
                    uint8_t r = getRegCode(arg1);
                    output.push_back(0b00000110 | (r << 3)); 
                    output.push_back((uint8_t)resolveValue(arg2, symbolTable, errors, line.lineNumber));
                } else { 
                    uint8_t dest = getRegCode(arg1);
                    uint8_t src = getRegCode(arg2);
                    output.push_back(0b01000000 | (dest << 3) | src);
                }
            }
        } else if (op == "ADD" || op == "SUB" || op == "AND" || op == "OR" || op == "XOR" || op == "CP") {
            std::string srcStr = (op == "ADD" && toUpper(arg1) == "A") ? arg2 : arg1;
            
            uint8_t baseCode = 0;
            if (op == "ADD") baseCode = 0b10000000;
            else if (op == "SUB") baseCode = 0b10010000;
            else if (op == "AND") baseCode = 0b10100000;
            else if (op == "OR")  baseCode = 0b10110000;
            else if (op == "XOR") baseCode = 0b10101000;
            else if (op == "CP")  baseCode = 0b10111000;
            
            if (isIndexed(srcStr)) { 
                bool isIX = (toUpper(srcStr).find("IX") != std::string::npos);
                output.push_back(isIX ? 0xDD : 0xFD);
                output.push_back(baseCode | 0b110); 
                output.push_back((uint8_t)extractOffset(srcStr, errors, line.lineNumber));
            } else if (isRegister(srcStr) || srcStr == "(HL)") {
                uint8_t r = getRegCode(srcStr);
                output.push_back(baseCode | r);
            } else { 
                uint8_t immBase = 0;
                if (op == "ADD") immBase = 0xC6;
                else if (op == "SUB") immBase = 0xD6;
                else if (op == "AND") immBase = 0xE6;
                else if (op == "OR")  immBase = 0xF6;
                else if (op == "XOR") immBase = 0xEE;
                else if (op == "CP")  immBase = 0xFE;
                
                output.push_back(immBase);
                output.push_back((uint8_t)resolveValue(srcStr, symbolTable, errors, line.lineNumber));
            }
        } else if (op == "JP" || op == "CALL") {
            uint8_t base = (op == "JP") ? 0xC3 : 0xCD;
            output.push_back(base);
            uint16_t addr = resolveValue(arg1, symbolTable, errors, line.lineNumber);
            output.push_back(addr & 0xFF);
            output.push_back((addr >> 8) & 0xFF);
        } else if (op == "JR") {
            output.push_back(0x18);
            uint16_t addr = resolveValue(arg1, symbolTable, errors, line.lineNumber);
            int offset = addr - (line.address + 2);
            if (offset < -128 || offset > 127) {
                errors.push_back("Linha " + std::to_string(line.lineNumber) + ": Pulo relativo 'JR' fora dos limites (-128 a 127).");
            }
            output.push_back((uint8_t)(int8_t)offset);
        } else {
             errors.push_back("Linha " + std::to_string(line.lineNumber) + ": Instrução não suportada na segunda passada: " + op);
        }
    }

    if (!errors.empty()) {
        return "";
    }

    std::string filename = "program.bin";
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        errors.push_back("Erro: Não foi possível criar o arquivo de saída " + filename);
        return "";
    }
    out.write(reinterpret_cast<const char*>(output.data()), output.size());
    out.close();

    return filename;
}

//retorna o path do binário gerado
std::string Assembler::assemble(const std::string& asmCode) {
    if (!firstPass(asmCode, 0)) {
        return "";
    }
    return secondPass();
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
