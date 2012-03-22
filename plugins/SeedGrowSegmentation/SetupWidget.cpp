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
  m_threshold->setValue(m_filter->m_args.threshold());
  connect(m_threshold, SIGNAL(valueChanged(int)),
	  this, SLOT(modifyFilter()));
  connect(modify, SIGNAL(clicked(bool)),
	  this, SLOT(modifyFilter()));
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::SetupWidget::modifyFilter()
{
  m_filter->setThreshold(m_threshold->value());
  m_filter->product(0)->volume().pipelineSource()->updatePipeline();
  EspinaCore::instance()->viewManger()->currentView()->forceRender();
}