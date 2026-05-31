### Parte 1: Máquina Virtual (Executor Z80)
[] **Botão** para carregar o arquivo final na memória da máquina virtual.[cite: 1, 2]
[] **Botão** para iniciar a execução contínua do programa (até encontrar uma instrução de parada).[cite: 1]
[] **Botão** para executar o código passo a passo (instrução por instrução).[cite: 1]
[] **Botão** para resetar a máquina virtual (zerar todos os valores).
[] **Área com labels** para exibir os valores dos registradores principais de 8 bits (A, B, C, D, E, H, L).[cite: 1]
[] **Área com labels** para exibir os valores dos pares de registradores de 16 bits (AF, BC, DE, HL).[cite: 1]
[] **Área com labels** para exibir os registradores especiais (PC e SP).[cite: 1]
[] **Área com labels** para exibir os registradores de índice (IX e IY).[cite: 1]
[] **Área com indicadores visuais** para o estado dos flags do registrador F (S, Z, H, P/V, N, C).[cite: 1]
[] **Grade/Tabela** para visualizar as posições e os bytes de toda a memória de 64 KB.[cite: 1]

### Parte 2: Montador de Dois Passos
[] **Área de texto** para digitar ou visualizar o código em linguagem Assembly Z80.[cite: 2]
[] **Botão** para montar o código (gerar o código objeto/máquina).[cite: 2]
[] **Tabela** para exibir a resolução de símbolos (rótulos/labels encontrados e seus respectivos endereços).[cite: 2]
[] **Área de texto** para exibir possíveis erros de compilação ou sintaxe.[cite: 2]

### Parte 3: Processador de Macros
[] **Botão** para processar as macros antes do início da montagem.
[] **Área de texto (apenas leitura)** para exibir o código fonte resultante, com todas as macros e chamadas aninhadas já expandidas.

### Parte 4: Ligador-Relocador
[] **Lista** para gerenciar e visualizar os múltiplos arquivos objetos que serão unidos.
[] **Botão** para adicionar novos arquivos para a lista de ligação.
[] **Campo de texto** para digitar o endereço inicial (endereço de carga) onde o programa relocado será posicionado.
[] **Botões de seleção (opções exclusivas)** para escolher se a relocação será resolvida pelo ligador ou deixada para o momento da carga.
[] **Tabela** para exibir a Tabela Global de Símbolos (GTS) compartilhados entre os diferentes arquivos.
[] **Botão** para ligar os arquivos e gerar o executável final combinado.
