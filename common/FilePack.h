#ifndef FILEPACKER_H
#define FILEPACKER_H

#include <zip.h>
#include <QString>
#include <QVector>
#include <qtextstream.h>
#include <boost/concept_check.hpp>
#include <QList>

class FilePack
{
public:
  enum flags{
    WRITE,
    READ
  };
  enum fileNames{
    TRACE,
    TAXONOMY,
    //SEGMENTATION
  };
  
  
  FilePack(QString FilePackName, flags flag);
  ~FilePack();

  bool fileCreated();

  //! Read a file with name @param fileName inside @param FilePackName
  void readFile(FilePack::fileNames name, QTextStream& data);
  //! Add a source file with name @param fileName inside pack-file
  int addSource(FilePack::fileNames name, QString& data);
  //! Create the file in the filesystem and close it. Return if there had been errors or not.
  bool close();
  
private:
  struct zip* m_file;
  int m_error; // Error number

  QList<QString> m_TmpFilesToRemove;
  QString getRealName(fileNames name);
};


class IOEspinaFile
{
public:
  static void loadFile(QString filePath,
                       QTextStream& TraceContent,
                       QTextStream& TaxonomyContent);

  static void saveFile(QString& filePath,
                       QString& TraceContent,
                       QString& TaxonomyContent);
  
};
#endif // FILEPACKER_H
