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
#include "FilterInspector.h"

// EspINA
#include "common/gui/ViewManager.h"
#include "common/widgets/RectangularRegion.h"

// Qt
#include <QDebug>
#include <QMessageBox>

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::FilterInspector::FilterInspector(Filter* filter, ViewManager* vm)
: m_viewManager(vm)
, m_region(NULL)
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
  for (int i=0; i<6; i++)
    m_voiBounds[i] = voiExtent[i] * spacing[i/2];

  m_leftMargin->setValue(m_voiBounds[0]);
  m_leftMargin->setSuffix(" nm");
  m_leftMargin->installEventFilter(this);
  connect(m_leftMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_rightMargin->setValue(m_voiBounds[1]);
  m_rightMargin->setSuffix(" nm");
  m_rightMargin->installEventFilter(this);
  connect(m_rightMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_topMargin->setValue(m_voiBounds[2]);
  m_topMargin->setSuffix(" nm");
  m_topMargin->installEventFilter(this);
  connect(m_topMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_bottomMargin->setValue(m_voiBounds[3]);
  m_bottomMargin->setSuffix(" nm");
  m_bottomMargin->installEventFilter(this);
  connect(m_bottomMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_upperMargin->setValue(m_voiBounds[4]);
  m_upperMargin->setSuffix(" nm");
  m_upperMargin->installEventFilter(this);
  connect(m_upperMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_lowerMargin->setValue(m_voiBounds[5]);
  m_lowerMargin->setSuffix(" nm");
  m_lowerMargin->installEventFilter(this);
  connect(m_lowerMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

//   connect(m_threshold, SIGNAL(valueChanged(int)),
// 	  this, SLOT(modifyFilter()));
  connect(m_modify, SIGNAL(clicked(bool)),
	  this, SLOT(modifyFilter()));
}

//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::FilterInspector::~FilterInspector()
{
  if (m_region)
  {
    m_viewManager->hideSliceSelectors(ViewManager::From|ViewManager::To);
    m_viewManager->removeWidget(m_region);
    delete m_region;
  }
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::FilterInspector::eventFilter(QObject* sender, QEvent* e)
{
  if (e->type() == QEvent::FocusIn)
  {
    m_viewManager->showSliceSelectors(ViewManager::From|ViewManager::To);
    if (!m_region)
    {
      m_region = new RectangularRegion(m_voiBounds, m_viewManager);
      connect(m_region, SIGNAL(modified(double*)),
              this, SLOT(redefineVOI(double*)));
      m_viewManager->addWidget(m_region);
      m_region->setEnabled(false);
      m_viewManager->updateViews();
    }
    connect(m_viewManager, SIGNAL(sliceSelected(Nm,PlaneType,ViewManager::SliceSelectors)),
            this, SLOT(redefineVOI(Nm,PlaneType,ViewManager::SliceSelectors)));
  }

  return QObject::eventFilter(sender, e);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::FilterInspector::redefineVOI(double* bounds)
{
  m_leftMargin->setValue(bounds[0]);
  m_rightMargin->setValue(bounds[1]);
  m_topMargin->setValue(bounds[2]);
  m_bottomMargin->setValue(bounds[3]);
  m_upperMargin->setValue(bounds[4]);
  m_lowerMargin->setValue(bounds[5]);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::FilterInspector::redefineVOI(Nm pos,
                                                          PlaneType plane,
                                                          ViewManager::SliceSelectors flags)
{
  switch (plane)
  {
    case AXIAL:
      if (flags.testFlag(ViewManager::From))
        m_upperMargin->setValue(pos);
      if (flags.testFlag(ViewManager::To))
        m_lowerMargin->setValue(pos);
      break;
    case SAGITTAL:
      if (flags.testFlag(ViewManager::From))
        m_leftMargin->setValue(pos);
      if (flags.testFlag(ViewManager::To))
        m_rightMargin->setValue(pos);
      break;
    case CORONAL:
      if (flags.testFlag(ViewManager::From))
        m_topMargin->setValue(pos);
      if (flags.testFlag(ViewManager::To))
        m_bottomMargin->setValue(pos);
      break;
  }
  updateRegionBounds();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::FilterInspector::modifyFilter()
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
  m_filter->update();
  m_viewManager->updateViews();
  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::FilterInspector::updateRegionBounds()
{
  m_voiBounds[0] = m_leftMargin->value();
  m_voiBounds[1] = m_rightMargin->value();
  m_voiBounds[2] = m_topMargin->value();
  m_voiBounds[3] = m_bottomMargin->value();
  m_voiBounds[4] = m_upperMargin->value();
  m_voiBounds[5] = m_lowerMargin->value();

  if (m_region)
    m_region->setBounds(m_voiBounds);
}
