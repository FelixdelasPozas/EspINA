#ifndef ESPINAIO_H
#define ESPINAIO_H

#include <Core/Model/Channel.h>
#include <Core/IO/IOErrorHandler.h>
#include <QFileInfo>

namespace EspINA
{
  class IEspinaModel;

  class EspinaIO
  {
    static const QString VERSION;

  public:
    static bool isChannelExtension(const QString &fileExtension);
    /**
     * Loads any file supported by EspINA.
     * @param file is the absolute path to be loaded
     * @param model is the EspinaModel in which the file is loaded into
     * @return Success if no other error is reported.
     */
    static IOErrorHandler::STATUS loadFile(QFileInfo       file,
                                           IEspinaModel   *model,
                                           IOErrorHandler *handler = NULL);

    /**
     * Load channel files supported by EspINA. Current implementation
     * supports the following extensions: mha, mhd, tiff, tif
     * @param file is the absolute path to be loaded
     * @param model is the EspinaModel in which the file is loaded into
     * @param channelPtr is used to retrieve loaded channel if loading was successful
     * @return Success if no other error is reported.
     */
    static IOErrorHandler::STATUS loadChannel(QFileInfo       file,
                                              IEspinaModel   *model,
                                              ChannelSPtr    &channel,
                                              IOErrorHandler *handler = NULL);
  };


}// namespace EspINA

#endif // ESPINAIO_H
