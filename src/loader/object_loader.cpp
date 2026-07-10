/*
aqui a gente le um arquivo objeto (.o) contendo codigo montado com enderecos
RELATIVOS ao inicio do modulo, mais um mapa dizendo onde esses enderecos estao

exemplo: CALL 0x0008 montado no offset 0x0002 do modulo
  CODE   -> CD 08 00           (operando relativo, little-endian)
  REALOC -> 0x0003             (offset do operando)

Loader: carrega o codigo em loadAddr e soma loadAddr aos operandos listados em
REALOC, virando enderecos absolutos. Com loadAddr = 0x0100 o CALL acima passa a
apontar para 0x0108.

Formato do .o: object_file_standard.md
*/

#include "object_loader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "../constants.h"
#include "../shared/shared.h"

bool ObjectLoader::parse(const std::string& path, ObjectFile& obj)
{
  std::ifstream file(path);
  if (!file)
  {
    std::cerr << "ObjectLoader: nao foi possivel abrir '" << path << "'\n";
    return false;
  }

  obj = ObjectFile{};
  ObjSection current = ObjSection::None;
  std::string rawLine;
  uint32_t lineNumber = 0;

  while (std::getline(file, rawLine))
  {
    ++lineNumber;

    // Shared::trim tira o comentario (do ';' em diante) e os espacos das pontas
    const std::string line = Shared::trim(rawLine);
    if (line.empty()) continue;

    const ObjSection keyword = Shared::toObjSection(line);
    if (keyword != ObjSection::None)
    {
      current = keyword;
      continue;
    }

    switch (current)
    {
      case ObjSection::Header:
        if (!parseHeaderLine(line, lineNumber, obj)) return false;
        break;

      case ObjSection::Realoc:
        if (!parseRelocLine(line, lineNumber, obj)) return false;
        break;

      case ObjSection::Code:
        if (!parseCodeLine(line, lineNumber, obj)) return false;
        break;

      // ExtDef/ExtRef: reconhecidas so pra nao caírem no CODE. Quem resolve
      // simbolo externo e o ligador (src/linker/extref_resolver.*).
      // None: preambulo antes de qualquer secao.
      case ObjSection::ExtDef:
      case ObjSection::ExtRef:
      case ObjSection::None:
        break;
    }
  }

  return validate(path, obj);
}

bool ObjectLoader::parseHeaderLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj)
{
  const size_t colon = line.find(':');
  if (colon == std::string::npos)
  {
    std::cerr << "ObjectLoader: linha " << lineNumber
              << ": HEADER esperava 'CAMPO: valor', veio '" << line << "'\n";
    return false;
  }

  const std::string field = Shared::trim(line.substr(0, colon));
  const std::string value = Shared::trim(line.substr(colon + 1));

  if (field == "MODULE")
  {
    obj.moduleName = value;
    return true;
  }

  if (field == "SIZE")
  {
    unsigned long size = 0;
    if (!Shared::tryParseNumber(value, 0, size) || size > 0xFFFF)
    {
      std::cerr << "ObjectLoader: linha " << lineNumber << ": SIZE invalido '" << value << "'\n";
      return false;
    }
    obj.declaredSize = static_cast<uint16_t>(size);
    return true;
  }

  // campos desconhecidos no HEADER sao ignorados de proposito, pra o .o poder
  // ganhar metadados novos sem quebrar loaders antigos
  return true;
}

bool ObjectLoader::parseRelocLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj)
{
  std::istringstream stream(line);
  std::string token;

  while (stream >> token)
  {
    unsigned long offset = 0;
    if (!Shared::tryParseNumber(token, 0, offset) || offset > 0xFFFF)
    {
      std::cerr << "ObjectLoader: linha " << lineNumber
                << ": offset de realocacao invalido '" << token << "'\n";
      return false;
    }
    obj.relocOffsets.push_back(static_cast<uint16_t>(offset));
  }

  return true;
}

bool ObjectLoader::parseCodeLine(const std::string& line, uint32_t lineNumber, ObjectFile& obj)
{
  std::istringstream stream(line);
  std::string token;

  while (stream >> token)
  {
    unsigned long byte = 0;
    if (!Shared::tryParseNumber(token, 16, byte) || byte > 0xFF)
    {
      std::cerr << "ObjectLoader: linha " << lineNumber
                << ": byte hexadecimal invalido '" << token << "'\n";
      return false;
    }
    obj.code.push_back(static_cast<uint8_t>(byte));
  }

  return true;
}

bool ObjectLoader::validate(const std::string& path, const ObjectFile& obj)
{
  if (obj.code.empty())
  {
    std::cerr << "ObjectLoader: '" << path << "' nao tem secao CODE ou ela esta vazia\n";
    return false;
  }

  // SIZE pode ser maior que o CODE (o modulo pode reservar espaco de dados),
  // mas nunca menor: isso significaria codigo fora do segmento declarado
  if (obj.declaredSize > 0 && obj.code.size() > obj.declaredSize)
  {
    std::cerr << "ObjectLoader: '" << path << "' declara SIZE=" << obj.declaredSize
              << " mas a secao CODE tem " << obj.code.size() << " bytes\n";
    return false;
  }

  return true;
}

bool ObjectLoader::relocate(ObjectFile& obj, uint16_t loadAddr)
{
  for (uint16_t offset : obj.relocOffsets)
  {
    // o operando ocupa 2 bytes, entao offset+1 tambem tem que existir
    if (static_cast<uint32_t>(offset) + 1 >= obj.code.size())
    {
      std::cerr << "ObjectLoader: offset de realocacao 0x" << std::hex << offset
                << " fora do codigo (" << std::dec << obj.code.size() << " bytes)\n";
      return false;
    }

    const uint16_t relative = static_cast<uint16_t>(obj.code[offset] |
                                                    (obj.code[offset + 1] << 8));
    const uint32_t absolute = static_cast<uint32_t>(relative) + loadAddr;

    if (absolute > 0xFFFF)
    {
      std::cerr << "ObjectLoader: endereco realocado 0x" << std::hex << absolute
                << " passa de 0xFFFF (relativo 0x" << relative << " + carga 0x" << loadAddr
                << ")\n" << std::dec;
      return false;
    }

    obj.code[offset] = static_cast<uint8_t>(absolute & 0xFF);
    obj.code[offset + 1] = static_cast<uint8_t>((absolute >> 8) & 0xFF);
  }

  return true;
}

uint32_t ObjectLoader::load(const std::string& path, Memory& memory, uint16_t loadAddr)
{
  ObjectFile obj;
  if (!parse(path, obj))
  {
    return 0;
  }

  // verifica se o codigo cabe na memoria a partir de loadAddr
  const uint32_t available = VM_MEMORY_IN_BYTES - loadAddr;
  if (obj.code.size() > available)
  {
    std::cerr << "ObjectLoader: codigo nao cabe na memoria (size=" << obj.code.size()
              << " bytes, disponivel a partir de 0x" << std::hex << loadAddr
              << " = " << std::dec << available << " bytes)\n";
    return 0;
  }

  if (!relocate(obj, loadAddr))
  {
    return 0;
  }

  for (uint32_t i = 0; i < obj.code.size(); ++i)
  {
    const uint16_t addr = static_cast<uint16_t>(loadAddr + i);
    if (!memory.write(addr, obj.code[i]))
    {
      std::cerr << "ObjectLoader: memoria recusou escrita em 0x" << std::hex << addr
                << std::dec << " (endereco dentro de um segmento ja fechado)\n";
      return 0;
    }
  }

  return static_cast<uint32_t>(obj.code.size());
}
