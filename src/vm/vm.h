#pragma once
#include <string>
#include "../memory/memory.h"
#include "../cpu/cpu.h"
#include "../instruction_set/decoder.h"
#include "../ui/vmstate.h"

class VM
{
public:
    /**
    * @brief  Carrega um programa .bin na memoria da VM (a partir de 0x0000).
    *         Deve ser chamado antes de run() quando se quer executar um programa.
    *
    * @param  binPath  Caminho do arquivo .bin montado pelo assembler.
    * @param  startAddr Endereço inicial de gravação(padrão 0x0000)
    * @return          true se carregou ao menos 1 byte; false em caso de erro.
    */
    bool load(const std::string& binPath, uint16_t startAddr = 0x0000, uint32_t codeSize = 0);

    /**
    * @brief  Carrega um arquivo objeto .o realocavel na memoria da VM.
    *         Diferente de load(), aqui os enderecos absolutos do codigo (alvos de
    *         JP/CALL) sao ajustados somando loadAddr, conforme a tabela REALOC do
    *         .o. O PC fica apontando para o inicio do modulo realocado.
    *
    * @param  objPath   Caminho do .o gerado pelo montador/ligador.
    * @param  loadAddr  Endereco inicial de carga (padrao 0x0000).
    * @return           true se carregou ao menos 1 byte; false em caso de erro.
    */
    bool loadObject(const std::string& objPath, uint16_t loadAddr = 0x0000);

    void run();
    bool step();
    void reset();
    VMState getState() const; //snapshot dos registradores e flags
    bool isLoaded() const;
    // uint8_t readMemory(uint16_t addr) const;           // lê um byte da memória
    // std::vector<uint8_t> readMemoryRange(uint16_t start, uint16_t length) const;

private:
    Memory mem;
    CPU cpu;
};
