# Projeto de PS

## Professor: Anderson Ferrugem

### Grupo: Lucas Cavallin Caczmareki, Matheus Persch, João Godinho, Celina Lemos, Inácio Teixeira, Hiago Muniz, Pedro Iskovitz, Augusto Menchaca

## Como rodar:

### Executar localmente com make
- `make run` — roda sem programa carregado
- `make run ARGS=programa.bin` — carrega o `.bin` antes de executar
- `make test-loader` — roda a suíte de testes do loader

### Executar a interface no Windows/Linux
1. Instale o Qt 6 no Windows.
   - Baixe o `Qt Online Installer` em: https://www.qt.io/download-open-source
2. Durante a instalação, selecione o kit `Qt 6` e os componentes de desenvolvimento do `Qt Widgets`.
3. Instale também o compilador `MinGW 64-bit` e o `CMake`.
4. Rode o projeto pelo QT, na hora de abrir selecione "File -> Open file or project" e selecione o CmakeLists.txt dentro da pasta UI.

## Docs
- [Documentação da VM - teoria](./vm.md)
- [Documentação da VM - código (nao tem ainda)](./vm-codigo.md)
- [Loader (formato .bin, API, teste)](./loader.md)
