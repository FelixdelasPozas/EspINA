/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// plugin
#include "AppositionSurfacePlugin.h"
#include <Filter/AppositionSurfaceFilter.h>
#include <Filter/SASDataFactory.h>
#include <GUI/Analysis/SASAnalysisDialog.h>
#include <GUI/AppositionSurfaceTool.h>
#include <GUI/Settings/AppositionSurfaceSettings.h>
#include <GUI/AppositionSurfaceTool.h>
#include <Core/Extensions/ExtensionFactory.h>

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <Core/IO/DataFactory/RasterizedVolumeFromFetchedMeshData.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Undo/AddCategoryCommand.h>
#include <Support/Utils/SelectionUtils.h>
#include <Support/Settings/EspinaSettings.h>
#include <Undo/AddSegmentations.h>
#include <Undo/AddRelationCommand.h>

// Qt
#include <QColorDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QVariant>

const QString SAS = QObject::tr("SAS");
const QString SASTAG_PREPEND = QObject::tr("SAS ");

using namespace ESPINA;

//-----------------------------------------------------------------------------
FilterTypeList AppositionSurfacePlugin::ASFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << AS_FILTER;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr AppositionSurfacePlugin::ASFilterFactory::createFilter(InputSList    inputs,
                                                            const Filter::Type& type,
                                                            SchedulerSPtr       scheduler) const
throw (Unknown_Filter_Exception)
{

  if (type != AS_FILTER) throw Unknown_Filter_Exception();

  auto filter = std::make_shared<AppositionSurfaceFilter>(inputs, type, scheduler);

  filter->setDataFactory(std::make_shared<SASDataFactory>());

  return filter;
}

//-----------------------------------------------------------------------------
AppositionSurfacePlugin::AppositionSurfacePlugin()
: m_context         {nullptr}
, m_settings        {new AppositionSurfaceSettings()}
, m_extensionFactory{new ASExtensionFactory()}
, m_filterFactory   {new ASFilterFactory()}
, m_delayedAnalysis {false}
{
  QStringList hierarchy;
  hierarchy << "Reports";

  QAction *action = new QAction(tr("Synaptic Apposition Surface"), this);
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(createSASAnalysis()));

  m_menuEntry = MenuEntry(hierarchy, action);
}

//-----------------------------------------------------------------------------
AppositionSurfacePlugin::~AppositionSurfacePlugin()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Apposition Surface Plugin";
//   qDebug() << "********************************************************";
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::init(Support::Context &context)
{
  if (m_context != nullptr)
  {
    qWarning() << "Already Initialized AppositionSurfacePlugin";
    Q_ASSERT(false);
  }

  m_context = &context;

  // for automatic computation of SAS
  connect(m_context->model().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList,TimeStamp)),
          this,                     SLOT(segmentationsAdded(ViewItemAdapterSList)));
}

//-----------------------------------------------------------------------------
ChannelExtensionFactorySList AppositionSurfacePlugin::channelExtensionFactories() const
{
  return ChannelExtensionFactorySList();
}

//-----------------------------------------------------------------------------
SegmentationExtensionFactorySList AppositionSurfacePlugin::segmentationExtensionFactories() const
{
  SegmentationExtensionFactorySList extensionFactories;

  extensionFactories << m_extensionFactory;

  return extensionFactories;
}

//-----------------------------------------------------------------------------
FilterFactorySList AppositionSurfacePlugin::filterFactories() const
{
  FilterFactorySList factories;

  factories << m_filterFactory;

  return factories;
}

//-----------------------------------------------------------------------------
NamedColorEngineSList AppositionSurfacePlugin::colorEngines() const
{
  return NamedColorEngineSList();
}

//-----------------------------------------------------------------------------
RepresentationFactorySList AppositionSurfacePlugin::representationFactories() const
{
  return RepresentationFactorySList();
}

//-----------------------------------------------------------------------------
QList<CategorizedTool> AppositionSurfacePlugin::tools() const
{
  QList<CategorizedTool> tools;

  auto plugin = const_cast<AppositionSurfacePlugin *>(this);

  tools << CategorizedTool(ToolCategory::SEGMENTATE, std::make_shared<AppositionSurfaceTool>(plugin, *m_context));

  return tools;
}

//-----------------------------------------------------------------------------
QList<DockWidget *> AppositionSurfacePlugin::dockWidgets() const
{
  return QList<DockWidget *>();
}

//-----------------------------------------------------------------------------
SettingsPanelSList AppositionSurfacePlugin::settingsPanels() const
{
  SettingsPanelSList settingsPanels;

  settingsPanels << m_settings;

  return settingsPanels;
}

//-----------------------------------------------------------------------------
QList<MenuEntry> AppositionSurfacePlugin::menuEntries() const
{
  QList<MenuEntry> entries;

  entries << m_menuEntry;

  return entries;
}

//-----------------------------------------------------------------------------
AnalysisReaderSList AppositionSurfacePlugin::analysisReaders() const
{
  return AnalysisReaderSList();
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::createSASAnalysis()
{
  Q_ASSERT(m_context);

  SegmentationAdapterList synapsis;

  //TODO: Uncomment all method
//   for(auto segmentation: defaultReportInputSegmentations(m_viewManager, m_model))
//   {
//     if (isValidSynapse(segmentation))
//     {
//       synapsis << segmentation;
//     }
//   }
//
//   if (synapsis.isEmpty())
//   {
//     QMessageBox::warning(nullptr, tr("ESPINA"), tr("Current analysis does not contain any synapses"));
//     return;
//   }
//
//   if (m_model->classification()->category(SAS) == nullptr)
//   {
//     m_undoStack->beginMacro(tr("Apposition Surface"));
//     m_undoStack->push(new AddCategoryCommand(m_model->classification()->root(), SAS, m_model, QColor(255, 255, 0)));
//     m_undoStack->endMacro();
//
//     m_model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
//     m_model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
//     m_model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
//   }
//
//   // check segmentations for SAS and create it if needed
//   for (auto segmentation : synapsis)
//   {
//     auto sasItems = m_model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS);
//     if (sasItems.empty())
//     {
//       if (!m_delayedAnalysis)
//       {
//         m_delayedAnalysis = true;
//         m_analysisSynapses = synapsis;
//         QApplication::setOverrideCursor(Qt::WaitCursor);
//       }
//
//       InputSList inputs;
//       inputs << segmentation->asInput();
//
//       auto filter = m_factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);
//
//       struct Data data(filter, m_model->smartPointer(segmentation));
//       m_executingTasks.insert(filter.get(), data);
//
//       connect(filter.get(), SIGNAL(finished()), this, SLOT(finishedTask()));
//
//       Task::submit(filter);
//     }
//     else
//     {
//       Q_ASSERT(sasItems.size() == 1);
//       auto sas = std::dynamic_pointer_cast<SegmentationAdapter>(sasItems.first());
//       if (!sas->hasExtension(AppositionSurfaceExtension::TYPE))
//       {
//         auto extension = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
//         std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension)->setOriginSegmentation(m_model->smartPointer(segmentation));
//         sas->addExtension(extension);
//       }
//     }
//  }
/*
  if (!m_delayedAnalysis)
  {
    SASAnalysisDialog *analysis = new SASAnalysisDialog(synapsis, m_model, m_undoStack, m_factory, m_viewManager, nullptr);
    analysis->exec();

    delete analysis;
  } */
}

//-----------------------------------------------------------------------------
bool AppositionSurfacePlugin::isValidSynapse(SegmentationAdapterPtr segmentation)
{
  bool isValidCategory = segmentation->category()->classificationName().contains(tr("Synapse"));
  bool hasRequiredData = segmentation->output()->hasData(VolumetricData<itkVolumeType>::TYPE);

  return (isValidCategory && hasRequiredData);
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::segmentationsAdded(ViewItemAdapterSList segmentations)
{
  ESPINA_SETTINGS(settings);
  settings.beginGroup("Apposition Surface");
  if (!settings.contains("Automatic Computation For Synapses") || !settings.value("Automatic Computation For Synapses").toBool())
    return;

  auto m_model   = m_context->model();
  auto m_factory = m_context->factory();
  SegmentationAdapterList validSegmentations;
  for(auto item : segmentations)
  {
    auto segmentation = segmentationPtr(item.get());

    if(isValidSynapse(segmentation) && segmentation->filter()->hasFinished())
    {
      bool valid = true;
      // must check if the segmentation already has a SAS, as this call
      // could be the result of a redo() in a UndoCommand
      for(auto relatedItem : m_model->relatedItems(segmentation, RELATION_OUT))
      {
        if (isSegmentation(relatedItem.get()) && isSAS(relatedItem))
        {
          valid = false;
          break;
        }
      }

      if (valid)
      {
        validSegmentations << segmentation;
      }
    }
  }

  for(auto seg: validSegmentations)
  {
    InputSList inputs;
    inputs << seg->asInput();

    auto filter = m_factory->createFilter<AppositionSurfaceFilter>(inputs, AS_FILTER);

    struct Data data(filter, m_model->smartPointer(seg));
    m_executingTasks.insert(filter.get(), data);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(finishedTask()));

    Task::submit(filter);
  }
}

//-----------------------------------------------------------------------------
void AppositionSurfacePlugin::finishedTask()
{
  auto m_model     = m_context->model();
  auto m_factory   = m_context->factory();
  auto m_undoStack = m_context->undoStack();

  auto filter = dynamic_cast<FilterPtr>(sender());
  disconnect(filter, SIGNAL(finished()), this, SLOT(finishedTask()));

  if(!filter->isAborted())
    m_finishedTasks.insert(filter, m_executingTasks[filter]);

  m_executingTasks.remove(filter);

  if (!m_executingTasks.empty())
    return;

  // maybe all tasks have been aborted.
  if(m_finishedTasks.empty())
    return;

  m_undoStack->beginMacro("Create Synaptic Apposition Surfaces");

  auto classification = m_model->classification();
  if (classification->category(SAS) == nullptr)
  {
    m_undoStack->push(new AddCategoryCommand(m_model->classification()->root(), SAS, m_model, QColor(255,255,0)));

    m_model->classification()->category(SAS)->addProperty(QString("Dim_X"), QVariant("500"));
    m_model->classification()->category(SAS)->addProperty(QString("Dim_Y"), QVariant("500"));
    m_model->classification()->category(SAS)->addProperty(QString("Dim_Z"), QVariant("500"));
  }

  CategoryAdapterSPtr category = classification->category(SAS);

  // we don't have an operator< for SampleAdapterSList so can't use a QMap and must use two lists to avoid
  // adding the segmentations one by one. Each segmentation can have a different sample list.
  QVector<SampleAdapterSList> usedSamples;
  QVector<SegmentationAdapterSList> createdSegmentations;
  int index = 0;

  RelationList createdRelations;
  ViewItemAdapterSList segmentationsToUpdate;

  for(auto filter: m_finishedTasks.keys())
  {
    auto segmentation = m_factory->createSegmentation(m_finishedTasks.value(filter).adapter, 0);
    segmentation->setCategory(category);
    segmentation->setData(SASTAG_PREPEND + QString::number(m_finishedTasks[filter].segmentation->number()), Qt::EditRole);

    auto extension = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE);
    std::dynamic_pointer_cast<AppositionSurfaceExtension>(extension)->setOriginSegmentation(m_finishedTasks[filter].segmentation);
    segmentation->addExtension(extension);

    auto samples = QueryAdapter::samples(m_finishedTasks.value(filter).segmentation);
    Q_ASSERT(!samples.empty());

    for(index = 0; index < usedSamples.size(); ++index)
      if(samples == usedSamples[index])
        break;

    if(index == usedSamples.size())
    {
      usedSamples.push_back(samples);
      SegmentationAdapterSList list;
      list << segmentation;
      createdSegmentations.push_back(list);
    }
    else
    {
      createdSegmentations[index] << segmentation;
    }

    Relation relation;
    relation.ancestor = m_finishedTasks[filter].segmentation;
    relation.successor = segmentation;
    relation.relation = SAS;

    createdRelations << relation;

    segmentationsToUpdate << segmentation;
  }

  // add segmentations by their related sample list
  for(index = 0; index < usedSamples.size(); ++index)
  {
    m_undoStack->push(new AddSegmentations(createdSegmentations[index], usedSamples[index], m_model));
  }

  // add relations
  m_undoStack->push(new AddRelationsCommand(createdRelations, m_model));

  m_undoStack->endMacro();

  m_model->notifyRepresentationsModified(segmentationsToUpdate);

  m_finishedTasks.clear();

  if(m_delayedAnalysis)
  {
    QApplication::restoreOverrideCursor();
    SASAnalysisDialog dialog(m_analysisSynapses, *m_context);
    dialog.exec();

    m_delayedAnalysis = false;
    m_analysisSynapses.clear();
  }
}

//-----------------------------------------------------------------------------
bool AppositionSurfacePlugin::isSAS(ItemAdapterSPtr item)
{
  bool result = false;

  auto sas = segmentationPtr(item.get());

  if (sas)
  {
    auto category = sas->category()->classificationName();

    result = category.startsWith("SAS/") || category.compare("SAS") == 0;
  }

  return result;
}

Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, ESPINA::AppositionSurfacePlugin)

