#ifndef FILEPACKER_H
#define FILEPACKER_H

#include </usr/include/zip.h>
#include <QString>
#include <QVector>
#include <qtextstream.h>
#include <boost/concept_check.hpp>
#include <QStringList>
#include <qdir.h>


class FilePack
{
public:
  enum flags{
    WRITE,
    READ
  };

  
  FilePack(QString FilePackName, flags flag);
  ~FilePack();

  bool fileCreated();

  //! Read a file with name @param name inside @param FilePackName
  void readFile(QString name, QTextStream& data);
  //! Add a source file with name @param name inside pack-file
  int addSource( QString fileName, QString& source);
  //! Create the file in the filesystem and close it. Return if there had been errors or not.
  bool close();
  //! Insert a new directory in the pack-file
  void addDir ( QDir path );
  //! Insert the file @param filePath inside the pack-file
  int addFile( QFileInfo file, QString fileNameInPack = "");
  //! Extract the files which has the product of a filter. For disk cache
  void ExtractFiles(QDir& filePath);
    
  
private:
  struct zip* m_file;
  int m_error; // Error number

  QList<QString> m_TmpFilesToRemove;
};

class IOEspinaFile
{
  
public:
  static bool loadFile(QString filePath,
                       QTextStream& TraceContent,
                       QTextStream& TaxonomyContent);

  static bool saveFile(QString& filePath,
                       QString& TraceContent,
                       QString& TaxonomyContent,
                       QStringList& segmentationPaths
                      );
  
};
#endif // FILEPACKER_H
