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

#include "SeedGrowSegmentationTool.h"

#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/ModelAdapterUtils.h>
#include <Filters/SeedGrowSegmentationFilter.h>

#include <QAction>
#include <QApplication>

using namespace EspINA;

const Filter::Type SGS_FILTER = "SeedGrowSegmentation";

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(ModelAdapterSPtr model,
                                                   ModelFactorySPtr factory,
                                                   ViewManagerSPtr  viewManager)
: m_model(model)
, m_factory(factory)
, m_viewManager(viewManager)
, m_enabled(false)
, m_selectorSwitch(new ActionSelector())
, m_seedThreshold(new SeedThreshold())
, m_applyROI(new ApplyROI())
{
  m_factory->registerFilterFactory(this);

  { // Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/pixelSelector.svg"),
                                  tr("Create segmentation based on selected pixel (Ctrl +)"),
                                  m_selectorSwitch);

    SelectorSPtr selector{new PixelSelector()};

    addVoxelSelector(action, selector);
  }


  { // Best Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                                  tr("Create segmentation based on best pixel (Ctrl +)"),
                                  m_selectorSwitch);

    SelectorSPtr selector{new BestPixelSelector()};
    QCursor      cursor(QPixmap(":/espina/crossRegion.svg"));

    selector->setCursor(cursor);

    addVoxelSelector(action, selector);
  }

  connect(m_selectorSwitch, SIGNAL(triggered(QAction*)),
          this, SLOT(changeSelector(QAction*)));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::~SeedGrowSegmentationTool()
{
  delete m_selectorSwitch;
  delete m_seedThreshold;
  delete m_applyROI;
}

//-----------------------------------------------------------------------------
FilterTypeList SeedGrowSegmentationTool::providedFilters() const
{
  FilterTypeList filters;

  filters << SGS_FILTER;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SeedGrowSegmentationTool::createFilter(OutputSList         inputs,
                                                  const Filter::Type& filter,
                                                  SchedulerSPtr       scheduler) const
{
  if (filter != SGS_FILTER) throw Unknown_Filter_Exception();

  return FilterSPtr{new SeedGrowSegmentationFilter(inputs, filter, scheduler)};
}

//-----------------------------------------------------------------------------
QList<QAction *> SeedGrowSegmentationTool::actions() const
{
  QList<QAction *> actions;

  actions << m_selectorSwitch;
  actions << m_seedThreshold;
  actions << m_applyROI;

  return actions;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEnabled(bool value)
{
  m_enabled = value;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::addVoxelSelector(QAction* action, SelectorSPtr selector)
{
  m_selectorSwitch->addAction(action);
  m_voxelSelectors[action] = selector;

  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);

  connect(selector.get(), SIGNAL(selectorInUse(bool)),
          m_selectorSwitch, SLOT(setChecked(bool)));
  connect(selector.get(), SIGNAL(itemsSelected(Selector::SelectionList)),
          this, SLOT(launchTask(Selector::SelectionList)));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::changeSelector(QAction* action)
{
  m_viewManager->setSelector(m_voxelSelectors[action]);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::launchTask(Selector::SelectionList selectedItems)
{
//  if (selectedItems.size() != 1)
//    return;

//  auto element = selectedItems.first();
//  auto input   = element.second;

//  Q_ASSERT(element.first->GetNumberOfPoints() == 1); // with one pixel

//  Nm seedPoint[3];
//  element.first->GetPoint(0, seedPoint);

//  Q_ASSERT(ItemAdapter::Type::CHANNEL == input->type());
  auto channel = m_viewManager->activeChannel();

//  if (!channel)
//    return;

//  auto channelVolume = volumetricData(channel->output());

//  Bounds voxelBounds({seedPoint[0], seedPoint[0], seedPoint[1], seedPoint[1], seedPoint[2], seedPoint[2]});
//  voxelBounds.setUpperInclusion(true);

//  auto voxel = channelVolume->itkImage(voxelBounds);

//  itkVolumeType::IndexType seed = channel->volume()->index(seedPoint[0], seedPoint[1], seedPoint[2]);
//  if (seed[0] < 0 || seed[1] < 0 || seed[2] < 0)
//    return;

//  double spacing[3];
//  channel->volume()->spacing(spacing);

//  TaxonomyElementPtr tax = m_viewManager->activeTaxonomy();
//  Q_ASSERT(tax);

//  Nm voiBounds[6];
//  IVOI::Region currentVOI = m_viewManager->voiRegion();
//  if (currentVOI)
//  {
//    memcpy(voiBounds, currentVOI, 6*sizeof(double));
//  }
//  else if (m_defaultVOI->useDefaultVOI())
//  {
//    voiBounds[0] = seed[0]*spacing[0];
//    voiBounds[1] = seed[0]*spacing[0];
//    voiBounds[2] = seed[1]*spacing[1];
//    voiBounds[3] = seed[1]*spacing[1];
//    voiBounds[4] = seed[2]*spacing[2];
//    voiBounds[5] = seed[2]*spacing[2];

//    if (!tax->properties().contains(TaxonomyElement::X_DIM) ||
//        !tax->properties().contains(TaxonomyElement::X_DIM) ||
//        !tax->properties().contains(TaxonomyElement::X_DIM))
//    {
//      tax->addProperty(TaxonomyElement::X_DIM, QVariant(m_settings->xSize()));
//      tax->addProperty(TaxonomyElement::Y_DIM, QVariant(m_settings->ySize()));
//      tax->addProperty(TaxonomyElement::Z_DIM, QVariant(m_settings->zSize()));
//    }

//    QVariant xTaxSize = tax->property(TaxonomyElement::X_DIM);
//    QVariant yTaxSize = tax->property(TaxonomyElement::Y_DIM);
//    QVariant zTaxSize = tax->property(TaxonomyElement::Z_DIM);

//    Nm xSize, ySize, zSize;

//    if (m_settings->taxonomicalVOI() && xTaxSize.isValid() && yTaxSize.isValid() && zTaxSize.isValid())
//    {
//      xSize = xTaxSize.toDouble();
//      ySize = yTaxSize.toDouble();
//      zSize = zTaxSize.toDouble();
//    }
//    else
//    {
//      xSize = m_settings->xSize();
//      ySize = m_settings->ySize();
//      zSize = m_settings->zSize();
//    }

//    voiBounds[0] -= xSize/2.0;
//    voiBounds[1] += xSize/2.0;
//    voiBounds[2] -= ySize/2.0;
//    voiBounds[3] += ySize/2.0;
//    voiBounds[4] -= zSize/2.0;
//    voiBounds[5] += zSize/2.0;

//  } else
//  {
//    channel->volume()->bounds(voiBounds);
//  }

//  int voiExtent[6];
//  for (int i=0; i<6; i++)
//    voiExtent[i] = vtkMath::Round(voiBounds[i] / spacing[i/2]);

//  Q_ASSERT(m_threshold->isSymmetrical());
//  if (m_threshold->isSymmetrical())
//  {
//    Q_ASSERT(m_threshold->lowerThreshold() == m_threshold->upperThreshold());
//    Q_ASSERT(m_threshold->lowerThreshold() >= 0);
//    Q_ASSERT(m_threshold->lowerThreshold() <= 255);
//  }

  bool validSeed = true;
//  if (voiExtent[0] <= seed[0] && seed[0] <= voiExtent[1] &&
//    voiExtent[2] <= seed[1] && seed[1] <= voiExtent[3] &&
//    voiExtent[4] <= seed[2] && seed[2] <= voiExtent[5])

  if (validSeed)
  {
//     QApplication::setOverrideCursor(Qt::WaitCursor);

    m_selectorSwitch->setEnabled(false);

    OutputSList inputs;

    inputs << channel->output();

    auto task = m_factory->createFilter<SeedGrowSegmentationFilter>(inputs, SGS_FILTER);
    connect(task.get(), SIGNAL(progress(int)),
            this, SLOT(onTaskProgres(int)));
    connect(task.get(), SIGNAL(finished()),
            this, SLOT(createSegmentation()));
    task->submit();

    m_executingTask = task;

//     QApplication::restoreOverrideCursor();
  }
//    SegmentationSList createdSegmentations;
//    m_undoStack->beginMacro(tr("Seed Grow Segmentation"));
//    m_undoStack->push(new SeedGrowSegmentationCommand(channel,
//                                                      seed,
//                                                      voiExtent,
//                                                      m_threshold->lowerThreshold(),
//                                                      m_threshold->upperThreshold(),
//                                                      m_settings->closing(),
//                                                      m_viewManager->activeTaxonomy(),
//                                                      m_model,
//                                                      m_viewManager,
//                                                      createdSegmentations));
//    m_model->emitSegmentationAdded(createdSegmentations);
//    m_undoStack->endMacro();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onTaskProgres(int progress)
{
  std::cout << "Progress: " << progress << std::endl;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::createSegmentation()
{
  std::cout << "Number of outputs" << m_executingTask->numberOfOutputs() << std::endl;
  m_selectorSwitch->setEnabled(true);
}


