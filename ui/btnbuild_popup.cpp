#include "btnbuild_popup.h"
#include "ui_btnbuild_popup.h"
#include <QFileDialog>

btnBuild_popup::btnBuild_popup(const QString &rootPath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::btnBuild_popup)
{
    ui->setupUi(this);
    this->rootPath = rootPath;
    //connect(QUEM_EMITE,       &CLASSE::SINAL,                 QUEM_ESCUTA,        &CLASSE::FUNÇÃO_SLOT);

    connect(ui->btnAdd,     &QPushButton::clicked, this, &btnBuild_popup::onAddFile);
    connect(ui->btnRemove,  &QPushButton::clicked, this, &btnBuild_popup::onRemoveFile);
}

btnBuild_popup::~btnBuild_popup()
{
    delete ui;
}

void btnBuild_popup::onAddFile() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Selecionar arquivos",
        rootPath,
        "Assembly (*.asm);;Todos (*)"
    );
}

void btnBuild_popup::onRemoveFile() {

}
