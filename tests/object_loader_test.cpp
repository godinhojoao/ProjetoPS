/*
Teste standalone do ObjectLoader.

Gera .o temporarios no formato texto de object_file_standard.md, carrega via
ObjectLoader::load e verifica que:
  - as secoes CODE e REALOC foram lidas (e EXTDEF/EXTREF ignoradas);
  - os operandos apontados por REALOC receberam o offset de carga;
  - os bytes NAO apontados por REALOC ficaram intactos;
  - .o malformado falha sem crashar.

Modulo de teste (11 bytes, opcodes da spec em vm.md), enderecos relativos:
  0x0000  3E 05      LD A, 0x05
  0x0002  CD 08 00   CALL 0x0008   -> operando em 0x0003 (realocavel)
  0x0005  C3 0A 00   JP   0x000A   -> operando em 0x0006 (realocavel)
  0x0008  3C         INC A
  0x0009  C9         RET
  0x000A  76         HALT

Nao depende de CPU, VM, decoder ou executor — so memory + object_loader.
*/

#include "../src/loader/object_loader.h"
#include "../src/memory/memory.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static const std::string VALID_OBJ =
  "; modulo de teste\n"
  "HEADER\n"
  "MODULE: teste\n"
  "SIZE: 11\n"
  "\n"
  "EXTDEF\n"
  "ENTRADA 0x0000\n"
  "\n"
  "EXTREF\n"
  "PRINT_HEX 0x0003\n"
  "\n"
  "REALOC\n"
  "0x0003\n"
  "0x0006\n"
  "\n"
  "CODE\n"
  "3E 05 CD 08 00 C3 0A 00\n"
  "3C C9 76\n";

// bytes do modulo antes de qualquer realocacao
static const std::vector<uint8_t> RELATIVE_CODE =
  { 0x3E, 0x05, 0xCD, 0x08, 0x00, 0xC3, 0x0A, 0x00, 0x3C, 0xC9, 0x76 };

static bool writeTempObj(const std::string& path, const std::string& content)
{
  std::ofstream out(path);
  if (!out) return false;
  out << content;
  return out.good();
}

static int fail(const std::string& msg)
{
  std::cerr << "[FAIL] " << msg << "\n";
  return 1;
}

// confere que mem[loadAddr + i] bate com o esperado, byte a byte
static int checkMemory(const Memory& mem, uint16_t loadAddr,
                       const std::vector<uint8_t>& expected, const std::string& label)
{
  for (uint32_t i = 0; i < expected.size(); ++i)
  {
    const uint16_t addr = static_cast<uint16_t>(loadAddr + i);
    const uint8_t got = mem.read(addr);
    if (got != expected[i])
    {
      char buf[160];
      std::snprintf(buf, sizeof(buf),
                    "%s: mem[0x%04X] = 0x%02X, esperado 0x%02X",
                    label.c_str(), addr, got, expected[i]);
      return fail(buf);
    }
  }
  return 0;
}

int main()
{
  const std::string objPath = "tests/projetops_object_loader_test.o";
  ObjectLoader loader;

  if (!writeTempObj(objPath, VALID_OBJ))
  {
    return fail("nao foi possivel escrever .o de teste em " + objPath);
  }

  // caso 1: parse le header, code e realoc; ignora EXTDEF/EXTREF
  ObjectFile obj;
  if (!loader.parse(objPath, obj))
  {
    return fail("parse de .o valido falhou");
  }
  if (obj.moduleName != "teste")
  {
    return fail("MODULE lido como '" + obj.moduleName + "', esperado 'teste'");
  }
  if (obj.declaredSize != 11)
  {
    return fail("SIZE lido como " + std::to_string(obj.declaredSize) + ", esperado 11");
  }
  if (obj.code != RELATIVE_CODE)
  {
    return fail("secao CODE nao bate (EXTDEF/EXTREF podem ter vazado pra dentro dela)");
  }
  if (obj.relocOffsets != std::vector<uint16_t>{ 0x0003, 0x0006 })
  {
    return fail("secao REALOC nao bate");
  }
  std::cout << "[OK] parse do .o (header, code, realoc; extdef/extref ignorados)\n";

  // caso 2: load em 0x0000 e identidade — somar 0 nao muda operando nenhum
  Memory mem1;
  uint32_t loaded1 = loader.load(objPath, mem1);
  if (loaded1 != RELATIVE_CODE.size())
  {
    return fail("bytes carregados (" + std::to_string(loaded1) + ") != 11");
  }
  if (int rc = checkMemory(mem1, 0x0000, RELATIVE_CODE, "carga em 0x0000")) return rc;
  std::cout << "[OK] load em 0x0000 sem deslocar enderecos (" << loaded1 << " bytes)\n";

  // caso 3: load em 0x0100 realoca CALL 0x0008 -> 0x0108 e JP 0x000A -> 0x010A,
  // e deixa todo o resto intacto
  Memory mem2;
  const uint16_t loadAddr = 0x0100;
  const std::vector<uint8_t> relocatedCode =
    { 0x3E, 0x05, 0xCD, 0x08, 0x01, 0xC3, 0x0A, 0x01, 0x3C, 0xC9, 0x76 };

  uint32_t loaded2 = loader.load(objPath, mem2, loadAddr);
  if (loaded2 != RELATIVE_CODE.size())
  {
    return fail("offset: bytes carregados != 11");
  }
  if (int rc = checkMemory(mem2, loadAddr, relocatedCode, "carga em 0x0100")) return rc;
  if (mem2.read(loadAddr - 1) != 0x00)
  {
    return fail("offset: byte antes do loadAddr foi alterado");
  }
  std::cout << "[OK] realocacao em 0x0100 (CALL 0x0108, JP 0x010A)\n";

  // caso 4: offset de realocacao apontando fora do codigo deve falhar
  const std::string badOffsetPath = "tests/projetops_object_loader_bad_offset.o";
  writeTempObj(badOffsetPath,
               "HEADER\nSIZE: 11\nREALOC\n0x0064\nCODE\n3E 05 CD 08 00 C3 0A 00 3C C9 76\n");

  Memory mem3;
  if (loader.load(badOffsetPath, mem3, 0x0000) != 0)
  {
    return fail("offset de realocacao fora do codigo deveria retornar 0");
  }
  std::cout << "[OK] offset de realocacao fora do codigo detectado\n";

  // caso 5: endereco realocado passando de 0xFFFF deve falhar (JP 0xFFF8 + 0x0100)
  const std::string overflowPath = "tests/projetops_object_loader_overflow.o";
  writeTempObj(overflowPath, "HEADER\nSIZE: 3\nREALOC\n0x0001\nCODE\nC3 F8 FF\n");

  Memory mem4;
  if (loader.load(overflowPath, mem4, 0x0100) != 0)
  {
    return fail("realocacao passando de 0xFFFF deveria retornar 0");
  }
  std::cout << "[OK] estouro de 16 bits na realocacao detectado\n";

  // caso 6: byte hexadecimal invalido na secao CODE deve falhar sem crashar
  const std::string badHexPath = "tests/projetops_object_loader_bad_hex.o";
  writeTempObj(badHexPath, "HEADER\nSIZE: 3\nCODE\n3E ZZ 76\n");

  Memory mem5;
  if (loader.load(badHexPath, mem5) != 0)
  {
    return fail("byte hexadecimal invalido deveria retornar 0");
  }
  std::cout << "[OK] byte hexadecimal invalido tratado\n";

  // caso 6b: lixo grudado no numero tambem invalida — std::stoul sozinho leria
  // "05Z" como 0x05 e engoliria o 'Z' calado
  const std::string trailingPath = "tests/projetops_object_loader_trailing.o";
  writeTempObj(trailingPath, "HEADER\nSIZE: 3\nCODE\n3E 05Z 76\n");

  Memory mem5b;
  if (loader.load(trailingPath, mem5b) != 0)
  {
    return fail("byte hexadecimal com lixo no fim deveria retornar 0");
  }
  std::cout << "[OK] lixo grudado no byte hexadecimal tratado\n";

  // caso 7: SIZE menor que o CODE indica .o inconsistente
  const std::string badSizePath = "tests/projetops_object_loader_bad_size.o";
  writeTempObj(badSizePath, "HEADER\nSIZE: 2\nCODE\n3E 05 76\n");

  Memory mem6;
  if (loader.load(badSizePath, mem6) != 0)
  {
    return fail("SIZE menor que CODE deveria retornar 0");
  }
  std::cout << "[OK] SIZE inconsistente com CODE detectado\n";

  // caso 8: arquivo inexistente deve retornar 0 sem crashar
  Memory mem7;
  if (loader.load("tests/arquivo_que_nao_existe.o", mem7) != 0)
  {
    return fail("arquivo inexistente deveria retornar 0");
  }
  std::cout << "[OK] arquivo inexistente tratado\n";

  // caso 9: codigo que nao cabe na memoria a partir do loadAddr deve falhar
  Memory mem8;
  if (loader.load(objPath, mem8, 0xFFFF) != 0)  // 11 bytes nao cabem em 1
  {
    return fail("overflow de memoria deveria retornar 0");
  }
  std::cout << "[OK] overflow de memoria detectado\n";

  std::remove(objPath.c_str());
  std::remove(badOffsetPath.c_str());
  std::remove(overflowPath.c_str());
  std::remove(badHexPath.c_str());
  std::remove(trailingPath.c_str());
  std::remove(badSizePath.c_str());
  std::cout << "\nTodos os testes do object loader passaram.\n";
  return 0;
}
