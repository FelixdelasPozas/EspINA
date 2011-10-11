#ifndef FILEPACKER_H
#define FILEPACKER_H

// #include </usr/include/zip.h>
#include <QString>
#include <QVector>
#include <qtextstream.h>
#include <boost/concept_check.hpp>
#include <QStringList>
#include <qdir.h>
#include <quazipfile.h>
/*
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
*/
class IOEspinaFile
{
  
public:
  /**
   * Loads a seg file. It must contain a trace and a taxonomy files. To build the
   * hole pipeline.
   * @param filepath is the path of the seg file
   * @param TraceContent is an output parameter which will contain the content of the trace file
   * @param TaxonomyContent is an output parameter which will contain the content of the taxonomy file
   * @return If everythin works well it returns true. Otherwise returns false.
   */
  static bool loadFile(QString filePath,
                       QTextStream& TraceContent,
                       QTextStream& TaxonomyContent);

  /**
   * Stores in a seg file (packed) all the pipeline built.
   * @param TraceContent is the content of the trace file
   * @param TaxonomyContent is the content of the taxonomy file
   * @param segmentationPaths is a list of file paths with the file generated per segmentaion. 
   * This files are packed inside the new file @param filePath
   * @param commonPathToRemove is a list of temporary files generated during the save process. They
   * will be removed after the file was saved.
   * @return If everythin works well it returns true. Otherwise returns false.
   */
  static bool saveFile(QString& filePath,
                       QString& TraceContent,
                       QString& TaxonomyContent,
                       QStringList& segmentationPaths,
                       QString commonPathToRemove
                      );
  
private:
  /**
   * Creates a zipped file called @param fileName inside @param zFile. @param content 
   * has the information to store in @param fileName. It controls the correct 
   * compression.
   * @return If everythin works well it returns true. Otherwise returns false.
   */
  static bool zipFile(QString fileName, QByteArray content, QuaZipFile& zFile);
  
};
#endif // FILEPACKER_H
