#include "mainwindow.h"
#include "./ui_mainwindow.h"
//ver oq isso faz
#include <QDir>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->treeFiles, &QTreeView::doubleClicked, this, &MainWindow::on_treeFiles_doubleClicked);
    //Esse código aq tu apaga dps e refaz na mão com mais calma depois de ler
        // QDir
        // QHeaderView
        // QFileSystemModel
        // QTreeView

    modelFiles = new QFileSystemModel(this); //instantica um leitor de arquivos

    // Isso aqui pode mudar dps se eu quiser colocar ele pra monitorar uma pasta q o cara selecionar
    // Por enquanto vou fazer ele mostrar a mesma pasta do projeto pra testes
    QString rootDir = QDir::currentPath();
    modelFiles->setRootPath(rootDir);

    //Conecta o tree view com o model
    ui->treeFiles->setModel(modelFiles);
    ui->treeFiles->setRootIndex(modelFiles->index(rootDir));

    //Aqui é só pra deixar bonitinho, como o treeview exibe igual um navegador de pasta (nome/tam/tipo/data) esconde as coluna q a gente n quer
    ui->treeFiles->hideColumn(1); //esconde tamanho
    ui->treeFiles->hideColumn(2); //esconde tipo
    ui->treeFiles->hideColumn(3); //esconde data
    // ui->treeFiles->header() dá pra mudar as coisas aq dps, fazer melhorzinho
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openNewFile(const QString &fileName, const QString &content) {
    // Cria um editor de texto novo na memória
    QPlainTextEdit *newEditor = new QPlainTextEdit(this); //this define que esse editor de texto pertence a esta (this) janela

    // Injeta o texto do arquivo lido pra dentro do editor de texto
    newEditor->setPlainText(content);

    // Seta uma fontezinha, dps da pra mudar
    QFont fonteMono("Monospace", 10);
    newEditor->setFont(fonteMono);

    // Adiciona o editor como uma nova página dentro do TabWidget
    // o método addTab retorna o índice (0, 1, 2...) que é a posição da aba no widget
    int indexTab = ui->tabCodeEditor->addTab(newEditor, fileName);
    ui->tabCodeEditor->setCurrentIndex(indexTab);
}

void MainWindow::on_treeFiles_doubleClicked(const QModelIndex &index) {
    // 1. Perguntamos para o motor qual é o caminho completo no HD do item clicado
    // Exemplo: "/home/usuario/projeto/main.asm"
    QString caminhoCompleto = modelFiles->filePath(index);

    // 2. Perguntamos também apenas o nome curto para colocar no título da aba
    // Exemplo: "main.asm"
    QString nomeCurto = modelFiles->fileName(index);

    // 3. Segurança: O usuário pode ter clicado em uma pasta, e não em um arquivo.
    // Não faz sentido abrir uma pasta como texto, então checamos isso:
    if (modelFiles->isDir(index)) {
        return; // É uma pasta? Ignora e sai da função.
    }

    // 4. Abrindo o arquivo real usando os recursos do Qt (QFile)
    QFile arquivo(caminhoCompleto);

    // Tentamos abrir o arquivo em modo de "Apenas Leitura" e como "Texto"
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {

        // O QTextStream é um leitor inteligente de fluxos de texto
        QTextStream fluxoLeitura(&arquivo);

        // Lemos o arquivo INTEIRO do HD e guardamos em uma String do Qt
        QString conteudoDoArquivo = fluxoLeitura.readAll();

        // Fechamos o arquivo para liberar o sistema operacional
        arquivo.close();

        // 5. Chamamos a nossa função do Passo 3 passando os dados!
        openNewFile(nomeCurto, conteudoDoArquivo);

    } else {
        // Se deu alguma zebra muito grande ao abrir o arquivo (ex: falta de permissão no Linux)
        // Você pode mandar uma mensagem pro seu Console de Erros que criamos embaixo:
        ui->textErrorLog->appendPlainText("Erro: Não foi possível abrir o arquivo " + nomeCurto);
    }
}
