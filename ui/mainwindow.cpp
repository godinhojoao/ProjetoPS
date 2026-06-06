#include "mainwindow.h"
#include "./ui_mainwindow.h"
//ver oq isso faz
#include <QDir>
#include <QHeaderView>

// Construtor da MainWindow (oq eu configurar por aqui vem por padrão na hora do run)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QObject::connect(QUEM_EMITE, &CLASSE::SINAL, QUEM_ESCUTA, &CLASSE::FUNÇÃO_SLOT);

    // Conecta o sinal doubleClicked da treeFiles com a função q fizemos
    connect(ui->treeFiles, &QTreeView::doubleClicked, this, &MainWindow::on_treeFiles_doubleClicked);

    ui->tabCodeEditor->setTabsClosable(true); //habilita o btn 'X' com signal nas abas do tabCodeEditor
    // nome do signal: tabCloseRequested(int index); index é a tab que foi clicada

    connect(ui->tabCodeEditor, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);


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
    ui->treeFiles->header()->hide(); //esconde o header

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openNewFile(const QString &fileName, const QString &content) {
    // Cria um editor de texto novo na memória
    QPlainTextEdit *newEditor = new QPlainTextEdit(this); //this define que esse editor de texto pertence a esta (this) janela
    // Um plaintext editor tem um document(), que vai ter algumas coisas interessantes: isModified(), e emite um signal toda vez que é modificado: modificationChanged(bool)
    // conecta o editor de texto instanciado (ele continua vivo na heap dps do fim dessa função, só perdemos o ponteiro, e o connect do signal dele continua ligado internamente)
    connect(newEditor->document(), &QTextDocument::modificationChanged, this, &MainWindow::onDocumentModified);

    // Injeta o texto do arquivo lido pra dentro do editor de texto
    newEditor->setPlainText(content);

    // Seta uma fontezinha, dps da pra mudar
    QFont fonteMono("Monospace", 10);
    newEditor->setFont(fonteMono);

    // Adiciona o editor como uma nova página dentro do TabWidget
    // o método addTab retorna o índice (0, 1, 2...) que é a posição da aba no widget
    int indexTab = ui->tabCodeEditor->addTab(newEditor, fileName); //adiciona o editor de texto (objeto. nome aba)
    ui->tabCodeEditor->setCurrentIndex(indexTab);
}

// Quando um sinal de doubleClicked do QTreeView é emitido, ele envia o QModelIndex daquele objeto(arquivo)
// Falta: Ler QFile e , suportar outros tipos de arquivos (além de apenas text), salvar arquivo
void MainWindow::on_treeFiles_doubleClicked(const QModelIndex &index) {
    // Pega o caminho ("/home/usuario/projeto/main.asm") através do index recebido
    QString caminhoCompleto = modelFiles->filePath(index);

    // Pega só o nome do arquivo (main.asm) pra colocar no título da aba
    QString nomeCurto = modelFiles->fileName(index);

    // Verificação de segurança, se o cara clicou numa pasta por exemplo, a gente não abre um editor de texto, n faz sentido
    if (modelFiles->isDir(index)) {
        return; // Ignora sinal
    }

    // Abre um leitor de arquivo do QT (QFile) -> ver melhor como ele funciona depois
    QFile arquivo(caminhoCompleto);

    // Tentamos abrir o arquivo em modo de "Apenas Leitura" e como "Texto"

    // Abre em modo de leitura (vamo só copiar oq ta escritO) e texto
    // dá pra ler como byte depois se o cara abrir um .o/.obj depois, por simplicidade por enquanto abre como texto mesmo
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {

        // O QTextStream é um leitor de fluxos de texto do QT -> ver melhor como ele funciona depois
        QTextStream fluxoLeitura(&arquivo);
        QString conteudoDoArquivo = fluxoLeitura.readAll(); //lê o arquivo inteiro e guarda numa string
        arquivo.close();

        openNewFile(nomeCurto, conteudoDoArquivo); //abre o arquivo agora no editor de texto (QPlainText)

    } else {
        //Se deu merda na hora de abrir, escreve no log
        ui->textErrorLog->appendPlainText("Erro: Não foi possível abrir o arquivo " + nomeCurto);
    }
}

//essa função fecha a aba E destrói o QPlainTextEdit que foi criado com abrimos ela (eles n são conectados por podrão)
void MainWindow::closeTab(int index) {
    QWidget *widget = ui->tabCodeEditor->widget(index); //resgata o ponteiro pro plaintext (widget da aba index)
    ui->tabCodeEditor->removeTab(index);                //deleta a aba
    widget->deleteLater();                              //deleta depois que os eventos relacionado à essa função finalizarem (evita segfault caso algum evento ainda esteja usando o widget)
}

//Recebe um sinal quando o arquivo foi modificado (se a flag mudou, de false pra true, alguém escreveu, de true pra false, salvou. Ambos sinais são capturados por essa função)
void MainWindow::onDocumentModified(bool modified) {
    /*
     * to usando a lógica de subir hierarquia, mas dava pra salvar a referência do editor em algum lugar
     * agr como ja ta pronto aqui é os guri
     *
    // Precisamos descobrir quem (qual aba) emitiu o sinal
    // Descobre qual texto foi alterado (dps parent()->parent retorna o editor de texto)
    // sender() retorna um QObject, converte ele pro objeto que a gente assume ser(via qobject_cast, q tem segurança, se n for o objeto q a gente deu cast, vai retornar null.)
    */

    //método sender descobre qual document (dos nossos widgets de texto, q tao numa tab) enviou o sinal
    QTextDocument *document = qobject_cast<QTextDocument*>(sender());
    if(!document) { return; }   // se não é um documento q enviou esse sinal, n faz nada (verificação de nulo)

    // Descobre o editor de texto que foi alterado
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(document->parent()->parent());
    if(!editor) { return; }

    // Descobre a aba que o editor de texto tá
    int tabIdx = ui->tabCodeEditor->indexOf(editor);

    QString tabName = ui->tabCodeEditor->tabText(tabIdx);
    if (modified) { // adiciona o *
        if(!tabName.startsWith("*")) {
            ui->tabCodeEditor->setTabText(tabIdx, "*" + tabName); //concatena um * no nome da aba se ela já n tiver um
        }
    } else {
        if(tabName.startsWith("*")) {
            // mid extrai uma substring de n caracteres começando em x. mid(x, n)
            ui->tabCodeEditor->setTabText(tabIdx, tabName.mid(1)); //mid 1 extrai todos caracteres a partir do 1
        }
    }

}






