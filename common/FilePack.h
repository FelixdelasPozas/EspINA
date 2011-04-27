#ifndef FILEPACKER_H
#define FILEPACKER_H

#include <zip.h>
#include <QString>

class FilePack
{
public:
  FilePack(QString FilePackName);
  ~FilePack();

  bool fileCreated();
  //! Add a source file with name @param fileName inside pack-file
  int addTextSource(QString fileName, QString& data);
  //! Create the file in the filesystem and close it. Return if there had been errors or not.
  bool close();
  
private:
  struct zip* m_file;
  int m_error; // Error number
};

#endif // FILEPACKER_H
