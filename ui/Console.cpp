#include "Console.h"
#include "Project.h"


Console::Console(Project *project, QObject *parent)
    : QObject(parent),
    project(project) {
}

void Console::executeCommand(const QString &line) {
    QStringList tokens = line.split(" ", Qt::SkipEmptyParts);

    if(tokens.isEmpty()) { return; }

    QString command = tokens[0];

    if(command == "clear") {
        emit clearRequested(); //emite sinal
    } else if(command == "mkdir") {
        createDir(tokens);
    } /*else if(command == "save") {
        project
    }*/


    else {
        emit output("Unknown operand");
    }
}

void Console::createDir(const QStringList &tokens) {
    if(tokens.size() < 2) { //comando mkdir solto
        emit output("Missing operand on mkdir");
        return;
    }

    QString name = tokens[1];
    QString path = ""; //por padrao vem vazio

    if(tokens.size() >= 3) { //se o cara digitou path
        path = tokens[2];
    }

    bool success = project->createDirectory(name, path);

    if(success) {
        emit output("Directory created");
    } else {
        emit output("Failed to create directory");
    }
}
