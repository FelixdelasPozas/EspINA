/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña <jorge.pena.pastor@gmail.com>

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
#include "ChannelInspector.h"

#include <Core/Model/Channel.h>
#include <Filters/ChannelReader.h>
#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>

// Qt
#include <QDebug>

//------------------------------------------------------------------------
ChannelInspector::ChannelInspector(Channel *channel,
                                   ViewManager *vm,
                                   QWidget *parent,
                                   Qt::WindowFlags f)
: QDialog(parent, f)
, m_channel(channel)
, m_viewManager(vm)
{
  setupUi(this);

  connect(m_unit, SIGNAL(currentIndexChanged(int)),
          this, SLOT(unitChanged(int)));
  connect(pushButton, SIGNAL(clicked(bool)),
          this, SLOT(updateSpacing()));

  double spacing[3];
  channel->volume()->spacing(spacing);

  m_xSize->setValue(spacing[0]);
  m_ySize->setValue(spacing[1]);
  m_zSize->setValue(spacing[2]);
}

//------------------------------------------------------------------------
void ChannelInspector::unitChanged(int unitIndex)
{
  m_xSize->setSuffix(m_unit->currentText());
  m_ySize->setSuffix(m_unit->currentText());
  m_zSize->setSuffix(m_unit->currentText());
}

//------------------------------------------------------------------------
void ChannelInspector::updateSpacing()
{
  itkVolumeType::SpacingType spacing;
  spacing[0] = m_xSize->value()*pow(1000,m_unit->currentIndex());
  spacing[1] = m_ySize->value()*pow(1000,m_unit->currentIndex());
  spacing[2] = m_zSize->value()*pow(1000,m_unit->currentIndex());
  ChannelReader *reader = dynamic_cast<ChannelReader *>(m_channel->filter());
  Q_ASSERT(reader);
  reader->setSpacing(spacing);
  m_channel->notifyModification();

  foreach(ModelItem *item, m_channel->relatedItems(ModelItem::OUT, Channel::LINK))
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      double oldSpacing[3];
      seg->volume()->spacing(oldSpacing);
      seg->volume()->toITK()->SetSpacing(spacing);
      itkVolumeType::PointType origin = seg->volume()->toITK()->GetOrigin();
      for (int i=0; i < 3; i++)
        origin[i] = origin[i]/oldSpacing[i]*spacing[i];
      seg->volume()->toITK()->SetOrigin(origin);
      seg->notifyModification();
    }
  }
  m_viewManager->resetViewCameras();
  setResult(QDialog::Accepted);
  accept();
}