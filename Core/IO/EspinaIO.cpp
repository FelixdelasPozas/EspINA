#include "EspinaIO.h"


// EspINA
#include "Core/Model/EspinaFactory.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/Taxonomy.h>

#include "Filters/ChannelReader.h"
#include "Undo/AddChannel.h"
#include "Undo/AddRelation.h"
#include "Undo/AddSample.h"

// ITK
#include <itkImageFileWriter.h>
#include <itkMetaImageIO.h>

// VTK
#include <vtkMetaImageReader.h>
#include <vtkTIFFReader.h>

// Qt
#include <QDebug>
#include <QDir>
#include <QUndoStack>
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
EspinaIO::STATUS EspinaIO::loadFile(QFileInfo file,
                                    EspinaModel *model,
                                    QUndoStack  *undoStack,
                                    QDir tmpDir)
{
  const QString ext = file.suffix();
  if ("mha" == ext || "mhd" == ext || "tiff" == ext || "tif" == ext)
  {
    ChannelSPtr channel;
    return loadChannel(file, model, undoStack, channel);
  }

  if ("seg" == ext)
    return loadSegFile(file, model, tmpDir);

  return model->factory()->readFile(file.absoluteFilePath(), ext)?SUCCESS:ERROR;
}

//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::loadChannel(QFileInfo file,
                                       EspinaModel *model,
                                       QUndoStack  *undoStack,
                                       ChannelSPtr &channelPtr)
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

    undoStack->push(new AddSample(existingSample, model));
  }

  //TODO: Check for channel information in DB
  QColor stainColor = QColor(Qt::black);

  Filter::NamedInputs noInputs;
  Filter::Arguments readerArgs;
  readerArgs[ChannelReader::FILE] = file.absoluteFilePath();
  FilterSPtr reader(new ChannelReader(noInputs, readerArgs, ChannelReader::TYPE));
  reader->update();
  if (reader->outputs().isEmpty())
    return ERROR;


  Channel::CArguments args;
  args[Channel::ID] = file.fileName();
  args.setHue(stainColor.hueF());
  ChannelSPtr channel = factory->createChannel(reader, 0);
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

  undoStack->beginMacro("Add Data To Analysis");
  undoStack->push(new AddChannel(reader, channel, model));
  undoStack->push(new AddRelation(existingSample, channel, Channel::STAINLINK, model));
  undoStack->endMacro();

  //existingSample->initialize();//TODO: Review if needed
  channel->initialize(args);
  channel->initializeExtensions();

  channelPtr = channel;//NOTE: Comprobar esto

  return SUCCESS;
}


//-----------------------------------------------------------------------------
EspinaIO::STATUS EspinaIO::loadSegFile(QFileInfo file,
                                       EspinaModel *model,
                                       QDir tmpDir)
{
  // Create tmp dir if necessary
  if (!tmpDir.exists())
    QDir::root().mkpath(tmpDir.absolutePath());

  QString tmpSegDir = QString::number(rand());
  tmpDir.mkdir(tmpSegDir);
  tmpDir.cd(tmpSegDir);

  QuaZip espinaZip(file.filePath());
  if( !espinaZip.open(QuaZip::mdUnzip) )
  {
    qWarning() << "IOEspinaFile: Could not open file" << file.filePath();
    return FILE_NOT_FOUND;
  }

  QuaZipFile espinaFile(&espinaZip);

  TaxonomySPtr taxonomy;
  QString traceContent;

  bool hasFile = espinaZip.goToFirstFile();
  while(hasFile)
  {
    QFileInfo file = espinaFile.getActualFileName();
    if( !espinaFile.open(QIODevice::ReadOnly) )
    {
      qWarning() << "IOEspinaFile: Could not extract the file" << file.filePath();
      if( file == TAXONOMY_FILE || file == TRACE_FILE )
        return ERROR;
      continue;
    }
    //qDebug() << "EspinaIO::loadSegFile: extracting" << file.filePath();
    if (file.fileName() == VERSION)
    {
      QString versionNumber = espinaFile.readAll();
      if (versionNumber < SEG_FILE_VERSION)
      {
        qWarning() << QObject::tr("Importing Seg File from older version. "
                                  "File Version=%1, Current Version %2")
                                  .arg(versionNumber).arg(SEG_FILE_VERSION);
        return INVALID_VERSION;
      }
    }
    else if(file.fileName() == TAXONOMY_FILE)
    {
      Q_ASSERT(taxonomy.isNull());
      taxonomy = IOTaxonomy::loadXMLTaxonomy(espinaFile.readAll());
      //taxonomy->print(3);
    } else if(file.fileName() == TRACE_FILE)
    {
      Q_ASSERT(traceContent.isEmpty());
      QTextStream traceStream(&traceContent);
      traceStream << espinaFile.readAll();
    } else
    {
      // Espina Volumes
      QFile destination(tmpDir.filePath(file.fileName()));
      /*qDebug()<< "Permissions set" <<
       *       destination.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
       *                                  QFile::ReadGroup | QFile::ReadOther);*/
      if(!destination.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        qWarning() << "IOEspinaFile::loadFile: could not create file " << file.filePath();
      }
      destination.write(espinaFile.readAll());
      destination.close();
    }
    espinaFile.close();
    hasFile = espinaZip.goToNextFile();
  }

  if(taxonomy.isNull() || traceContent.isEmpty())
  {
    qWarning() << "IOEspinaFile::loadFile: could not load taxonomy and/or trace files";
    return ERROR;
  }

  STATUS status;
  model->addTaxonomy(taxonomy);
  std::istringstream trace(traceContent.toStdString().c_str());
  status = model->loadSerialization(trace, tmpDir)?SUCCESS:ERROR;

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
  QString volumeName = QString("%1_%2").arg(filter->tmpId()).arg(output.id);
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
EspinaIO::STATUS EspinaIO::saveSegFile(QFileInfo file, EspinaModel *model)
{
  // Create tmp dir
//   qDebug() << file.absolutePath();
  QDir tmpDir = file.absoluteDir();
  if (!file.baseName().isEmpty())
  {
    tmpDir.mkdir(file.baseName());
    tmpDir.cd(file.baseName());
  }
//   qDebug() << "Temporal Dir" << tmpDir;

  QFile zFile(file.filePath());
  QuaZip zip(&zFile);
  if(!zip.open(QuaZip::mdCreate)) 
  {
    qWarning() << "IOEspinaFile::saveFile" << file.fileName() << "error while creating file";
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

  foreach(FilterSPtr filter, model->filters())
  {
    Filter::OutputList outputs = filter->outputs();
    //qDebug() << "Making" << filter->data().toString() << "snapshot";
    foreach(Filter::Output output, outputs)
    {
      if (output.isCached)
      {
        filter->update(); // TODO 2012-11-20 Recuperar los .mhd editados sin tener q cargarlos del filtro...
        //NOTE: In case the filter is updated the output is not (it's just a copy of the old one)
        zipVolume(filter->output(output.id), tmpDir, outFile);
      }
    }
  }

  QStringList filters;
  filters << "*.mhd" << "*.raw";
  foreach(QFileInfo tmpFile, tmpDir.entryInfoList(filters))
  {
    //qDebug() << "Removing" << tmpFile.fileName();
    Q_ASSERT(tmpDir.remove(tmpFile.fileName()));
  }

  if (!file.baseName().isEmpty())
  {
    tmpDir.cdUp();
    tmpDir.rmdir(file.baseName());
  }

  return SUCCESS;
}

//-----------------------------------------------------------------------------
bool EspinaIO::zipFile(QString fileName, QByteArray content, QuaZipFile& zFile)
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
