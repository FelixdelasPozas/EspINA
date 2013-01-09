#ifndef HELPERS_H
#define HELPERS_H

#include <QString>
#include <QStringList>
#include <QDir>

int loadWithOutRequiredFile(const char* fileName);
bool differentFiles(QString filePath1, QString filePath2);
void recursiveRemoveDir(char* dir);

/**
 * Retrieve all the file paths inside a specific directory and recursively iterates
 * through the inside directories
 */
QStringList recursiveFilePaths(QDir searchPath);


int saveSimpleFile(QString referenceSegFile, QString destination="save.seg");
int loadSimpleFile(QDir fileDir, QString referenceFileName );

#endif
