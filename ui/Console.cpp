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
    } else if(command == "rmdir") {
        deleteDir(tokens);
    }

    /*else if(command == "save") {
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

    // espera receber mkdir "path"
    // path sempre inclui a root, mas fica abstraido pro usuario
    QString path = tokens [1];
    bool success = project->createDirectory(path);

    if(success) {
        emit output("Directory created");
    } else {
        emit output("Failed to create directory");
    }
}

void Console::deleteDir(const QStringList &tokens) {
    if(tokens.size() < 2) {
        emit output("Missing operand on rmdir");
        return;
    }

    QString path = tokens[1];
    bool success = project->deleteDirectory(path);

    if(success) {
        emit output("Directory deleted.");
    } else {
        emit output("Failed to delete directory.");
    }

}






















