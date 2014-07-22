/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Plugin
#include "SegmhaImporterPlugin.h"

// ESPINA
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/IO/ReadOnlyFilter.h>
#include <Filters/SourceFilter.h>

using namespace ESPINA;

//const QString SegmhaImporter::UndoCommand::FILTER_TYPE = "Segmha Importer";

static const QString SEGMHA = "segmha";

static const Filter::Type SEGMHA_FILTER_V4 = "Segmha Importer";
static const Filter::Type SEGMHA_FILTER = "SegmhaReader";

//-----------------------------------------------------------------------------
FilterSPtr SegmhaFilterFactory::createFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception)
{
  FilterSPtr filter;

  if(type == SEGMHA_FILTER)
  {
    filter = FilterSPtr{new SourceFilter(inputs, type, scheduler)};
  }
  else
    if(type == SEGMHA_FILTER_V4)
    {
      filter = FilterSPtr{new IO::SegFile::ReadOnlyFilter(inputs, type)};
    }
    else
      throw Unknown_Filter_Exception();

  filter->setFetchBehaviour(FetchBehaviourSPtr{new MarchingCubesFromFetchedVolumetricData()});
  return filter;
}

//-----------------------------------------------------------------------------
FilterTypeList SegmhaFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SEGMHA_FILTER;
  filters << SEGMHA_FILTER_V4;

  return filters;
}


//-----------------------------------------------------------------------------
SegmhaImporterPlugin::SegmhaImporterPlugin()
: m_undoStack    {nullptr}
, m_reader       {new SegmhaReader()}
, m_filterFactory{new SegmhaFilterFactory()}
{
}


//-----------------------------------------------------------------------------
SegmhaImporterPlugin::~SegmhaImporterPlugin()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying SegmhaImporter Plugin";
//   qDebug() << "********************************************************";
}

//-----------------------------------------------------------------------------
void SegmhaImporterPlugin::init(ModelAdapterSPtr model,
                                ViewManagerSPtr  viewManager,
                                ModelFactorySPtr factory,
                                SchedulerSPtr    scheduler,
                                QUndoStack*      undoStack)
{
  m_model       = model;
  m_viewManager = viewManager;
  m_factory     = factory;
  m_scheduler   = scheduler;
  m_undoStack   = undoStack;
}

//------------------------------------------------------------------------
AnalysisReaderSList SegmhaImporterPlugin::analysisReaders() const
{
  AnalysisReaderSList readers;

  readers << m_reader;

  return readers;
}

//------------------------------------------------------------------------
NamedColorEngineSList SegmhaImporterPlugin::colorEngines() const
{
  return NamedColorEngineSList();
}

//------------------------------------------------------------------------
QList<ToolGroup* > SegmhaImporterPlugin::toolGroups() const
{
  return QList<ToolGroup *>();
}

//------------------------------------------------------------------------
QList<DockWidget *> SegmhaImporterPlugin::dockWidgets() const
{
  return QList<DockWidget *>();
}

//------------------------------------------------------------------------
ChannelExtensionFactorySList SegmhaImporterPlugin::channelExtensionFactories() const
{
  return ChannelExtensionFactorySList();
}

//------------------------------------------------------------------------
SegmentationExtensionFactorySList SegmhaImporterPlugin::segmentationExtensionFactories() const
{
  return SegmentationExtensionFactorySList();
}

//------------------------------------------------------------------------
FilterFactorySList SegmhaImporterPlugin::filterFactories() const
{
  FilterFactorySList factories;

  factories << m_filterFactory;

  return factories;
}

//------------------------------------------------------------------------
RendererSList SegmhaImporterPlugin::renderers() const
{
  return RendererSList();
}

//------------------------------------------------------------------------
SettingsPanelSList SegmhaImporterPlugin::settingsPanels() const
{
  return SettingsPanelSList();
}

//------------------------------------------------------------------------
QList<MenuEntry> SegmhaImporterPlugin::menuEntries() const
{
  return QList<MenuEntry>();
}

// //-----------------------------------------------------------------------------
// FilterSPtr SegmhaImporter::createFilter(const QString              &filter,
//                                         const Filter::NamedInputs  &inputs,
//                                         const ModelItem::Arguments &args)
// {
//   Q_ASSERT(UndoCommand::FILTER_TYPE == filter);
//   FilterSPtr reader(new SegmhaImporterFilter(inputs, args, UndoCommand::FILTER_TYPE));
//   SetBasicGraphicalRepresentationFactory(reader);
//   return reader;
// }
//
// //-----------------------------------------------------------------------------
// void SegmhaImporter::initFileReader(EspinaModel *model,
//                                     QUndoStack  *undoStack,
//                                     ViewManager *viewManager)
// {
//   m_model = model;
//   m_undoStack = undoStack;
//   m_viewManager = viewManager;
//   // Register filter and reader factories
//   QStringList supportedExtensions;
//   supportedExtensions << SEGMHA;
//   m_model->factory()->registerReaderFactory(this,
//                                            SegmhaImporterFilter::SUPPORTED_FILES,
//                                            supportedExtensions);
// }
//
// //-----------------------------------------------------------------------------
// bool SegmhaImporter::readFile(const QFileInfo file, IOErrorHandler *handler)
// {
//   Q_ASSERT(SEGMHA == file.suffix());
//
//   QFileInfo channelFile = handler->fileNotFound(QFileInfo(),
//                                                 file.dir(),
//                                                 CHANNEL_FILES,
//                                                 tr("Select channel file for %1:").arg(file.fileName()));
//   if (!channelFile.exists())
//   {
//     handler->error(tr("%1 doesn't exist").arg(channelFile.absoluteFilePath()));
//     return false;
//   }
//
//   m_model->setTraceable(false);
//
//   ChannelSPtr channel;
//   if (IOErrorHandler::SUCCESS != EspinaIO::loadChannel(channelFile, m_model, channel, handler))
//     return false;
//
//   Filter::NamedInputs inputs;
//   Filter::Arguments args;
//   args[SegmhaImporterFilter::FILE] = file.absoluteFilePath();
//   SegmhaImporterFilter::Parameters params(args);
//   params.setSpacing(channel->volume()->toITK()->GetSpacing());
//
//   SegmhaImporterFilterSPtr filter(new SegmhaImporterFilter(inputs, args, UndoCommand::FILTER_TYPE));
//   filter->setGraphicalRepresentationFactory(GraphicalRepresentationFactorySPtr(new BasicGraphicalRepresentationFactory()));
//   filter->update();
//   if (filter->outputs().isEmpty())
//   {
//     handler->error(tr("Failed to import %1").arg(file.absoluteFilePath()));
//     return false;
//   }
//
//   SampleSPtr sample = channel->sample();
//
//   // update taxonomy
//   m_model->setTaxonomy(filter->taxonomy());
//
//   SegmentationSList segmentations;
//   SegmentationSPtr seg;
//   foreach(SegmentationOutputSPtr segOutput, filter->outputs())
//   {
//     seg = m_model->factory()->createSegmentation(filter, segOutput->id());
//     filter->initSegmentation(seg, segOutput->id());
//     segmentations << seg;
//   }
//
//   Channel::ExtensionPtr extension = channel->extension(CountingFrameExtensionID);
//   if (!extension)
//   {
//     Channel::ExtensionPtr prototype = m_model->factory()->channelExtension(CountingFrameExtensionID);
//     if (prototype)
//     {
//       extension = prototype->clone();
//       channel->addExtension(extension);
//     }
//   }
//
//  if (extension)
//   {
//     CountingFrameExtension *cfExtension = dynamic_cast<CountingFrameExtension *>(extension);
//
//     Nm inclusive[3], exclusive[3];
//     filter->countingFrame(inclusive, exclusive);
//     double spacing[3];
//     channel->volume()->spacing(spacing);
//     for(int i=0; i<3;i++)
//     {
//       inclusive[i] = inclusive[i]*spacing[i];
//       exclusive[i] = exclusive[i]*spacing[i];
//     }
//
//     cfExtension->plugin()->createRectangularCF(channel.get(), inclusive, exclusive);
//   }
//
//   m_model->addFilter(filter);
//   m_model->addSegmentation(segmentations);
//
//   m_model->addRelation(channel, filter, Channel::LINK);
//   foreach(SegmentationSPtr seg, segmentations)
//   {
//     m_model->addRelation(filter,  seg, Filter::CREATELINK);
//     m_model->addRelation(sample,  seg, Relations::LOCATION);
//     m_model->addRelation(channel, seg, Channel::LINK);
//     seg->initializeExtensions();
//   }
//
//   m_model->emitSegmentationAdded(segmentations);
//
//   return true;
// }

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, ESPINA::SegmhaImporterPlugin)
