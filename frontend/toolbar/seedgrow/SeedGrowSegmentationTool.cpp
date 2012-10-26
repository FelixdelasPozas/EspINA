/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "SeedGrowSegmentationTool.h"

#include "common/model/Channel.h"
#include "common/model/EspinaModel.h"
#include "common/model/EspinaFactory.h"
#include "common/model/Sample.h"
#include "common/model/Segmentation.h"
#include "common/gui/EspinaRenderView.h"
#include "common/gui/ViewManager.h"
#include "common/tools/PickableItem.h"
#include "frontend/toolbar/seedgrow/SeedGrowSegmentationFilter.h"
#include "frontend/toolbar/seedgrow/gui/ThresholdAction.h"
#include "gui/DefaultVOIAction.h"
#include "Settings.h"

#include <QApplication>
#include <QWheelEvent>

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::CreateSegmentation::CreateSegmentation(Channel* channel,
                                                                 SeedGrowSegmentationFilter* filter,
                                                                 TaxonomyElement* taxonomy,
                                                                 EspinaModel* model)
: m_model   (model)
, m_channel (channel)
, m_filter  (filter)
, m_taxonomy(taxonomy)
{
  m_sample = m_channel->sample();
  Q_ASSERT(m_sample);

  m_seg = m_model->factory()->createSegmentation(m_filter, 0);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::CreateSegmentation::redo()
{
  m_model->addFilter(m_filter);
  m_model->addRelation(m_channel->filter(), m_filter, SeedGrowSegmentationFilter::INPUTLINK);
  m_seg->setTaxonomy(m_taxonomy);
  m_model->addSegmentation(m_seg);
  m_model->addRelation(m_filter, m_seg, CREATELINK);
  m_model->addRelation(m_sample, m_seg, Sample::WHERE);
  m_model->addRelation(m_channel, m_seg, Channel::LINK);
  m_seg->initializeExtensions();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::CreateSegmentation::undo()
{
  m_model->removeRelation(m_channel->filter(), m_seg, SeedGrowSegmentationFilter::INPUTLINK);
  m_model->removeRelation(m_sample, m_seg, Sample::WHERE);
  m_model->removeRelation(m_filter, m_seg, CREATELINK);
  m_model->removeSegmentation(m_seg);
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  m_model->removeFilter(m_filter);
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(EspinaModel *model,
                                                   QUndoStack  *undoStack,
                                                   ViewManager *viewManager,
                                                   ThresholdAction  *th,
                                                   DefaultVOIAction *voi,
                                                   SeedGrowSegmentation::Settings *settings,
                                                   IPicker *picker)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_threshold(th)
, m_defaultVOI(voi)
, m_settings(settings)
, m_picker(NULL)
, m_inUse(true)
, m_enabled(true)
, m_preview(NULL)
{
  Q_ASSERT(m_threshold);
  setChannelPicker(picker);
}

//-----------------------------------------------------------------------------
QCursor SeedGrowSegmentationTool::cursor() const
{
  QCursor cursor(Qt::ArrowCursor);

  if (m_picker && m_inUse)
    cursor = m_picker->cursor();

  return cursor;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::filterEvent(QEvent* e, EspinaRenderView *view)
{
  if (!m_enabled)
    return false;

  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
    if (we->modifiers() == Qt::CTRL)
    {
      int numSteps = we->delta()/8/15;//Refer to QWheelEvent doc.
      m_threshold->setUpperThreshold(m_threshold->upperThreshold() + numSteps);//Using stepBy highlight the input text
      m_threshold->setLowerThreshold(m_threshold->lowerThreshold() + numSteps);//Using stepBy highlight the input text
      //       if (m_preview)
      //       {
        //         m_preview->setThreshold(m_threshold->threshold());
      //       }
      //       view->view()->forceRender();

      return true;
    }
  }else if(e->type() == QEvent::MouseMove)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() == Qt::SHIFT)
    {
//       DisplayRegionList regions;
//       DisplayRegion singlePixel;
// 
//       int xPos, yPos;
//       view->eventPosition(xPos, yPos);
//       singlePixel << QPoint(xPos,yPos);
//       regions << singlePixel;
// 
//       PickList pickList = view->pick(m_filters, regions);
//       if (pickList.size() == 0)
//         return false;

//       Q_ASSERT(pickList.size() == 1);// Only one element selected
//       IPicker::PickedItem element = pickList.first();
//       Q_ASSERT(element.first.size() == 1); // with one pixel
// 
//       QVector3D pick = element.first.first();
//       SelectableItem *input = element.second;
//       int seed[3] = {pick.x(), pick.y(), pick.z()};
      //       if (null == m_preview)
      //       {
        // //     const int w = 40;
      // //       int voi[6] = {seed[0] - w, seed[0] + w,
      // //       seed[1] - w, seed[1] + w,
      // //       seed[2] - w, seed[2] + w};
      //
      //         int voi[6];
      //        view->previewextent(voi);
      //        m_preview = new seedgrowsegmentationfilter(input->volume(), seed, m_threshold->threshold(), voi);
      //        view->addpreview(m_preview);
      //       }
      //       else
      //       {
        //      m_preview->setinput(input->volume());
      //        m_preview->setseed(seed);
      //       }
      //       view->view()->forcerender();
    }else
    {
      //       if (m_preview)
      //       {
        // 	view->removePreview(m_preview);
        // 	delete m_preview;
        // 	m_preview = NULL;
        // 	view->view()->forceRender();
        //       }
    }
  }else if(e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if (me->modifiers() != Qt::CTRL && m_picker)
      return m_picker->filterEvent(e,view);
  }

  return false;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEnabled(bool enable)
{
  if (m_enabled != enable)
    m_enabled = enable;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setInUse(bool value)
{
  if (m_inUse != value)
  {
    m_inUse = value;
    if (!m_inUse)
      emit segmentationStopped();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::previewOn()
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::previewOff()
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setChannelPicker(IPicker* picker)
{
  if (picker == m_picker)
    return;

  if (m_picker)
  {
    disconnect(m_picker, SIGNAL(itemsPicked(IPicker::PickList)),
               this, SLOT(startSegmentation(IPicker::PickList)));
  }

  m_picker = picker;

  if (m_picker)
  {
    m_picker->setPickable(IPicker::CHANNEL);
    m_picker->setPickable(IPicker::SEGMENTATION, false);
    connect(m_picker, SIGNAL(itemsPicked(IPicker::PickList)),
            this, SLOT(startSegmentation(IPicker::PickList)));
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::startSegmentation(IPicker::PickList pickedItems)
{
  if (pickedItems.size() != 1)
    return;

  IPicker::PickedItem element = pickedItems.first();
  PickableItem *input = element.second;

  Q_ASSERT(element.first->GetNumberOfPoints() == 1); // with one pixel
  Nm seedPoint[3];
  element.first->GetPoint(0, seedPoint);
  //     qDebug() << "Channel:" << input->volume().id();
  //     qDebug() << "Threshold:" << m_threshold->threshold();
  //     qDebug() << "Seed:" << seed;
  //     qDebug() << "Use Default VOI:" << m_useDefaultVOI->useDefaultVOI();

  Q_ASSERT(ModelItem::CHANNEL == input->type());
  Channel *channel = m_viewManager->activeChannel();

  if (!channel)
    return;
  EspinaVolume::IndexType seed = channel->index(seedPoint[0], seedPoint[1], seedPoint[2]);
  if (seed[0] < 0 || seed[1] < 0 || seed[2] < 0)
    return;

  double spacing[3];
  channel->spacing(spacing);

  Nm voiBounds[6];
  IVOI::Region currentVOI = m_viewManager->voiRegion();
  if (currentVOI)
  {
    memcpy(voiBounds, currentVOI, 6*sizeof(double));
  }
  else if (m_defaultVOI->useDefaultVOI())
  {
    voiBounds[0] = seed[0]*spacing[0] - 60;//m_settings->xSize();
    voiBounds[1] = seed[0]*spacing[0] + 60;//m_settings->xSize();
    voiBounds[2] = seed[1]*spacing[1] - 60;//m_settings->ySize();
    voiBounds[3] = seed[1]*spacing[1] + 60;//m_settings->ySize();
    voiBounds[4] = seed[2]*spacing[2] - 60;//m_settings->zSize();
    voiBounds[5] = seed[2]*spacing[2] + 60;//m_settings->zSize();
  } else
  {
    channel->bounds(voiBounds);
  }

  int voiExtent[6];
  for (int i=0; i<6; i++)
    voiExtent[i] = (voiBounds[i] / spacing[i/2]) + 0.5;

  Q_ASSERT(m_threshold->isSymmetrical());
  if (m_threshold->isSymmetrical())
  {
    Q_ASSERT(m_threshold->lowerThreshold() == m_threshold->upperThreshold());
    Q_ASSERT(m_threshold->lowerThreshold() >= 0);
    Q_ASSERT(m_threshold->lowerThreshold() <= 255);
  }

  if (voiExtent[0] <= seed[0] && seed[0] <= voiExtent[1] &&
    voiExtent[2] <= seed[1] && seed[1] <= voiExtent[3] &&
    voiExtent[4] <= seed[2] && seed[2] <= voiExtent[5])
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    Filter::NamedInputs inputs;
    Filter::Arguments args;
    SeedGrowSegmentationFilter::Parameters params(args);
    params.setSeed(seed);
    params.setLowerThreshold(m_threshold->lowerThreshold());
    params.setUpperThreshold(m_threshold->upperThreshold());
    params.setVOI(voiExtent);
    params.setCloseValue(m_settings->closing());
    inputs[SeedGrowSegmentationFilter::INPUTLINK] = channel->filter();
    args[Filter::INPUTS] = SeedGrowSegmentationFilter::INPUTLINK + "_" + QString::number(channel->outputNumber());
    SeedGrowSegmentationFilter *filter;
    filter = new SeedGrowSegmentationFilter(inputs, args);
    filter->update();
    Q_ASSERT(filter->numberOutputs() == 1);

    TaxonomyElement *tax = m_viewManager->activeTaxonomy();
    Q_ASSERT(tax);

    m_undoStack->push(new CreateSegmentation(channel, filter, tax, m_model));
    QApplication::restoreOverrideCursor();
  }
}