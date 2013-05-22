#include "SegFileReader.h"

#include <QFile>

#include <EspinaConfig.h>

// EspINA
#include "IOErrorHandler.h"

#include "Core/Model/EspinaModel.h"
#include "Core/Model/Output.h"
#include "Core/Model/EspinaFactory.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/Taxonomy.h>
#include <Core/Extensions/ChannelExtension.h>
#include <Core/Extensions/SegmentationExtension.h>

#include "Core/Filters/ChannelReader.h"

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
const QString SegFileReader::FILE_VERSION = "version"; //backward compatibility
const QString SEG_FILE_VERSION  = "4";
const QString SEG_FILE_COMPATIBLE_VERSION  = "1";

//-----------------------------------------------------------------------------
IOErrorHandler::STATUS SegFileReader::loadSegFile(QFileInfo       file,
                                                  IEspinaModel   *model,
                                                  IOErrorHandler *handler)
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
    return IOErrorHandler::FILE_NOT_FOUND;
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
        return IOErrorHandler::ERROR;

      continue;
    }
    // qDebug() << "SegFileReader::loadSegFile: extracting" << file.filePath();
    if (file.fileName() == SETTINGS)
    {
      IOErrorHandler::STATUS status = readSettings(espinaFile, model, handler);
      if (status != IOErrorHandler::SUCCESS)
        return status;
    }
    if (file.fileName() == FILE_VERSION)
    {
      QString versionNumber = espinaFile.readAll();
      if (versionNumber < SEG_FILE_COMPATIBLE_VERSION)
      {
        if (handler)
          handler->error(QObject::tr("Invalid seg file version. File Version=%1, current Version %2").arg(versionNumber).arg(SEG_FILE_VERSION));
        removeTemporalDir();
        espinaFile.close();
        return IOErrorHandler::INVALID_VERSION;
      }
      model->setTraceable(false);
    }
    else if (file.fileName() == TAXONOMY_FILE)
    {
      Q_ASSERT(taxonomy.get() == NULL);
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

  if(taxonomy.get() == NULL || traceContent.isEmpty())
  {
    if (handler)
      handler->error("IOEspinaFile::loadFile: could not load taxonomy and/or trace files");
    removeTemporalDir();
    return IOErrorHandler::ERROR;
  }

  IOErrorHandler::STATUS status;
  model->addTaxonomy(taxonomy);
  std::istringstream trace(traceContent.toStdString().c_str());
  status = loadSerialization(model, trace, temporalDir, handler) ? IOErrorHandler::SUCCESS : IOErrorHandler::ERROR;

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
IOErrorHandler::STATUS SegFileReader::saveSegFile(QFileInfo file, IEspinaModel *model, IOErrorHandler *handler)
{
  if (file.baseName().isEmpty())
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile file name is empty");
    return IOErrorHandler::ERROR;
  }

  // use system/user temporal directory, OS dependent
  QDir temporalDir = QDir::tempPath();
  QString path = QString("espina%1io%1%2").arg(QDir::separator()).arg(file.baseName());
  temporalDir.mkpath(path);
  temporalDir.cd(path);

  QString temporalSegFileName = temporalDir.path() + QDir::separator() + file.fileName();

  QFile zFile(temporalSegFileName);
  QuaZip zip(&zFile);
  if(!zip.open(QuaZip::mdCreate)) 
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile" + zFile.fileName() + "error while creating file");

    removeTemporalDir(temporalDir);
    return IOErrorHandler::ERROR;
  }
  QuaZipFile outFile(&zip);

  // Store Version Number
  zipFile(SETTINGS, settings(model), outFile);

  // Save Taxonomy
  QString taxonomy;
  IOTaxonomy::writeXMLTaxonomy(model->taxonomy(), taxonomy);
  if( !zipFile(QString(TAXONOMY_FILE), taxonomy.toAscii(), outFile) )
  {
    removeTemporalDir(temporalDir);
    return IOErrorHandler::ERROR;
  }

  // Save Trace
  std::ostringstream trace;
  serializeRelations(model, trace);
  if( !zipFile(QString(TRACE_FILE),  trace.str().c_str(), outFile) )
  {
    removeTemporalDir(temporalDir);
    return IOErrorHandler::ERROR;
  }

  // Store filter data
  foreach(FilterSPtr filter, model->filters())
  {
    Snapshot snapshot;
    if (filter->dumpSnapshot(snapshot))
    {
      foreach(SnapshotEntry entry, snapshot)
      {
        if( !zipFile(entry.first, entry.second, outFile) )
        {
          removeTemporalDir(temporalDir);
          return IOErrorHandler::ERROR;
        }
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
        {
          removeTemporalDir(temporalDir);
          return IOErrorHandler::ERROR;
        }
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
        {
          removeTemporalDir(temporalDir);
          return IOErrorHandler::ERROR;
        }
      }
    }
  }

  zip.close();

  if (zip.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error("IOEspinaFile::saveFile IOErrorHandler::ERROR: Failed to close" + zFile.fileName() + "zip file");

    removeTemporalDir(temporalDir);
    return IOErrorHandler::ERROR;
  }

  // remove old file if it exists or rename won't work
  if (temporalDir.exists(file.absoluteFilePath()))
  {
    if (!temporalDir.remove(file.absoluteFilePath()))
    {
      qDebug() << "IOEspinaFile::saveFile IOErrorHandler::ERROR: remove" << temporalSegFileName << "file";
      removeTemporalDir(temporalDir);
      return IOErrorHandler::ERROR;
    }
  }

  // rename(oldFile, newFile) only fails if:
  // - oldName doesn't exist
  // - newName already exist
  // - rename crosses filesystem boundaries
  if (!temporalDir.rename(temporalSegFileName, file.absoluteFilePath()))
  {
    qDebug() << "IOEspinaFile::saveFile IOErrorHandler::ERROR: Failed to rename" << temporalSegFileName << "to" << file.absoluteFilePath();
    removeTemporalDir(temporalDir);
    return IOErrorHandler::ERROR;
  }

  removeTemporalDir(temporalDir);

  return IOErrorHandler::SUCCESS;
}

void SegFileReader::serializeRelations(IEspinaModel *model, ostream &stream, RelationshipGraph::PrintFormat format)
{
  model->relationships()->updateVertexInformation();
  model->relationships()->write(stream, format);
}

//-----------------------------------------------------------------------------
QByteArray SegFileReader::settings(IEspinaModel *model)
{
  QByteArray settingsData;

  QTextStream settingsStream(&settingsData);

  settingsStream << SEG_FILE_VERSION << endl;
  settingsStream << QString("Traceable=") << model->isTraceable()    << endl;
  settingsStream << QString("EspINA Version=%1").arg(ESPINA_VERSION) << endl;

  return settingsData;
}

//-----------------------------------------------------------------------------
bool SegFileReader::zipFile(QString fileName, const QByteArray &content, QuaZipFile& zFile)
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
  TaxonomyElementPtr parent = taxonomy->root().get();
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
          if (defaultNode.get() != NULL)
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

        parent = node.get();
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
  if(tax.get() != NULL)
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
IOErrorHandler::STATUS SegFileReader::removeTemporalDir(QDir temporalDir)
{
  if (temporalDir == QDir())
  {
    temporalDir = QDir::temp();
    if (temporalDir.exists("espina"))
      temporalDir.cd("espina");
    else
      return IOErrorHandler::SUCCESS;
  }

  bool result = true;
  foreach(QFileInfo temporalFile, temporalDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot))
  {
    if (temporalFile.isDir())
      result &= (removeTemporalDir(QDir(temporalFile.absoluteFilePath())) == IOErrorHandler::SUCCESS);
    else
      result &= temporalDir.remove(temporalFile.fileName());
  }

  QString dirName = temporalDir.dirName();
  Q_ASSERT(!dirName.isEmpty());
  result &= temporalDir.cdUp();
  result &= temporalDir.rmdir(dirName);

  if (!result)
  {
    qWarning() << "SegFileReader: Failed to remove temporal directory" << dirName;
    return IOErrorHandler::ERROR;
  }

  return IOErrorHandler::SUCCESS;
}

//-----------------------------------------------------------------------------
bool SegFileReader::loadSerialization(IEspinaModel *model,
                                 istream &stream,
                                 QDir tmpDir,
                                 IOErrorHandler *handler,
                                 RelationshipGraph::PrintFormat format)
{
  boost::shared_ptr<RelationshipGraph> input(new RelationshipGraph());

  input->read(stream);
//   qDebug() << "Check";
//   input->write(std::cout, RelationshipGraph::GRAPHVIZ);

  typedef QPair<ModelItemSPtr , ModelItem::Arguments> NonInitilizedItem;

  SegmentationSList           newSegmentations;
  RelationshipGraph::Vertices segmentationNodes;

  EspinaFactory *factory = model->factory();

  foreach(RelationshipGraph::Vertex v, input->vertices())
  {
    switch (RelationshipGraph::type(v))
    {
      case SAMPLE:
      {
        ModelItem::Arguments args(v.args.c_str());
        SampleSPtr sample = factory->createSample(v.name.c_str(), v.args.c_str());
        model->addSample(sample);
        input->setItem(v, sample.get());
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
        RelationshipGraph::Vertices ancestors = input->ancestors(v, link[0]);
        Q_ASSERT(ancestors.size() == 1);
        ModelItemPtr item = ancestors.first().item;
        Q_ASSERT(FILTER == item->type());
        FilterSPtr filter = model->findFilter(item);
        Q_ASSERT(filter.get() != NULL);
        try
        {
          FilterOutputId channelId = link[1].toInt();
          filter->update(channelId);
          ChannelSPtr channel = factory->createChannel(filter, channelId);
          channel->initialize(args);
          if (channel->volume()->toITK().IsNull())
            return false;
          model->addChannel(channel);
          input->setItem(v, channel.get());
        }
        catch(int e)
        {
          RelationshipGraph::Vertices successors = input->succesors(v, QString());
          if (!successors.empty())
            return false;

          input->removeEdges(v);
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
          RelationshipGraph::Vertices ancestors = input->ancestors(v, link[0]);
          Q_ASSERT(ancestors.size() == 1);
          ModelItemPtr item = ancestors.first().item;
          Q_ASSERT(FILTER == item->type());
          FilterSPtr filter = model->findFilter(item);
          inputs[link[0]] = filter;
        }
        FilterSPtr filter = factory->createFilter(v.name.c_str(), inputs, args);
        filter->setCacheDir(tmpDir);
        model->addFilter(filter);
        input->setItem(v, filter.get());
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

  foreach(RelationshipGraph::Vertex v, segmentationNodes)
  {
    RelationshipGraph::Vertices ancestors = input->ancestors(v, Filter::CREATELINK);
    Q_ASSERT(ancestors.size() == 1);

    ModelItem::Arguments args       (QString(v.args.c_str()));
    FilterOutputId       outputId = args[Segmentation::OUTPUT].toInt();

    ModelItemPtr item   = ancestors.first().item;
    FilterSPtr   filter = model->findFilter(item);

    filter->update(outputId);
    if (filter->validOutput(outputId))
    {
      ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
      args.remove(ModelItem::EXTENSIONS);
      SegmentationSPtr seg = factory->createSegmentation(filter, outputId);
      seg->initialize(args);
      seg->setNumber(args[Segmentation::NUMBER].toInt());

      QString taxonomyQualifiedName = args[Segmentation::TAXONOMY];
      TaxonomyElementSPtr taxonomy = model->taxonomy()->element(taxonomyQualifiedName);
      if (taxonomy.get() != NULL)
        seg->setTaxonomy(taxonomy);
      else
      {
        taxonomy = model->taxonomy()->createElement(taxonomyQualifiedName);
        seg->setTaxonomy(taxonomy);
      }
      newSegmentations << seg;
      input->setItem(v, seg.get());
    }
  }

  model->addSegmentation(newSegmentations);

  foreach(RelationshipGraph::Edge e, input->edges())
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
IOErrorHandler::STATUS SegFileReader::readSettings(QuaZipFile   &file,
                                                   IEspinaModel *model,
                                                   IOErrorHandler *handler)
{
  IOErrorHandler::STATUS status = IOErrorHandler::SUCCESS;

  QTextStream settingsStream(file.readAll());

  QString versionNumber = settingsStream.readLine();

  if (versionNumber < SEG_FILE_COMPATIBLE_VERSION)
  {
    if (handler)
      handler->error(QObject::tr("Invalid seg file version. File Version=%1, current Version %2").arg(versionNumber).arg(SEG_FILE_VERSION));
    removeTemporalDir();
    file.close();
    status = IOErrorHandler::INVALID_VERSION;
  } else
  {
    QString line = settingsStream.readLine();
    bool traceable = line.split("=")[1].toInt();
    model->setTraceable(traceable);
  }

  return status;
}
