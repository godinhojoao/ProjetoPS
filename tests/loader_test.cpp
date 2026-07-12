/*
Teste standalone do BinaryLoader.

Gera um .bin temporario com um programinha conhecido, carrega via
BinaryLoader::load, e verifica byte a byte que a memoria ficou correta.

Programa de teste (4 bytes, opcodes da spec em vm.md):
  LD A, 0x42   ->  0x3E 0x42   (00 111 110 + n)
  ADD A, B     ->  0x80        (10 000 000)
 HLT         ->  0x76        (01 110 110)

Nao depende de CPU, VM, decoder ou executor — so memory + binary_loader.
*/

#include "../src/loader/binary_loader.h"
#include "../src/memory/memory.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool writeTempBin(const std::string& path, const std::vector<uint8_t>& bytes)
{
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
  return out.good();
}

static int fail(const std::string& msg)
{
  std::cerr << "[FAIL] " << msg << "\n";
  return 1;
}

int main()
{
  const std::string tmpPath = "/tmp/projetops_loader_test.bin";
  const std::vector<uint8_t> program = { 0x3E, 0x42, 0x80, 0x76 };

  // caso 1: carregar programa valido em 0x0000 e conferir bytes
  if (!writeTempBin(tmpPath, program))
  {
    return fail("nao foi possivel escrever .bin de teste em " + tmpPath);
  }

  Memory mem;
  BinaryLoader loader;
  uint32_t loaded = loader.load(tmpPath, mem);

  if (loaded != program.size())
  {
    return fail("bytes carregados (" + std::to_string(loaded) +
                ") != tamanho do programa (" + std::to_string(program.size()) + ")");
  }

  for (uint32_t i = 0; i < program.size(); ++i)
  {
    uint8_t got = mem.read(static_cast<uint16_t>(i));
    if (got != program[i])
    {
      char buf[128];
      std::snprintf(buf, sizeof(buf),
                    "mem[0x%04X] = 0x%02X, esperado 0x%02X",
                    i, got, program[i]);
      return fail(buf);
    }
  }
  std::cout << "[OK] load em 0x0000 OK (" << loaded << " bytes)\n";

  // caso 2: carregar com startAddr offset e conferir que NAO mexeu antes do offset
  Memory mem2;
  const uint16_t offset = 0x0100;
  uint32_t loaded2 = loader.load(tmpPath, mem2, offset);

  if (loaded2 != program.size())
  {
    return fail("offset: bytes carregados != tamanho do programa");
  }
  if (mem2.read(offset - 1) != 0x00)
  {
    return fail("offset: byte antes do startAddr foi alterado");
  }
  for (uint32_t i = 0; i < program.size(); ++i)
  {
    if (mem2.read(static_cast<uint16_t>(offset + i)) != program[i])
    {
      return fail("offset: byte gravado em endereco errado");
    }
  }
  std::cout << "[OK] load em 0x0100 OK\n";

  // caso 3: arquivo inexistente deve retornar 0 sem crashar
  Memory mem3;
  uint32_t loaded3 = loader.load("/tmp/arquivo_que_nao_existe.bin", mem3);
  if (loaded3 != 0)
  {
    return fail("arquivo inexistente deveria retornar 0");
  }
  std::cout << "[OK] arquivo inexistente tratado\n";

  // caso 4: arquivo que estoura a memoria (startAddr alto + programa) deve falhar
  Memory mem4;
  uint32_t loaded4 = loader.load(tmpPath, mem4, 0xFFFE);  // 4 bytes nao cabem
  if (loaded4 != 0)
  {
    return fail("overflow de memoria deveria retornar 0");
  }
  std::cout << "[OK] overflow de memoria detectado\n";

  std::remove(tmpPath.c_str());
  std::cout << "\nTodos os testes do loader passaram.\n";
  return 0;
}
