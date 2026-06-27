#include "btnbuild_popup.h"
#include "ui_btnbuild_popup.h"
#include <QFileDialog>

btnBuild_popup::btnBuild_popup(Project *project, const QString &action, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::btnBuild_popup)
{
    ui->setupUi(this);
    this->project = project;
    this->rootPath = project->getRootDir();
    this->action = action;

    setWindowTitle("File selection");

    //connect(QUEM_EMITE,       &CLASSE::SINAL,                 QUEM_ESCUTA,        &CLASSE::FUNÇÃO_SLOT);
    connect(ui->btnAdd,     &QPushButton::clicked, this, &btnBuild_popup::onAddFile);
    connect(ui->btnRemove,  &QPushButton::clicked, this, &btnBuild_popup::onRemoveFile);
}

btnBuild_popup::~btnBuild_popup()
{
    delete ui;
}

void btnBuild_popup::onAddFile() {
    // A string (ultimo parâmetro) define asopções de filtragem do dropdown do qfiledialog
    // formato: "Nome visível (*.extensao);;Outro nome (*.ext2)"

    QFileDialog dialog(this, "Selecionar arquivos", rootPath, "Assembly (*.asm);;Binário (*.bin);;Todos (*)");
    dialog.setDirectory(rootPath);
    dialog.setOption(QFileDialog::DontResolveSymlinks);
    dialog.setDirectoryUrl(QUrl::fromLocalFile(rootPath)); //limita o fileDialog pra só mostrar a root(pasta workspace do projeto)
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if(dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        for (const QString &file : files) {
            auto matches = ui->fileList->findItems(file, Qt::MatchExactly);
            if (matches.isEmpty()) {
                ui->fileList->addItem(file);
            }
        }
    }
}

// pega o arquivo selecionado no List Widget e deleta quando o botão for clicado
void btnBuild_popup::onRemoveFile() {
    delete ui->fileList->takeItem(ui->fileList->currentRow());
}

void btnBuild_popup::on_buttonBox_accepted() {
    QStringList files;


    for(int i = 0; i < ui->fileList->count(); i++) {
        //assemble faz resolvePath pra funcionar via terminal
        // é um passo desnecessário aq, mas pra evitar 2 funções 1 pra botão e 1 pra terminal
        // esse monte de conversão garante q eu to enviando o caminho relativo à root
        // e.g: src/main (eu envio src/main junto, pq o resolve path só concatena root)
        QFileInfo fileInfo(ui->fileList->item(i)->text());
        QString relativePath = QDir(project->getRootDir()).relativeFilePath(fileInfo.absoluteFilePath());

        files.append(fileInfo.fileName());
    }

    if(action == "assemble") {
        project->assemble(files);
    } else if (action == "link") {
        project->link(files);
    } else if (action == "build") {
        project->build(files);
    }
}

