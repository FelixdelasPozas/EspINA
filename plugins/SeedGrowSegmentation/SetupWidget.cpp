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


#include "SetupWidget.h"
#include <pqPipelineSource.h>
#include <common/EspinaCore.h>
#include <common/gui/EspinaView.h>
#include <common/selection/SelectionManager.h>

#include <QDebug>
#include <QMessageBox>

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SetupWidget::SetupWidget(Filter* filter)
{
  setupUi(this);
  m_filter = dynamic_cast<SeedGrowSegmentationFilter *>(filter);
  int seed[3];
  m_filter->seed(seed);
  m_xSeed->setText(QString("%1").arg(seed[0]));
  m_ySeed->setText(QString("%1").arg(seed[1]));
  m_zSeed->setText(QString("%1").arg(seed[2]));
  m_threshold->setMaximum(255);
  m_threshold->setValue(m_filter->m_args.threshold());
  int VOI[6];
  m_filter->m_args.voi(VOI);
  m_leftMargin->setValue(VOI[0]);
  m_rightMargin->setValue(VOI[1]);
  m_topMargin->setValue(VOI[2]);
  m_bottomMargin->setValue(VOI[3]);
  m_upperMargin->setValue(VOI[4]);
  m_lowerMargin->setValue(VOI[5]);

  m_leftMargin->installEventFilter(this);
  m_rightMargin->installEventFilter(this);
  m_topMargin->installEventFilter(this);
  m_bottomMargin->installEventFilter(this);
  m_upperMargin->installEventFilter(this);
  m_lowerMargin->installEventFilter(this);
//   connect(m_threshold, SIGNAL(valueChanged(int)),
// 	  this, SLOT(modifyFilter()));
  connect(m_modify, SIGNAL(clicked(bool)),
	  this, SLOT(modifyFilter()));
}

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SetupWidget::~SetupWidget()
{
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
  if (!SelectionManager::instance()->voi())
    view->setSliceSelectors(SliceView::NoSelector);
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::SetupWidget::eventFilter(QObject* sender, QEvent* e)
{
  if (e->type() == QEvent::FocusIn)
  {
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    view->setSliceSelectors(SliceView::From| SliceView::To);
    connect(view, SIGNAL(selectedFromSlice(double,vtkPVSliceView::VIEW_PLANE)),
	    this, SLOT(redefineFromVOI(double,vtkPVSliceView::VIEW_PLANE)));
    connect(view, SIGNAL(selectedToSlice(double,vtkPVSliceView::VIEW_PLANE)),
	    this, SLOT(redefineToVOI(double,vtkPVSliceView::VIEW_PLANE)));
  }

  return QObject::eventFilter(sender, e);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::redefineFromVOI(double value, vtkPVSliceView::VIEW_PLANE plane)
{
  switch (plane)
  {
    case vtkPVSliceView::AXIAL:
      m_upperMargin->setValue(value);
      break;
    case vtkPVSliceView::SAGITTAL:
      m_leftMargin->setValue(value);
      break;
    case vtkPVSliceView::CORONAL:
      m_topMargin->setValue(value);
      break;
  }
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::redefineToVOI(double value, vtkPVSliceView::VIEW_PLANE plane)
{
  switch (plane)
  {
    case vtkPVSliceView::AXIAL:
      m_lowerMargin->setValue(value);
      break;
    case vtkPVSliceView::SAGITTAL:
      m_rightMargin->setValue(value);
      break;
    case vtkPVSliceView::CORONAL:
      m_bottomMargin->setValue(value);
      break;
  }
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::modifyFilter()
{
  int VOI[6];
  VOI[0] = m_leftMargin->value();
  VOI[1] = m_rightMargin->value();
  VOI[2] = m_topMargin->value();
  VOI[3] = m_bottomMargin->value();
  VOI[4] = m_upperMargin->value();
  VOI[5] = m_lowerMargin->value();

  int x = m_xSeed->text().toInt();
  int y = m_ySeed->text().toInt();
  int z = m_zSeed->text().toInt();

  if ( VOI[0] > x || VOI[1] < x
    || VOI[2] > y || VOI[3] < y
    || VOI[4] > z || VOI[5] < z )
  {
    QMessageBox::warning(this,
			 tr("Seed Grow Segmentation"),
			 tr("Segmentation couldn't be modified. Seed is outside VOI"));
    return;
  }

  m_filter->setThreshold(m_threshold->value());
  m_filter->setVOI(VOI);
  m_filter->run();
  m_filter->product(0)->volume().pipelineSource()->updatePipeline();
  EspinaCore::instance()->viewManger()->currentView()->forceRender();
}