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

const QString SETTINGS = "settings.ini";
const QString EspinaIO::VERSION = "version"; //backward compatibility
const QString SEG_FILE_VERSION  = "3";
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
EspinaIO::STATUS EspinaIO::loadFile(QFileInfo file,
                                    IEspinaModel *model,
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
                                       IEspinaModel *model,
                                       ChannelSPtr &channel,
                                       ErrorHandler *handler)
{
  SampleSPtr existingSample;

  EspinaFactory *factory = model->factory();

  if (existingSample.isNull())
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
      return FILE_NOT_FOUND;
  }

  readerArgs[ChannelReader::FILE] = file.absoluteFilePath();
  FilterSPtr reader(new ChannelReader(noInputs, readerArgs, ChannelReader::TYPE, handler));
  reader->update(Filter::ALL_INPUTS);
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
                                       IEspinaModel *model,
                                       ErrorHandler *handler)
{
  // generate random dir based on file name
  QDir temporalDir = QDir::tempPath();
  if (!temporalDir.exists("espina"))
    temporalDir.mkdir("espina");
  temporalDir.cd("espina");

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
    if (file.fileName() == SETTINGS)
    {
      STATUS status = readSettings(espinaFile, model, handler);
      if (status != SUCCESS)
        return status;
    }
    if (file.fileName() == VERSION)
    {
      QString versionNumber = espinaFile.readAll();
      if (versionNumber < SEG_FILE_COMPATIBLE_VERSION)
      {
        if (handler)
          handler->error(QObject::tr("Invalid seg file version. File Version=%1, current Version %2").arg(versionNumber).arg(SEG_FILE_VERSION));
        removeTemporalDir();
        espinaFile.close();
        return INVALID_VERSION;
      }
      model->setTraceable(false);
    }
    else if (file.fileName() == TAXONOMY_FILE)
    {
      Q_ASSERT(taxonomy.isNull());
      taxonomy = IOTaxonomy::loadXMLTaxonomy(espinaFile.readAll());
      //taxonomy->print(3);
    }
    else if (file.fileName() == TRACE_FILE)
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

      // Otherwise save it to the temporal directory
      if (!extensionFile)
      {
        QFileInfo destination = temporalDir.absoluteFilePath(file.filePath());
        if (!destination.absoluteDir().exists())
          temporalDir.mkpath(file.path());

        QFile destinationFile(destination.absoluteFilePath());
        /*qDebug()<< "Permissions set" <<
         *       destination.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
         *                                  QFile::ReadGroup | QFile::ReadOther);*/
        if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
          if (handler)
            handler->warning("IOEspinaFile::loadFile: could not create file " + destinationFile.fileName() + " in " + temporalDir.path());
        }

        destinationFile.write(espinaFile.readAll());
        destinationFile.close();
      }
    }

    espinaFile.close();
    hasFile = espinaZip.goToNextFile();
  }

  if(taxonomy.isNull() || traceContent.isEmpty())
  {
    if (handler)
      handler->error("IOEspinaFile::loadFile: could not load taxonomy and/or trace files");
    removeTemporalDir();
    return ERROR;
  }

  STATUS status;
  model->addTaxonomy(taxonomy);
  std::istringstream trace(traceContent.toStdString().c_str());
  status = loadSerialization(model, trace, temporalDir, handler) ? SUCCESS : ERROR;

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
  io->SetFileName(mhd.toUtf8());
  writer->SetFileName(mhd.toUtf8().data());
  output.update();
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
EspinaIO::STATUS EspinaIO::saveSegFile(QFileInfo file, IEspinaModel *model, ErrorHandler *handler)
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
  zipFile(SETTINGS, settings(model), outFile);

  // Save Taxonomy
  QString taxonomy;
  IOTaxonomy::writeXMLTaxonomy(model->taxonomy(), taxonomy);
  if( !zipFile(QString(TAXONOMY_FILE), taxonomy.toAscii(), outFile) )
    return ERROR;

  // Save Trace
  std::ostringstream trace;
  serializeRelations(model, trace);
  if( !zipFile(QString(TRACE_FILE),  trace.str().c_str(), outFile) )
    return ERROR;

  // Store filter data
  foreach(FilterSPtr filter, model->filters())
  {
    Snapshot snapshot;
    if (filter->dumpSnapshot(snapshot))
    {
      foreach(SnapshotEntry entry, snapshot)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
          return ERROR;
      }
    }
  }

  // Save Extensions' Information
  foreach(Channel::ExtensionPtr extension, model->factory()->channelExtensions())
  {
    Snapshot snapshot;
    if (extension->saveCache(snapshot))
    {
      foreach(SnapshotEntry entry, snapshot)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
          return ERROR;
      }
    }
  }

  foreach(Segmentation::InformationExtension extension, model->factory()->segmentationExtensions())
  {
    Snapshot snapshot;
    if (extension->saveCache(snapshot))
    {
      foreach(SnapshotEntry entry, snapshot)
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

void EspinaIO::serializeRelations(IEspinaModel *model, ostream &stream, RelationshipGraph::PrintFormat format)
{
  model->relationships()->updateVertexInformation();
  model->relationships()->write(stream, format);
}

//-----------------------------------------------------------------------------
QByteArray EspinaIO::settings(IEspinaModel *model)
{
  QByteArray settingsData;

  QTextStream settingsStream(&settingsData);

  settingsStream << SEG_FILE_VERSION << endl;
  settingsStream << QString("Traceable=") << model->isTraceable() << endl;

  return settingsData;
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

TaxonomySPtr IOTaxonomy::readXML(QXmlStreamReader& xmlStream)
{
  // Read the XML
//   QXmlStreamReader xmlStream(&file);
  QStringRef nodeName, color;
  TaxonomySPtr taxonomy(new Taxonomy());
  TaxonomyElementPtr parent = taxonomy->root().data();
  while(!xmlStream.atEnd())
  {
    xmlStream.readNextStartElement();
    if( xmlStream.name() == "node")
    {
      if( xmlStream.isStartElement() )
      {
        nodeName = xmlStream.attributes().value("name");
        color = xmlStream.attributes().value("color");
        TaxonomyElementSPtr node   = taxonomy->createElement( nodeName.toString(), parent);
        node->setColor(color.toString());
        QXmlStreamAttribute attrib;
        foreach(attrib, xmlStream.attributes())
        {
          if (attrib.name() == "name" || attrib.name() == "color")
            continue;
          node->addProperty(attrib.name().toString(), attrib.value().toString());
        }

        // BUGFIX: Some taxonomies didn't contain some properties
        if (!node->properties().contains("Dim_X") || !node->properties().contains("Dim_Y") || !node->properties().contains("Dim_Z"))
        {
          qWarning() << "Taxonomy" << node->name() << "is missing some properties.";
          TaxonomySPtr defaultTaxonomy = IOTaxonomy::openXMLTaxonomy(":/espina/defaultTaxonomy.xml");
          TaxonomyElementSPtr defaultNode = defaultTaxonomy->element(node->qualifiedName());
          if (!defaultNode.isNull())
          {
            foreach(QString property, defaultNode->properties())
            {
              if (!node->properties().contains(property))
              {
                qWarning() << "adding missing property" << property << "with value" << defaultNode->property(property).toString();
                node->addProperty(property, defaultNode->property(property));
              }
            }
          }
          else
            qWarning() << "Taxonomy" << node->qualifiedName() << "doesn't exist in the default taxonomy definition";
        }

        parent = node.data();
      }
      else if( xmlStream.isEndElement() )
      {
        parent = parent->parent();
      }
    }
  }

  return taxonomy;
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

  TaxonomySPtr tax = readXML(xmlStream);

  file.close();

  return tax;
}

TaxonomySPtr IOTaxonomy::loadXMLTaxonomy(QString content)
{
  // Read the XML
  QXmlStreamReader xmlStream(content);

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
  if (temporalDir == QDir())
  {
    temporalDir = QDir::temp();
    temporalDir.cd("espina");
  }

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

//-----------------------------------------------------------------------------
bool EspinaIO::loadSerialization(IEspinaModel *model,
                                 istream &stream,
                                 QDir tmpDir,
                                 EspinaIO::ErrorHandler *handler,
                                 RelationshipGraph::PrintFormat format)
{
  QSharedPointer<RelationshipGraph> input(new RelationshipGraph());

  input->read(stream);
//   qDebug() << "Check";
//   input->write(std::cout, RelationshipGraph::GRAPHVIZ);

  typedef QPair<ModelItemSPtr , ModelItem::Arguments> NonInitilizedItem;
  QList<NonInitilizedItem> nonInitializedItems;
  QList<VertexProperty> segmentationNodes;
  SegmentationSList newSegmentations;

  EspinaFactory *factory = model->factory();

  foreach(VertexProperty v, input->vertices())
  {
    VertexProperty fv;
    if (model->relationships()->find(v, fv))
    {
      input->setItem(v.vId, fv.item);
      qDebug() << "Updating existing vertex" << fv.item->data(Qt::DisplayRole).toString();
    }else
    {
      switch (RelationshipGraph::type(v))
      {
        case SAMPLE:
        {
          ModelItem::Arguments args(v.args.c_str());
          SampleSPtr sample = factory->createSample(v.name.c_str(), v.args.c_str());
          model->addSample(sample);
          nonInitializedItems << NonInitilizedItem(sample, args);
          input->setItem(v.vId, sample.data());
          break;
        }
        case CHANNEL:
        {
          ModelItem::Arguments args(v.args.c_str());
          ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
          args.remove(ModelItem::EXTENSIONS);

          // TODO: Move link management code inside Channel's Arguments class
          QStringList link = args[Channel::VOLUME].split("_");
          Q_ASSERT(link.size() == 2);
          Vertices ancestors = input->ancestors(v.vId, link[0]);
          Q_ASSERT(ancestors.size() == 1);
          ModelItemPtr item = ancestors.first().item;
          Q_ASSERT(FILTER == item->type());
          FilterSPtr filter = model->findFilter(item);
          Q_ASSERT(!filter.isNull());
          try
          {
            Filter::OutputId channelId = link[1].toInt();
            filter->update(channelId);
            ChannelSPtr channel = factory->createChannel(filter, channelId);
            channel->initialize(args);
            if (channel->volume()->toITK().IsNull())
              return false;
            model->addChannel(channel);
            nonInitializedItems << NonInitilizedItem(channel, extArgs);
            input->setItem(v.vId, channel.data());
          }
          catch(int e)
          {
            Vertices successors = input->succesors(v.vId, QString());
            if (!successors.empty())
              return false;

            input->removeEdges(v.vId);
          }
          break;

        }
        case FILTER:
        {
          Filter::NamedInputs inputs;
          Filter::Arguments args(v.args.c_str());
          QStringList inputLinks = args[Filter::INPUTS].split(",", QString::SkipEmptyParts);
          // We need to update id values for future filters
          foreach(QString inputLink, inputLinks)
          {
            QStringList link = inputLink.split("_");
            Q_ASSERT(link.size() == 2);
            Vertices ancestors = input->ancestors(v.vId, link[0]);
            Q_ASSERT(ancestors.size() == 1);
            ModelItemPtr item = ancestors.first().item;
            Q_ASSERT(FILTER == item->type());
            FilterSPtr filter = model->findFilter(item);
            inputs[link[0]] = filter;
          }
          FilterSPtr filter = factory->createFilter(v.name.c_str(), inputs, args);
          filter->setCacheDir(tmpDir);
          model->addFilter(filter);
          input->setItem(v.vId, filter.data());
          break;
        }
        case SEGMENTATION:
        {
          segmentationNodes << v;
          break;
        }
        default:
          Q_ASSERT(false);
          break;
      }
    }
  }

  foreach(VertexProperty v, segmentationNodes)
  {
    Vertices ancestors = input->ancestors(v.vId, Filter::CREATELINK);
    Q_ASSERT(ancestors.size() == 1);

    ModelItem::Arguments args(QString(v.args.c_str()));
    Filter::OutputId outputId =  args[Segmentation::OUTPUT].toInt();

    ModelItemPtr item = ancestors.first().item;
    FilterSPtr filter = model->findFilter(item);
    filter->update(outputId);
    // NOTE: add return value to update?
    if (filter->outputs().isEmpty())
      return false;

    ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
    args.remove(ModelItem::EXTENSIONS);
    SegmentationSPtr seg = factory->createSegmentation(filter, outputId);
    seg->setNumber(args[Segmentation::NUMBER].toInt());
    TaxonomyElementSPtr taxonomy = model->taxonomy()->element(args[Segmentation::TAXONOMY]);
    if (!taxonomy.isNull())
      seg->setTaxonomy(taxonomy);
    newSegmentations << seg;
    nonInitializedItems << NonInitilizedItem(seg, extArgs);
    input->setItem(v.vId, seg.data());
  }

  model->addSegmentation(newSegmentations);

  foreach(Edge e, input->edges())
  { //Should store just the modelitem?
    Q_ASSERT(e.source.item);
    Q_ASSERT(e.target.item);

    ModelItemSPtr source = model->find(e.source.item);
    ModelItemSPtr target = model->find(e.target.item);

    model->addRelation(source, target, e.relationship.c_str());
  }

  foreach(FilterSPtr filter, model->filters())
    filter->upkeeping();

  return true;
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::readSettings(QuaZipFile   &file,
                                        IEspinaModel *model,
                                        ErrorHandler *handler)
{
  STATUS status = SUCCESS;

  QTextStream settingsStream(file.readAll());

  QString versionNumber = settingsStream.readLine();

  if (versionNumber < SEG_FILE_COMPATIBLE_VERSION)
  {
    if (handler)
      handler->error(QObject::tr("Invalid seg file version. File Version=%1, current Version %2").arg(versionNumber).arg(SEG_FILE_VERSION));
    removeTemporalDir();
    file.close();
    status = INVALID_VERSION;
  } else
  {
    QString line = settingsStream.readLine();
    bool traceable = line.split("=")[1].toInt();
    model->setTraceable(traceable);
  }

  return status;
}
