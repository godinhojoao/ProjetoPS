#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Console.h"
#include "Project.h"
#include "btnrun_popup.h"
#include "btnbuild_popup.h"
#include <QDir>
#include <QHeaderView>
#include <QIcon>

// Construtor da MainWindow (oq eu configurar por aqui vem por padrão na hora do run)
MainWindow::MainWindow(Project *project, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->project = project;

    setWindowTitle("Z80");

    // project = new Project();              // classe q controla o backend (controller)
    console = new Console(project, this); // classe q controla o console

    ui->tabCodeEditor->setTabsClosable(true); //habilita o btn 'X' com signal nas abas do tabCodeEditor
    QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this); //cria um shortcut Ctrl+S

    ui->tabShowData->setTabText(0, "Registradores e Flags");
    ui->tabShowData->setTabText(1, "Assembler");
    ui->tabShowData->setTabText(2, "Linker");

    // ---------------------------------------------------
    // Conexões entre Signals/Slots
    // ---------------------------------------------------
    //connect(QUEM_EMITE,       &CLASSE::SINAL,                 QUEM_ESCUTA,        &CLASSE::FUNÇÃO_SLOT);

    connect(ui->treeFiles,      &QTreeView::doubleClicked,      this,               &MainWindow::on_treeFiles_doubleClicked);
    connect(ui->tabCodeEditor,  &QTabWidget::tabCloseRequested, this,               &MainWindow::closeTab);
    connect(saveShortcut,       &QShortcut::activated,          this,               &MainWindow::saveCurrentFile);
    connect(console,            &Console::saveFileRequested,    this,               &MainWindow::saveFile);//Save <path>
    connect(ui->commandInput,   &QLineEdit::returnPressed,      this,               &MainWindow::onCommandEntered);
    connect(console,            &Console::output,               ui->consoleOutput,  &QPlainTextEdit::appendPlainText);
    connect(console,            &Console::clearRequested,       ui->consoleOutput,  &QPlainTextEdit::clear);

    connect(ui->btnAssemble,    &QPushButton::clicked,          this,               &MainWindow::onAssembleClicked);
    connect(ui->btnLink,        &QPushButton::clicked,          this,               &MainWindow::onLinkClicked);
    connect(ui->btnBuild,       &QPushButton::clicked,          this,               &MainWindow::onBuildClicked);
    connect(ui->btnLoad,        &QPushButton::clicked,          this,               &MainWindow::onLoadClicked);
    connect(ui->btnRun,         &QPushButton::clicked,          this,               &MainWindow::onRunClicked);
    connect(ui->btnStep,        &QPushButton::clicked,          this,               &MainWindow::onStepClicked);
    connect(ui->btnReset,       &QPushButton::clicked,          this,               &MainWindow::onResetClicked);

    connect(project, &Project::flagsAndReg_Modified, this, &MainWindow::updateRegFlagTable);

    // ---------------------------------------------------
    // Conexão e configuração do visualizador de arquivos
    // ---------------------------------------------------
    modelFiles = new QFileSystemModel(this);        //instantcia um leitor de arquivos
    modelFiles->setRootPath(project->getRootDir()); // Monitora só uma pasta de root arbitrária por enquanto

    //Conecta o tree view com o model(leitor de arquivo)
    ui->treeFiles->setModel(modelFiles);
    ui->treeFiles->setRootIndex(modelFiles->index(project->getRootDir()));

    //Aqui é só pra deixar bonitinho, como o treeview exibe igual um navegador de pasta (nome/tam/tipo/data) esconde as coluna q a gente n quer
    ui->treeFiles->hideColumn(1);       //esconde tamanho
    ui->treeFiles->hideColumn(2);       //esconde tipo
    ui->treeFiles->hideColumn(3);       //esconde data
    ui->treeFiles->header()->hide();    //esconde o header

    // ---------------------------------------------------
    // Configuração das tabelas de registrador e memória
    // ---------------------------------------------------
    // Registradores
    ui->tableRegs->setRowCount(10);
    ui->tableRegs->setColumnCount(2);
    ui->tableRegs->setHorizontalHeaderLabels({"Registrador", "Valor"});

    ui->tableRegs->setItem(0, 0, new QTableWidgetItem("A"));
    ui->tableRegs->setItem(1, 0, new QTableWidgetItem("B"));
    ui->tableRegs->setItem(2, 0, new QTableWidgetItem("C"));
    ui->tableRegs->setItem(3, 0, new QTableWidgetItem("D"));
    ui->tableRegs->setItem(4, 0, new QTableWidgetItem("E"));
    ui->tableRegs->setItem(5, 0, new QTableWidgetItem("H"));
    ui->tableRegs->setItem(6, 0, new QTableWidgetItem("L"));
    ui->tableRegs->setItem(7, 0, new QTableWidgetItem("PC"));
    ui->tableRegs->setItem(8, 0, new QTableWidgetItem("SP"));
    ui->tableRegs->setItem(9, 0, new QTableWidgetItem("F"));

    // Coloca valores iniciais igual a 0.
    for(int i = 0; i < 10; i++) {
        ui->tableRegs->setItem(i, 1, new QTableWidgetItem("0"));
    }

    // Flags
    ui->tableFlags->setRowCount(6);
    ui->tableFlags->setColumnCount(2);
    ui->tableFlags->setHorizontalHeaderLabels({"Flag", "Estado"});

    ui->tableFlags->setItem(0, 0, new QTableWidgetItem("Z"));
    ui->tableFlags->setItem(1, 0, new QTableWidgetItem("C"));
    ui->tableFlags->setItem(2, 0, new QTableWidgetItem("S"));
    ui->tableFlags->setItem(3, 0, new QTableWidgetItem("P/V"));
    ui->tableFlags->setItem(4, 0, new QTableWidgetItem("H"));
    ui->tableFlags->setItem(5, 0, new QTableWidgetItem("N"));

    for(int i = 0; i < 6; i++) {
        ui->tableFlags->setItem(i, 1, new QTableWidgetItem("0"));
    }

    // updateRegFlagTable(project->getVmState());

    ui->tableRegs->horizontalHeader()->setStretchLastSection(true);
    ui->tableFlags->horizontalHeader()->setStretchLastSection(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Quando um sinal de doubleClicked do QTreeView é emitido, ele envia o QModelIndex daquele objeto(arquivo)
 * Essa função é chamada através do signal doubleClicked do treeView e abre o arquivo no index enviado.
 */
void MainWindow::on_treeFiles_doubleClicked(const QModelIndex &index) {
    QString caminhoCompleto = modelFiles->filePath(index);  // Pega o caminho ("/home/usuario/projeto/main.asm") através do index recebido
    QString nomeCurto = modelFiles->fileName(index);        // Pega só o nome do arquivo (main.asm) pra colocar no título da aba

    // Verificação de segurança, se o cara clicou numa pasta por exemplo, a gente não abre um editor de texto, n faz sentido
    if (modelFiles->isDir(index)) { return; }

    QFile arquivo(caminhoCompleto); //abre o leitor de arquivo do QT


    // Abre em modo de leitura (vamo só copiar oq ta escritO) e texto
    // dá pra ler como byte depois se o cara abrir um .o/.obj depois, por simplicidade por enquanto abre como texto mesmo

    /*
     * Abre o arquivo em módulo de leitura, copia o texto pra dentro do editor q vai ser criado
     * e fecha o arquivo.
     */
    if (arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream fluxoLeitura(&arquivo);
        QString conteudoDoArquivo = fluxoLeitura.readAll();         //lê o arquivo inteiro e guarda numa string
        arquivo.close();
        openNewFile(nomeCurto, conteudoDoArquivo, caminhoCompleto); //abre o arquivo agora no editor de texto (QPlainText)

    } else {
        ui->consoleOutput->appendPlainText("Erro: Não foi possível abrir o arquivo " + nomeCurto); //Se deu merda na hora de abrir, escreve no log
    }
}

/*
 * Essa função é chamada pela on_treeFiles_doubleClicked.
 * Ela é responsável por criar a nova aba com o texto do arquivo dentro.
 */
void MainWindow::openNewFile(const QString &fileName, const QString &content, const QString &filepath) {
    // Cria um editor de texto novo na memória
    QPlainTextEdit *newEditor = new QPlainTextEdit(this);   //this define que esse editor de texto pertence a esta (this) janela
    newEditor->setProperty("filepath", filepath);           // Isso faz com que todo editor criado a partir de um arquivo contenha o caminho daquele arquivo.


    // Um plaintext editor tem um document(), que vai ter algumas coisas interessantes: isModified(), e emite um signal toda vez que é modificado: modificationChanged(bool)
    // conecta o editor de texto instanciado (ele continua vivo na heap dps do fim dessa função, só perdemos o ponteiro, e o connect do signal dele continua ligado internamente)
    connect(newEditor->document(), &QTextDocument::modificationChanged, this, &MainWindow::onDocumentModified);

    newEditor->setPlainText(content); // Injeta o texto do arquivo lido pra dentro do editor de texto
    QFont fonteMono("Monospace", 10); // Seta uma fontezinha, dps da pra mudar
    newEditor->setFont(fonteMono);

    // Adiciona o editor como uma nova página dentro do TabWidget
    // o método addTab retorna o índice (0, 1, 2...) que é a posição da aba no widget
    int indexTab = ui->tabCodeEditor->addTab(newEditor, fileName); //adiciona o editor de texto (objeto. nome aba)
    ui->tabCodeEditor->setCurrentIndex(indexTab);
}

/*
 * Essa função fica escutando o signal do botão "x" habilitado nas abas
 * Qaundo escuta, ela fecha a aba E destrói o QPlainTextEdit que foi criado com abrimos ela (eles n são conectados por podrão)
 */
void MainWindow::closeTab(int index) {
    QWidget *widget = ui->tabCodeEditor->widget(index); //resgata o ponteiro pro plaintext (widget da aba index)
    ui->tabCodeEditor->removeTab(index);                //deleta a aba
    widget->deleteLater();                              //deleta depois que os eventos relacionado à essa função finalizarem (evita segfault caso algum evento ainda esteja usando o widget)
}

/*
 * Recebe um sinal quando o arquivo foi modificado (se a flag mudou, de false pra true, alguém escreveu, de true pra false, salvou.
 * Ambos sinais são capturados por essa função).
 * Ela muda o texto da aba pra "*arquivo" quando ele é modificado, e volta para "arquivo" quando ele é salvo
 *
 * Foi utilizada a lógica de subir hierarquia de classes do QT através do sender(), um método pra descobrir quem enviou o
 * signal "modified"
 */
void MainWindow::onDocumentModified(bool modified) {

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
            //mid 1 extrai todos caracteres a partir do 1
            ui->tabCodeEditor->setTabText(tabIdx, tabName.mid(1));  //basicamente tira fora o "*"
        }
    }
}

/*
 * Abre o editor atual, pega o path do arquivo q ele representa, abre uma stream de escrita, escreve o conteudo
 * do editor dentro desse arquivo (reescreve tudo mesmo) e emite um signal de q agora n foi mais modificado (ta igual disco)
*/
void MainWindow::saveCurrentFile() {
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabCodeEditor->currentWidget());
    QString path = editor->property("filepath").toString();

    // Se deu merda na hora de salvar imprime
    if(!project->saveFileShortcut(path, editor->toPlainText())){
        ui->consoleOutput->appendPlainText("Erro durante salvamento do arquivo");
    }
    editor->document()->setModified(false);
}

/*
 * percorre tabs, acha oq tem filepath igual e salva.
 * horrivel melhor usar o ctrl+s, mas se o puto quiser tem.
 */
void MainWindow::saveFile(const QString &path) {
    for (int i = 0; i < ui->tabCodeEditor->count(); i++) {
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabCodeEditor->widget(i));
        if (editor && editor->property("filepath").toString() == path) {
            if (!project->saveFileShortcut(path, editor->toPlainText())) {
                ui->consoleOutput->appendPlainText("Erro durante salvamento do arquivo");
            }
            editor->document()->setModified(false);
            return;
        }
    }
}

/*
 * Escuta o signal da CLI embutida (quando o cara pressiona enter)
 * e envia a string capturada pra classe q gerencia o Console
 * limpando oq tiver escrito
 */
void MainWindow::onCommandEntered() {
    QString line = ui->commandInput->text();
    console->executeCommand(line);
    ui->commandInput->clear();
}


/*
 * O BOTAO TA MANDANDO PATH ERRADO E TA EXPLODINDO O ASSEMBLER
 * A INTERFACE DE COMANDO POR LINHA FUNCIONA
 */

void MainWindow::onAssembleClicked() {
    // ui->consoleOutput->appendPlainText("assemble clicked");

    // Aqui a própria janela gerencia a lógica interna
    // com o mesmo controller (project)
    // oq eu fiz com o btnRun foi diferente por causa dos objetos (e foi meio gambiarra)
    btnBuild_popup dialog(project, "assemble", this);
    dialog.exec();
}

void MainWindow::onLinkClicked() {
    // ui->consoleOutput->appendPlainText("link clicked");
    btnBuild_popup dialog(project, "link", this);
    dialog.exec();
}

void MainWindow::onBuildClicked() {
    // ui->consoleOutput->appendPlainText("build clicked");
    btnBuild_popup dialog(project, "build", this);
    dialog.exec();
}

void MainWindow::onLoadClicked() {
    btnRun_popup dialog(project->getRootDir(), this);

    if(dialog.exec() == QDialog::Accepted) {
        project->resetCpu();
        project->load(dialog.getSelectedFileName());
        ui->consoleOutput->appendPlainText("File loaded successfully");
    } else {
        ui->consoleOutput->appendPlainText("Failed to laod .bin file");
    }
}

void MainWindow::onRunClicked() {
    btnRun_popup dialog(project->getRootDir(), this);

    if(dialog.exec() == QDialog::Accepted) {
        project->resetCpu();
        project->run(dialog.getSelectedFilePath());
        ui->consoleOutput->appendPlainText("Runned succesfully");
    } else {
        ui->consoleOutput->appendPlainText("Failed to run .bin file");
    }
}

void MainWindow::onStepClicked() {
    if(project->isLoaded()) {
        project->step();
        // ui->consoleOutput->appendPlainText("step");
    } else {
        ui->consoleOutput->appendPlainText("Nenhum programa carregado na memória");
    }
}

void MainWindow::onResetClicked() {
    project->resetCpu();
    ui->consoleOutput->appendPlainText("CPU Reseted");
}

void MainWindow::updateRegFlagTable(const VMState &state) {
    //da pra adicionar na interface dps um label com o state da VM (idle, running etc)
    // e atualizar via state snapshot tb
    ui->tableRegs->item(0, 1)->setText(QString::number(state.A));
    ui->tableRegs->item(1, 1)->setText(QString::number(state.B));
    ui->tableRegs->item(2, 1)->setText(QString::number(state.C));
    ui->tableRegs->item(3, 1)->setText(QString::number(state.D));
    ui->tableRegs->item(4, 1)->setText(QString::number(state.E));
    ui->tableRegs->item(5, 1)->setText(QString::number(state.H));
    ui->tableRegs->item(6, 1)->setText(QString::number(state.L));
    ui->tableRegs->item(7, 1)->setText(QString::number(state.F));
    ui->tableRegs->item(8, 1)->setText(QString::number(state.PC));
    ui->tableRegs->item(9, 1)->setText(QString::number(state.SP));

    ui->tableFlags->item(0, 1)->setText(state.flagZ ? "1" : "0");
    ui->tableFlags->item(1, 1)->setText(state.flagS ? "1" : "0");
    ui->tableFlags->item(2, 1)->setText(state.flagC ? "1" : "0");
    ui->tableFlags->item(3, 1)->setText(state.flagH ? "1" : "0");
    ui->tableFlags->item(4, 1)->setText(state.flagN ? "1" : "0");
    ui->tableFlags->item(5, 1)->setText(state.flagPV ? "1" : "0");
}
