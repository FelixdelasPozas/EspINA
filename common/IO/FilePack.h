#ifndef FILEPACKER_H
#define FILEPACKER_H

#include <QFileInfo>
#include <model/EspinaModel.h>

class QuaZipFile;
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
  static bool loadFile(QFileInfo file,
                       QSharedPointer<EspinaModel> model
                      );


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
  static bool saveFile(QFileInfo file,
                       QSharedPointer<EspinaModel> model
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
