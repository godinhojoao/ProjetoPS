#include "btnrun_popup.h"
#include "ui_btnrun_popup.h"
#include <QDirIterator>

btnRun_popup::btnRun_popup(const QString &rootPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::btnRun_popup)
{
    ui->setupUi(this);
    this->rootPath = rootPath;

    // Formatação de texto fixa
    setWindowTitle("File selection");

    //Preenche a comboBox
    //(1. onde a busca inicia, 2. filtro por nome (qualquer nome com .bin entra na lista),
    // 3. opção doq vai ser buscado (podia incluir diretório por exemplo),
    // 4. diz como o iterador vai percorrer, o subdirectories entra dentro de diretórios)
    // dps de instanciado o QDirIterator é uma lista com os caminhos (strings)
    QDirIterator fileOpt(rootPath, QStringList() << "*.bin", QDir::Files, QDirIterator::Subdirectories);

    while(fileOpt.hasNext()) {
        QString filePath = fileOpt.next();
        QFileInfo fileInfo(filePath);

        // O comboBox permite receber o nome do item + uma informação associada àquele nome
        // o segundo parâmetro é invisível pro usuário
        ui->fileOptions->addItem(fileInfo.fileName(), filePath);
    }

}

btnRun_popup::~btnRun_popup()
{
    delete ui;
}

// pega o caminho relativo à ROOT (usado pra executar)
QString btnRun_popup::getSelectedFilePath() const {
    return QDir(rootPath).relativeFilePath(ui->fileOptions->currentData().toString());
}

QString btnRun_popup::getSelectedFileName() const {
    return ui->fileOptions->currentText();
}
