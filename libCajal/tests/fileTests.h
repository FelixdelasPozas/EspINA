#ifndef FILETESTS_H
#define FILETESTS_H

#include <QString>
#include <QStringList>
#include <QDir>

//! Returns whether files' content differ or not
//! false: Same content
//! true: Different content
bool fileDiff(QString filePath1, QString filePath2);

void recursiveRemoveDir(char* dir);

/**
 * Retrieve all the file paths inside a specific directory and recursively iterates
 * through the inside directories
 */
QStringList recursiveFilePaths(QDir searchPath);

#endif //FILETESTS_H
