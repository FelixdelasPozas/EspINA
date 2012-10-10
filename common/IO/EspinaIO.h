#ifndef ESPINAIO_H
#define ESPINAIO_H

#include "common/model/EspinaModel.h"
#include "common/model/Filter.h"

#include <QFileInfo>
#include <QDir>

class QUndoStack;
class QuaZipFile;
class QuaZipFile;

class EspinaIO
{
public:
  enum STATUS
  { SUCCESS
  , FILE_NOT_FOUND
  , ERROR
  };

  /**
   * Loads any file supported by EspINA.
   * @param file is the absolute path to be loaded
   * @param model is the EspinaModel in which the file is loaded into
   * @return Success if no other error is reported.
   */
  static STATUS loadFile(QFileInfo file, EspinaModel* model, QUndoStack* undoStack);

  /**
   * Load channel files supported by EspINA. Current implementation
   * supports the following extensions: mha, mhd, tiff, tif
   * @param file is the absolute path to be loaded
   * @param model is the EspinaModel in which the file is loaded into
   * @param undosktack loaded channels command are added to the undo stack
   * @param channelPtr is used to retrieve loaded channel if loading was successful
   * @return Success if no other error is reported.
   */
  static STATUS loadChannel(QFileInfo file, EspinaModel *model, QUndoStack *undoStack, Channel **channelPtr=NULL);

  /**
   * Loads a seg file which must contain at least a trace and a taxonomy file.
   * @param filepath is the path of the seg file
   * @param model is the EspinaModel in which the file is loaded into
   * @return Success if no other error is reported.
   */
  static STATUS loadSegFile(QFileInfo file, EspinaModel *model);

  /**
   * Create a new seg file containing all information provided by @param model
   * @param filepath is the path where the model must be saved
   * @param model is the EspinaModel which is saved in @param file
   */
  static STATUS saveFile(QFileInfo file, EspinaModel *model);

private:
  /**
   * Creates a zipped file called @param fileName inside @param zFile. @param content 
   * has the information to store in @param fileName. It controls the correct 
   * compression.
   * @return If everythin works well it returns true. Otherwise returns false.
   */
  static bool zipVolume(Filter *filter,
                        OutputNumber outputNumber,
                        QDir tmpDir,
                        QuaZipFile &outFile);
  static bool zipFile(QString fileName, QByteArray content, QuaZipFile& zFile);
};
#endif // ESPINAIO_H