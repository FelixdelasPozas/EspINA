/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "SegFile_V5.h"

#include <EspinaConfig.h>

#include "SegFile.h"
#include "ClassificationXML.h"
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Classification.h>
#include <Core/Analysis/Extensions/ExtensionProvider.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Storage.h>

using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;

const QString SegFile::SegFile_V5::FORMAT_INFO_FILE = "formatInfo.ini";

const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";
const QString CLASSIFICATION_FILE = "classification.xml";
const QString SEG_FILE_VERSION    = "5";

//-----------------------------------------------------------------------------
QByteArray formatInfo()
{
  QByteArray info;

  QTextStream infoStream(&info);

  infoStream << QString("SegFile Version=%1").arg(SEG_FILE_VERSION) << endl;
  infoStream << QString("EspINA Version=%1").arg(ESPINA_VERSION) << endl;

  return info;
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::load(QuaZip& zip, ErrorHandlerPtr handler)
{
  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  Persistent::StorageSPtr storage{new Persistent::Storage(tmpDir)};

  AnalysisSPtr analysis{new Analysis()};

  if (!zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (handler)
      handler->error(QObject::tr("Could not load analysis classification"));

    throw (Classification_Not_Found_Exception());
  }

  try
  {
    analysis->setClassification(ClassificationXML::parse(readCurrentFileFromZip(zip, handler)));
  } catch (ClassificationXML::Parse_Exception e) 
  {
    if (handler)
      handler->error(QObject::tr("Error while loading classification"));

    throw (Parse_Exception());
  }
  
  std::cerr << print(analysis->classification()).toStdString();

  bool hasFile = zip.goToFirstFile();
  while (hasFile)
  {
    QFileInfo file = zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE
     && file != CLASSIFICATION_FILE
     && file != CONTENT_FILE
     && file != RELATIONS_FILE)
    {
      storage->saveSnapshot(SnapshotData(file.fileName(), readCurrentFileFromZip(zip, handler)));
    }

    hasFile = zip.goToNextFile();
  }

  loadContent(analysis, zip, storage, handler);
  loadRelations(analysis, zip, handler);
}

//-----------------------------------------------------------------------------
void SegFile_V5::save(AnalysisPtr analysis, QuaZip& zip, ErrorHandlerPtr handler)
{
  try {
    addFileToZip(FORMAT_INFO_FILE, formatInfo(), zip, handler);
  } catch (IO_Error_Exception e) 
  {
    if (handler)
      handler->error("Error while saving Analysis Format Information");

    throw (e);
  }

  QByteArray classification;
  try {
    classification = ClassificationXML::dump(analysis->classification(), handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Classification");

    throw(e);
  }

  try {
    addFileToZip(CLASSIFICATION_FILE, classification, zip, handler);
  }
  catch (IO_Error_Exception e){
    if (handler)
      handler->error("Error while saving Analysis Classification");

    throw(e);
  }

  std::ostringstream content;
  write(analysis->content(), content);
  try {
    addFileToZip(CONTENT_FILE, content.str().c_str(), zip, handler);
  } catch (IO_Error_Exception e) 
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    throw (e);
  }

  std::ostringstream relations;
  write(analysis->relationships(), relations);
  try {
    addFileToZip(RELATIONS_FILE, relations.str().c_str(), zip, handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    throw (e);
  }

  foreach(DirectedGraph::Vertex v, analysis->content()->vertices()) {
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.item.get());
    foreach(SnapshotData data, item->saveSnapshot())
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      } catch (IO_Error_Exception e)
      {
        throw (e);
      }
    }
  }

  foreach(ExtensionProviderSPtr provider, analysis->extensionProviders()) {
    foreach(SnapshotData data, provider->saveSnapshot())
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      } catch (IO_Error_Exception e)
      {
        throw (e);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadContent(AnalysisSPtr            analysis,
                             QuaZip&                 zip,
                             Persistent::StorageSPtr storage,
                             ErrorHandlerPtr         handler)
{
  DirectedGraphSPtr content(new DirectedGraph());

  QTextStream textStream(readFileFromZip(CONTENT_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, content);

//   qDebug() << "Check";
  write(content, std::cout);
// 
//   typedef QPair<ModelItemSPtr, ModelItem::Arguments> NonInitilizedItem;
// 
//   SegmentationSList newSegmentations;
//   RelationshipGraph::Vertices segmentationNodes;
// 
//   EspinaFactory *factory = model->factory();
// 
//   foreach(RelationshipGraph::Vertex v, input->vertices())
//   {
//     // Input may be modified inside parse vertex, so we need to update
//     // vertex structure
//     RelationshipGraph::Vertex vertex = input->vertex(v.descriptor);
//     if (!parseVertex(input, vertex, model, tmpDir, segmentationNodes))
//     return false;
//   }
// 
//   foreach(RelationshipGraph::Vertex v, segmentationNodes)
//   {
//     RelationshipGraph::Vertices ancestors = input->ancestors(v, Filter::CREATELINK);
//     Q_ASSERT(ancestors.size() == 1);
// 
//     ModelItem::Arguments args (QString(v.args.c_str()));
//     FilterOutputId outputId = args[Segmentation::OUTPUT].toInt();
// 
//     ModelItemPtr item = ancestors.first().item;
//     FilterSPtr filter = model->findFilter(item);
// 
//     filter->update(outputId);
//     if (filter->validOutput(outputId))
//     {
//       ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
//       args.remove(ModelItem::EXTENSIONS);
//       SegmentationSPtr seg = factory->createSegmentation(filter, outputId);
//       seg->initialize(args);
//       seg->setNumber(args[Segmentation::NUMBER].toInt());
// 
//       QString taxonomyQualifiedName = args[Segmentation::TAXONOMY];
//       TaxonomyElementSPtr taxonomy = model->taxonomy()->element(taxonomyQualifiedName);
//       if (taxonomy.get() != NULL)
//       seg->setTaxonomy(taxonomy);
//       else
//       {
//         taxonomy = model->taxonomy()->createElement(taxonomyQualifiedName);
//         seg->setTaxonomy(taxonomy);
//       }
//       newSegmentations << seg;
//       input->setItem(v, seg.get());
//     }
//   }
// 
//   model->addSegmentation(newSegmentations);
// 
//   foreach(RelationshipGraph::Edge e, input->edges())
//   {
//     // Should store just the modelitem?
//     Q_ASSERT(e.source.item);
//     Q_ASSERT(e.target.item);
// 
//     ModelItemSPtr source = model->find(e.source.item);
//     ModelItemSPtr target = model->find(e.target.item);
// 
//     model->addRelation(source, target, e.relationship.c_str());
//   }
// 
//   foreach(FilterSPtr filter, model->filters())
//     filter->upkeeping();
// 
//   // the user could have canceled the load of channels with no
//   // segmentations, so there could be empty samples.
//   foreach(SampleSPtr sample, model->samples())
//     if (sample->channels().isEmpty())
//       model->removeSample(sample);
// 
//   return true;
}


//-----------------------------------------------------------------------------
void SegFile_V5::loadRelations(AnalysisSPtr    analysis,
                               QuaZip&         zip,
                               ErrorHandlerPtr handler)
{

}


