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

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Classification.h>
#include <Core/Analysis/Extensions/ExtensionProvider.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Storage.h>
#include <Core/Factory/CoreFactory.h>


using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;
using namespace EspINA::IO::Graph;

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
AnalysisSPtr SegFile_V5::load(QuaZip&         zip,
                              CoreFactorySPtr factory,
                              ErrorHandlerPtr handler)
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

  loadContent(analysis, zip, storage, factory, handler);
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
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.get());
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
                             CoreFactorySPtr         factory,
                             ErrorHandlerPtr         handler)
{
  DirectedGraphSPtr content(new DirectedGraph());

  QTextStream textStream(readFileFromZip(CONTENT_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, content);

  foreach(DirectedGraph::Vertex roVertex, content->vertices())
  {
    PersistentSPtr vertex = parseVertex(analysis, content, roVertex, factory, handler);
  }

//   qDebug() << "Check";
//  write(content, std::cout);
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
PersistentSPtr SegFile_V5::parseVertex(AnalysisSPtr          analysis,
                                       DirectedGraphSPtr     content,
                                       DirectedGraph::Vertex roVertex,
                                       CoreFactorySPtr       factory,
                                       ErrorHandlerPtr       handler)
{
  ReadOnlyVertex *rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());

  PersistentSPtr vertex;
  switch(rov->type())
  {
    case VertexType::SAMPLE:
    {
      SampleSPtr sample = factory->createSample();
      sample->restoreState(roVertex->saveState());
      //sample->setPersistentStorage(storage);
      analysis->add(sample);
      break;
    }
    case VertexType::CHANNEL:
    {
//       ModelItem::Arguments args(rov.args.c_str());
//       ModelItem::Arguments extArgs(args.value(ModelItem::EXTENSIONS, QString()));
//       args.remove(ModelItem::EXTENSIONS);
//       
//       // TODO: Move link management code inside Channel's Arguments class
//       QStringList link = args[Channel::VOLUME].split("_");
//       Q_ASSERT(link.size() == 2);
//       RelationshipGraph::Vertices ancestors = input->ancestors(rov, link[0]);
//       Q_ASSERT(ancestors.size() == 1);
//       ModelItemPtr item = ancestors.first().item;
//       Q_ASSERT(FILTER == item->type());
//       FilterSPtr filter = model->findFilter(item);
//       Q_ASSERT(filter.get() != NULL);
//       
//       FilterOutputId channelId = link[1].toInt();
//       filter->update(channelId);
//       ChannelSPtr channel = factory->createChannel(filter, channelId);
//       channel->initialize(args);
//       
//       if (filter->validOutput(channelId) && channel->volume()->toITK().IsNotNull())
//       {
//         model->addChannel(channel);
//         input->setItem(rov, channel.get());
//       }
//       else
//       {
//         RelationshipGraph::Vertices successors = input->succesors(rov, QString());
//         if (!successors.empty())
//         {
//           QApplication::setOverrideCursor(Qt::ArrowCursor);
//           QMessageBox mBox;
//           mBox.setIcon(QMessageBox::Critical);
//           mBox.setWindowTitle(QMessageBox::tr("EspINA"));
//           mBox.setText(QMessageBox::tr("Channel \"%1\" couldn't be loaded and there are elements that depend on it. File loading aborted.").arg(QString(v.name.c_str())));
//           mBox.setStandardButtons(QMessageBox::Ok);
//           mBox.exec();
//           QApplication::restoreOverrideCursor();
//           return false;
//         }
//         
//         model->removeFilter(filter);
//         input->removeEdges(rov);
//       }
      break;
    }
    case VertexType::FILTER:
    {
      DirectedGraph::Edges inputConections = content->inEdges(roVertex);

      DirectedGraph::Vertices analysisVertices = analysis->content()->vertices();

      OutputSList inputs;
      foreach(DirectedGraph::Edge edge, inputConections)
      {
        std::cout << edge.relationship << std::endl;
      }

      FilterSPtr filter = factory->createFilter(inputs, roVertex->name());

//       Filter::NamedInputs inputs;
//       Filter::Arguments args(rov.args.c_str());
//       QStringList inputLinks = args[Filter::INPUTS].split(",", QString::SkipEmptyParts);
//       
//       // We need to update id values for future filters
//       foreach(QString inputLink, inputLinks)
//       {
//         QStringList link = inputLink.split("_");
//         Q_ASSERT(link.size() == 2);
//         RelationshipGraph::Vertices ancestors = input->ancestors(rov, link[0]);
//         if (ancestors.size() != 1)
//         {
//           qWarning() << QString("Seg File Reader: unexpected number of filters (%1) for %2 relationship").arg(ancestors.size()).arg(link[0]);
//         }
//         Q_ASSERT(ancestors.size() == 1);
//         //qDebug() << "\t" << QString(ancestors.first().args.c_str());
//         RelationshipGraph::Vertex &ancestor = ancestors.first();
//         if (!ancestor.item)
//         {
//           parseVertex(input, ancestor, model, tmpDir, segmentationNodes);
//           qWarning() << QString("Seg File Reader: Unordered relationship (%1 --%2--> %3) fixed.").arg(ancestor.args.c_str()).arg(link[0]).arg(rov.args.c_str());
//         }
//         ModelItemPtr item = ancestor.item;
//         Q_ASSERT(FILTER == item->type());
//         FilterSPtr filter = model->findFilter(item);
//         inputs[link[0]] = filter;
//       }
//       
//       FilterSPtr filter = factory->createFilter(v.name.c_str(), inputs, args);
//       filter->setCacheDir(tmpDir);
//       model->addFilter(filter);
//       input->setItem(rov, filter.get());
      break;
    }
    case VertexType::SEGMENTATION:
    {
      //segmentationNodes << rov;
      break;
    }
    case VertexType::EXTENSION_PROVIDER:
      break;
    default:
      throw Graph::Unknown_Type_Found();
      break;
  }

  return vertex;
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadRelations(AnalysisSPtr    analysis,
                               QuaZip&         zip,
                               ErrorHandlerPtr handler)
{

}


