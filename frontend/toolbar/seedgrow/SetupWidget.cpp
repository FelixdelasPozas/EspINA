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
#include <common/EspinaCore.h>
#include <common/gui/EspinaView.h>
#include <common/selection/SelectionManager.h>
#include <widgets/RectangularSelection.h>

#include <QDebug>
#include <QMessageBox>

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SetupWidget::SetupWidget(Filter* filter)
{
  setupUi(this);
  m_filter = dynamic_cast<SeedGrowSegmentationFilter *>(filter);
  EspinaVolume::IndexType seed = m_filter->seed();
  m_xSeed->setText(QString("%1").arg(seed[0]));
  m_ySeed->setText(QString("%1").arg(seed[1]));
  m_zSeed->setText(QString("%1").arg(seed[2]));
  m_threshold->setMaximum(255);
  m_threshold->setValue(m_filter->m_param.lowerThreshold());
  int voiExtent[6];
  m_filter->m_param.voi(voiExtent);
  EspinaVolume::SpacingType spacing = filter->output(0)->GetSpacing();
  Nm voiBoudns[6];
  for (int i=0; i<6; i++)
    voiBoudns[i] = voiExtent[i] * spacing[i/2];

  m_leftMargin->setValue(voiBoudns[0]);
  m_leftMargin->setSuffix(" nm");
  m_leftMargin->installEventFilter(this);

  m_rightMargin->setValue(voiBoudns[1]);
  m_rightMargin->setSuffix(" nm");
  m_rightMargin->installEventFilter(this);

  m_topMargin->setValue(voiBoudns[2]);
  m_topMargin->setSuffix(" nm");
  m_topMargin->installEventFilter(this);

  m_bottomMargin->setValue(voiBoudns[3]);
  m_bottomMargin->setSuffix(" nm");
  m_bottomMargin->installEventFilter(this);

  m_upperMargin->setValue(voiBoudns[4]);
  m_upperMargin->setSuffix(" nm");
  m_upperMargin->installEventFilter(this);

  m_lowerMargin->setValue(voiBoudns[5]);
  m_lowerMargin->setSuffix(" nm");
  m_lowerMargin->installEventFilter(this);

//   connect(m_threshold, SIGNAL(valueChanged(int)),
// 	  this, SLOT(modifyFilter()));
  connect(m_modify, SIGNAL(clicked(bool)),
	  this, SLOT(modifyFilter()));


//   m_region = new RectangularRegion();
//   EspinaCore::instance()->viewManger()->currentView()->addWidget(m_region);
//   m_region->setBounds(voiBoudns);
}

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SetupWidget::~SetupWidget()
{
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
  if (!SelectionManager::instance()->voi())
    view->setSliceSelectors(SliceView::NoSelector);

//   EspinaCore::instance()->viewManger()->currentView()->removeWidget(m_region);
//   delete m_region;
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::SetupWidget::eventFilter(QObject* sender, QEvent* e)
{
  if (e->type() == QEvent::FocusIn)
  {
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    view->setSliceSelectors(SliceView::From| SliceView::To);
    connect(view, SIGNAL(selectedFromSlice(double,PlaneType)),
	    this, SLOT(redefineFromVOI(double,PlaneType)));
    connect(view, SIGNAL(selectedToSlice(double,PlaneType)),
	    this, SLOT(redefineToVOI(double,PlaneType)));
  }

  return QObject::eventFilter(sender, e);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::redefineFromVOI(double value, PlaneType plane)
{
  switch (plane)
  {
    case AXIAL:
      m_upperMargin->setValue(value);
      break;
    case SAGITTAL:
      m_leftMargin->setValue(value);
      break;
    case CORONAL:
      m_topMargin->setValue(value);
      break;
  }
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::redefineToVOI(double value, PlaneType plane)
{
  switch (plane)
  {
    case AXIAL:
      m_lowerMargin->setValue(value);
      break;
    case SAGITTAL:
      m_rightMargin->setValue(value);
      break;
    case CORONAL:
      m_bottomMargin->setValue(value);
      break;
  }
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::modifyFilter()
{
  EspinaVolume::SpacingType spacing = m_filter->output(0)->GetSpacing();
  int VOI[6];
  VOI[0] = m_leftMargin->value()/spacing[0];
  VOI[1] = m_rightMargin->value()/spacing[0];
  VOI[2] = m_topMargin->value()/spacing[1];
  VOI[3] = m_bottomMargin->value()/spacing[1];
  VOI[4] = m_upperMargin->value()/spacing[2];
  VOI[5] = m_lowerMargin->value()/spacing[2];

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

  m_filter->setLowerThreshold(m_threshold->value());
  m_filter->setUpperThreshold(m_threshold->value());
  m_filter->setVOI(VOI);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_filter->run();
  EspinaCore::instance()->viewManger()->currentView()->forceRender();
  QApplication::restoreOverrideCursor();
}