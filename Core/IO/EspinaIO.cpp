#include "EspinaIO.h"

#include <QFile>

// EspINA
#include "Core/Model/EspinaModel.h"
#include "Core/Model/Filter.h"
#include "Core/Model/EspinaFactory.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/Taxonomy.h>
#include <Core/Extensions/ChannelExtension.h>
#include <Core/Extensions/SegmentationExtension.h>
#include "ErrorHandler.h"

#include "Filters/ChannelReader.h"

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
const QString TAXONOMY_FILE = "taxonomy.xml";

typedef itk::ImageFileWriter<itkVolumeType> EspinaVolumeWriter;

const QString EspinaIO::VERSION = "version";
const QString SEG_FILE_VERSION  = "1";

//-----------------------------------------------------------------------------
bool EspinaIO::isChannelExtension(const QString &fileExtension)
{
  return ("mha"  == fileExtension
       || "mhd"  == fileExtension
       || "tiff" == fileExtension
       || "tif"  == fileExtension);
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::loadFile(QFileInfo file,
                                    EspinaModel *model,
                                    ErrorHandler *handler)
{
  const QString ext = file.suffix();
  if (isChannelExtension(ext))
  {
    ChannelSPtr channel;
    return loadChannel(file, model, channel, handler);
  }

  if ("seg" == ext)
    return loadSegFile(file, model, handler);

  return model->factory()->readFile(file.absoluteFilePath(), ext, handler) ? SUCCESS : ERROR;
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::loadChannel(QFileInfo file,
                                       EspinaModel *model,
                                       ChannelSPtr &channel,
                                       ErrorHandler *handler)
{
  //TODO 2012-10-07
  // Try to recover sample form DB using channel information
  SampleSPtr existingSample;

  EspinaFactory *factory = model->factory();

  if (existingSample.isNull())
  {
    // TODO: Look for real channel's sample in DB or prompt dialog
    // Try to recover sample form DB using channel information
    existingSample = factory->createSample(file.baseName());

    model->addSample(existingSample);
  }

  //TODO: Check for channel information in DB
  QColor stainColor = QColor(Qt::black);

  Filter::NamedInputs noInputs;
  Filter::Arguments readerArgs;

  if (!file.exists())
  {
    if (handler)
      file = handler->fileNotFound(file);

    if (!file.exists())
      return FILE_NOT_FOUND;
  }

  readerArgs[ChannelReader::FILE] = file.absoluteFilePath();
  FilterSPtr reader(new ChannelReader(noInputs, readerArgs, ChannelReader::TYPE, handler));
  reader->update();
  if (reader->outputs().isEmpty())
    return ERROR;

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

  return SUCCESS;
}


//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::loadSegFile(QFileInfo    file,
                                       EspinaModel *model,
                                       ErrorHandler *handler)
{
  // generate random dir based on file name
  QDir temporalDir = QDir::tempPath();
  QString random = QString::number(rand());
  while (temporalDir.exists(file.baseName() + QString("_") + random))
      random = QString::number(rand());

  QString temporalDirName = file.baseName() + QString("_") + random;
  temporalDir.mkdir(temporalDirName);
  temporalDir.cd(temporalDirName);

  QuaZip espinaZip(file.filePath());
  if( !espinaZip.open(QuaZip::mdUnzip) )
  {
    if (handler)
      handler->error("IOEspinaFile: Could not open file" + file.filePath());
    return FILE_NOT_FOUND;
  }

  QuaZipFile espinaFile(&espinaZip);

  TaxonomySPtr taxonomy;
  QString traceContent;
  QMap<QString, ModelItem::ExtensionPtr> extensionFiles;

  bool hasFile = espinaZip.goToFirstFile();
  while (hasFile)
  {
    QFileInfo file = espinaFile.getActualFileName();
    if (!espinaFile.open(QIODevice::ReadOnly))
    {
      if (handler)
        handler->error("IOEspinaFile: Could not extract the file" + file.filePath());
      if (file == TAXONOMY_FILE || file == TRACE_FILE)
        return ERROR;

      continue;
    }
    // qDebug() << "EspinaIO::loadSegFile: extracting" << file.filePath();
    if (file.fileName() == VERSION)
    {
      QString versionNumber = espinaFile.readAll();
      if (versionNumber < SEG_FILE_VERSION)
      {
        if (handler)
          handler->error(QObject::tr("Invalid seg file version. File Version=%1, current Version %2").arg(versionNumber).arg(SEG_FILE_VERSION));
        removeTemporalDir(temporalDir);
        espinaFile.close();
        return INVALID_VERSION;
      }
    }
    else
      if (file.fileName() == TAXONOMY_FILE)
      {
        Q_ASSERT(taxonomy.isNull());
        taxonomy = IOTaxonomy::loadXMLTaxonomy(espinaFile.readAll());
        //taxonomy->print(3);
      }
      else
        if (file.fileName() == TRACE_FILE)
        {
          Q_ASSERT(traceContent.isEmpty());
          QTextStream traceStream(&traceContent);
          traceStream << espinaFile.readAll();
        }
        else
        {
          bool extensionFile = false;

          // Check whether is a Channel Extension
          {
            int i = 0;
            Channel::ExtensionList extensions = model->factory()->channelExtensions();
            while (!extensionFile && i < extensions.size())
            {
              Channel::ExtensionPtr extension = extensions[i];

              if (extension->isCacheFile(file.filePath()))
              {
                extensionFile = true;
                extensionFiles[file.filePath()] = extension;
              }
              i++;
            }
          }

          // Check whether is a Segmentation Extension
          if (!extensionFile)
          {
            int i = 0;
            Segmentation::InformationExtensionList extensions = model->factory()->segmentationExtensions();
            while (!extensionFile && i < extensions.size())
            {
              Segmentation::InformationExtension extension = extensions[i];

              if (extension->isCacheFile(file.filePath()))
              {
                extensionFile = true;
                extensionFiles[file.filePath()] = extension;
              }
              i++;
            }
          }

          // Otherwise is an Espina Volumes
          if (!extensionFile)
          {
            QFile destination(temporalDir.path() + QString("/") + file.fileName());
            /*qDebug()<< "Permissions set" <<
             *       destination.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
             *                                  QFile::ReadGroup | QFile::ReadOther);*/
            if (!destination.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
              if (handler)
                handler->warning("IOEspinaFile::loadFile: could not create file " + destination.fileName() + " in " + temporalDir.path());
            }
            destination.write(espinaFile.readAll());
            destination.close();
          }
        }

    espinaFile.close();
    hasFile = espinaZip.goToNextFile();
  }

  if(taxonomy.isNull() || traceContent.isEmpty())
  {
    if (handler)
      handler->error("IOEspinaFile::loadFile: could not load taxonomy and/or trace files");
    removeTemporalDir(temporalDir);
    return ERROR;
  }

  STATUS status;
  model->addTaxonomy(taxonomy);
  std::istringstream trace(traceContent.toStdString().c_str());
  status = model->loadSerialization(trace, temporalDir, handler) ? SUCCESS : ERROR;

  // Load Extensions' cache
  foreach(QString file, extensionFiles.keys())
  {
    espinaZip.setCurrentFile(file);
    espinaFile.open(QuaZipFile::ReadOnly);
    extensionFiles[file]->loadCache(espinaFile, temporalDir, model);
    espinaFile.close();
  }

  espinaZip.close();
  return status;
}

//-----------------------------------------------------------------------------
bool EspinaIO::zipVolume(Filter::Output output,
                         QDir tmpDir,
                         QuaZipFile& outFile)
{
  itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
  EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();
  FilterPtr filter = output.filter;
  QString volumeName = QString("%1_%2").arg(filter->id()).arg(output.id);
  QString mhd = tmpDir.absoluteFilePath(volumeName + ".mhd");
  QString raw = tmpDir.absoluteFilePath(volumeName + ".raw");
  io->SetFileName(mhd.toStdString());
  writer->SetFileName(mhd.toStdString());
  filter->update();
  itkVolumeType::Pointer volume = output.volume->toITK();
  bool releaseFlag = volume->GetReleaseDataFlag();
  volume->ReleaseDataFlagOff();
  writer->SetInput(volume);
  writer->SetImageIO(io);
  writer->Write();
  volume->SetReleaseDataFlag(releaseFlag);

  QFile mhdFile(mhd);
  mhdFile.open(QIODevice::ReadOnly);
  QFile rawFile(raw);
  rawFile.open(QIODevice::ReadOnly);

  if( !zipFile(volumeName + ".mhd", mhdFile.readAll() , outFile) )
  {
    qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".mhd");
    return false;
  }
  if( !zipFile(volumeName + ".raw", rawFile.readAll() , outFile) )
  {
    qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".raw");
    return false;
  }

  mhdFile.close();
  rawFile.close();

  return true;
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::saveSegFile(QFileInfo file, EspinaModel *model, ErrorHandler *handler)
{
  if (file.baseName().isEmpty())
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile file name is empty");
    return ERROR;
  }

  QFile zFile(file.absoluteFilePath());
  QuaZip zip(&zFile);
  if(!zip.open(QuaZip::mdCreate)) 
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile" + zFile.fileName() + "error while creating file");
    return ERROR;
  }
  QuaZipFile outFile(&zip);

  // Store Version Number
  zipFile(VERSION, SEG_FILE_VERSION.toUtf8(), outFile);

  // Save Taxonomy
  QString taxonomy;
  IOTaxonomy::writeXMLTaxonomy(model->taxonomy(), taxonomy);
  if( !zipFile(QString(TAXONOMY_FILE), taxonomy.toAscii(), outFile) )
    return ERROR;

  // Save Trace
  std::ostringstream trace;
  model->serializeRelations(trace);
  if( !zipFile(QString(TRACE_FILE),  trace.str().c_str(), outFile) )
    return ERROR;

  // Store filter data
  foreach(FilterSPtr filter, model->filters())
  {
    QList<QPair<QString, QByteArray> > fileList;
    if (filter->dumpSnapshot(fileList))
    {
      QPair<QString, QByteArray> entry;
      foreach(entry, fileList)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
          return ERROR;
      }
    }
  }

  // Save Extensions' Information
  foreach(Channel::ExtensionPtr extension, model->factory()->channelExtensions())
  {
    ModelItem::Extension::CacheList cacheList;
    if (extension->saveCache(cacheList))
    {
      QPair<QString, QByteArray> entry;
      foreach(entry, cacheList)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
          return ERROR;
      }
    }
  }

  foreach(Segmentation::InformationExtension extension, model->factory()->segmentationExtensions())
  {
    ModelItem::Extension::CacheList cacheList;
    if (extension->saveCache(cacheList))
    {
      QPair<QString, QByteArray> entry;
      foreach(entry, cacheList)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
          return ERROR;
      }
    }
  }

  zip.close();
  if (zip.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile ERROR: close" + file.absoluteFilePath() + "zip file");
    return ERROR;
  }

  return SUCCESS;
}

//-----------------------------------------------------------------------------
bool EspinaIO::zipFile(QString fileName, const QByteArray &content, QuaZipFile& zFile)
{
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);
  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if( !zFile.open(QIODevice::WriteOnly, zFileInfo) )
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not open " << fileName 
              << "inside" << zFile.getFileName() 
              << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.write(content);
  if(zFile.getZipError()!=UNZ_OK) 
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not store the content in" << fileName
            << "inside" << zFile.getFileName() 
            << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.close();
  if(zFile.getZipError()!=UNZ_OK) {
    qWarning() << "IOEspinaFile::zipFile(): Could not close the file" << fileName
           << "inside" << zFile.getFileName() 
           << ". Code error:" << zFile.getZipError();
    return false;
  }
  return true;
}

/****************
 ** IOTaxonomy **
 ****************/

IOTaxonomy::IOTaxonomy()
{
}

IOTaxonomy::~IOTaxonomy()
{
}

QString concatenate(std::stack<QString> hierarchy)
{
  QString res;
  while (!hierarchy.empty())
  {
    res = QString(hierarchy.top()) + "/" + res;
    hierarchy.pop();
  }
  return res;
}

TaxonomySPtr IOTaxonomy::readXML(QXmlStreamReader& xmlStream)
{
  // Read the XML
//   QXmlStreamReader xmlStream(&file);
  QStringRef nodeName, color;
  TaxonomySPtr tax(new Taxonomy());
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        color = xmlStream.attributes().value("color");
//         if( taxHierarchy.empty() )
//         {
//           tax = new Taxonomy();
//         }
//         else
//         {
          QString qualified = concatenate(taxHierarchy);
          // TODO 2012-12-15 Cambiar el algoritmo de cargar los nodos, no tiene sentido ir acumulando
          // y calculando los nombres cualificados si siempre se accede de forma secuencial...
          TaxonomyElementSPtr parent = tax->element(qualified);
          TaxonomyElementSPtr node   = tax->createElement( nodeName.toString(), parent);
          node->setColor(color.toString());
          QXmlStreamAttribute attrib;
          foreach(attrib, xmlStream.attributes())
          {
            if (attrib.name() == "name" || attrib.name() == "color")
              continue;
            node->addProperty(attrib.name().toString(), attrib.value().toString());
          }
//         }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  return tax;
}


TaxonomySPtr IOTaxonomy::openXMLTaxonomy(QString fileName)
{
  QFile file( fileName );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
//    QMessageBox::critical(this, "IOTaxonomy::openXMLTaxonomy",
//               "Couldn't open the file",
//               QMessageBox::Ok);
    qDebug() <<"File could not be oppended";
    return TaxonomySPtr();
  }

  // Read the XML
  QXmlStreamReader xmlStream(&file);
  /*
  QStringRef nodeName;
  TaxonomyNode* tax;
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        if( taxHierarchy.empty() )
        {
          tax = new TaxonomyNode( nodeName.toString() );
        }
        else
        {
          tax->addElement( nodeName.toString(), taxHierarchy.top() );
        }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  */
  TaxonomySPtr tax = readXML(xmlStream);
  file.close();
  return tax;
}

TaxonomySPtr IOTaxonomy::loadXMLTaxonomy(QString content)
{

//   if( content.device() )
//     xmlStream.setDevice( content.device() );
//   else if( content.string() )
//     xmlStream = QXmlStreamReader(*content.string());
    
  // Read the XML
  QXmlStreamReader xmlStream(content);
  /*
  QStringRef nodeName;
  TaxonomyNode* tax;
  std::stack<QString> taxHierarchy;
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        if( taxHierarchy.empty() )
        {
          tax = new TaxonomyNode( nodeName.toString() );
        }
        else
        {
          tax->addElement( nodeName.toString(), taxHierarchy.top() );
        }
        taxHierarchy.push( nodeName.toString() );
      }
      else if( xmlStream.isEndElement() )
      {
        taxHierarchy.pop();
      }
    }
  }
  return tax;
  */
  return readXML(xmlStream);
}

void IOTaxonomy::writeTaxonomy(TaxonomySPtr tax, QXmlStreamWriter& stream)
{
  if( !tax.isNull() )
    foreach(TaxonomyElementSPtr node, tax->elements())
      IOTaxonomy::writeTaxonomyElement(node, stream);
}

void IOTaxonomy::writeTaxonomyElement(TaxonomyElementSPtr node, QXmlStreamWriter& stream)
{
  // TODO 2012-12-15 Quitar name y color y usar solo las properties...
  if( node )
  {
    stream.writeStartElement( "node" );
    stream.writeAttribute("name", node->name());
    stream.writeAttribute("color", node->color().name());
    foreach(QString prop, node->properties())
    {
      stream.writeAttribute(prop, node->property(prop).toString());
    }
    foreach(TaxonomyElementSPtr subnode, node->subElements())
    {
      IOTaxonomy::writeTaxonomyElement(subnode, stream );
    }
    stream.writeEndElement();
  }
}

//-----------------------------------------------------------------------------
void IOTaxonomy::writeXMLTaxonomy(TaxonomySPtr tax, QString& destination)
{
  QXmlStreamWriter stream(&destination);

  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("Taxonomy");

  IOTaxonomy::writeTaxonomy(tax, stream);

  stream.writeEndElement();
  stream.writeEndDocument();
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::removeTemporalDir(QDir temporalDir)
{
  bool result = true;
  foreach(QFileInfo temporalFile, temporalDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot))
  {
    if (temporalFile.isDir())
    {
      result &= removeTemporalDir(QDir(temporalFile.absoluteFilePath()));
      continue;
    }

    result &= temporalDir.remove(temporalFile.fileName());
  }

  QString dirName = temporalDir.dirName();
  Q_ASSERT(!dirName.isEmpty());
  result &= temporalDir.cdUp();
  result &= temporalDir.rmdir(dirName);

  if (!result)
    return ERROR;

  return SUCCESS;
}
