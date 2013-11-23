#include "EspinaIO.h"

#include <QFile>

// EspINA
#include "Core/Model/EspinaModel.h"
#include "Core/Model/Output.h"
#include "Core/Model/EspinaFactory.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/Taxonomy.h>
#include <Core/Extensions/ChannelExtension.h>
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/IO/IOErrorHandler.h>

#include "Core/Filters/ChannelReader.h"
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>

// ITK
#include <itkImageFileWriter.h>
#include <itkMetaImageIO.h>

// VTK
#include <vtkMetaImageReader.h>
#include <vtkTIFFReader.h>

// Qt
#include <QDebug>
#include <QDir>
#include <QXmlStreamReader>

// Quazip
#include <quazipfile.h>

#include <stack>

using namespace EspINA;

const QString TRACE_FILE    = "trace.dot";
const QString CLASSIFICATION_FILE = "taxonomy.xml";

typedef itk::ImageFileWriter<itkVolumeType> EspinaVolumeWriter;

const QString SETTINGS = "settings.ini";
const QString EspinaIO::VERSION = "version"; //backward compatibility
const QString SEG_FILE_VERSION  = "4";
const QString SEG_FILE_COMPATIBLE_VERSION  = "1";

//-----------------------------------------------------------------------------
bool EspinaIO::isChannelExtension(const QString &fileExtension)
{
  return ("mha"  == fileExtension
       || "mhd"  == fileExtension
       || "tiff" == fileExtension
       || "tif"  == fileExtension);
}

//-----------------------------------------------------------------------------
IOErrorHandler::STATUS EspinaIO::loadFile(QFileInfo file,
                                    IEspinaModel *model,
                                    IOErrorHandler *handler)
{
  const QString ext = file.suffix();
  if (isChannelExtension(ext))
  {
    ChannelSPtr channel;
    return loadChannel(file, model, channel, handler);
  }

  if ("seg" == ext)
    return SegFileReader::loadSegFile(file, model, handler);

  return model->factory()->readFile(file.absoluteFilePath(), ext, handler) ? IOErrorHandler::SUCCESS : IOErrorHandler::IO_ERROR;
}

//-----------------------------------------------------------------------------
IOErrorHandler::STATUS EspinaIO::loadChannel(QFileInfo file,
                                             IEspinaModel *model,
                                             ChannelSPtr &channel,
                                             IOErrorHandler *handler)
{
  SampleSPtr existingSample;

  EspinaFactory *factory = model->factory();

  if (!existingSample)
  {
    //TODO Try to recover sample form OMERO using channel information or prompt dialog to create a new sample
    existingSample = factory->createSample(file.baseName());

    model->addSample(existingSample);
  }

  //TODO: Recover channel information from centralized system (OMERO)
  QColor stainColor = QColor(Qt::black);

  Filter::NamedInputs noInputs;
  Filter::Arguments readerArgs;

  if (!file.exists())
  {
    if (handler)
      file = handler->fileNotFound(file);

    if (!file.exists())
      return IOErrorHandler::FILE_NOT_FOUND;
  }

  readerArgs[ChannelReader::FILE] = file.absoluteFilePath();
  FilterSPtr reader(new ChannelReader(noInputs, readerArgs, ChannelReader::TYPE, handler));
  reader->setGraphicalRepresentationFactory(GraphicalRepresentationFactorySPtr(new BasicGraphicalRepresentationFactory()));
  reader->update();
  if (reader->numberOfOutputs() == 0)
    return IOErrorHandler::IO_ERROR;

  Channel::CArguments args;
  args[Channel::ID] = file.fileName();
  args.setHue(stainColor.hueF());
  channel = factory->createChannel(reader, 0);
  channel->setHue(stainColor.hueF());// It is needed to display proper stain color
  //file.absoluteFilePath(), args);
  channel->setOpacity(-1.0);
  if (channel->hue() != -1.0)
    channel->setSaturation(1.0);
  else
    channel->setSaturation(0.0);
  channel->setContrast(1.0);
  channel->setBrightness(0.0);

  double pos[3];
  existingSample->position(pos);
  channel->setPosition(pos);

  model->addFilter(reader);
  model->addChannel(channel);
  model->addRelation(reader, channel, Channel::VOLUME_LINK);
  model->addRelation(existingSample, channel, Channel::STAIN_LINK);

  channel->initialize(args);
  channel->initializeExtensions();

  return IOErrorHandler::SUCCESS;
}
