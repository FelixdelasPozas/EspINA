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


#include "MarginsSegmentationExtension.h"

#include "MarginsChannelExtension.h"

#include <QDebug>
#include <common/model/Segmentation.h>
#include <common/model/Channel.h>
#include <QApplication>

const ModelItemExtension::ExtId MarginsSegmentationExtension::ID = "MarginsExtension";

const ModelItemExtension::InfoTag MarginsSegmentationExtension::LEFT_MARGIN   = "Left Margin";
const ModelItemExtension::InfoTag MarginsSegmentationExtension::TOP_MARGIN    = "Top Margin";
const ModelItemExtension::InfoTag MarginsSegmentationExtension::UPPER_MARGIN  = "Upper Margin";
const ModelItemExtension::InfoTag MarginsSegmentationExtension::RIGHT_MARGIN  = "Right Margin";
const ModelItemExtension::InfoTag MarginsSegmentationExtension::BOTTOM_MARGIN = "Bottom Margin";
const ModelItemExtension::InfoTag MarginsSegmentationExtension::LOWER_MARGIN  = "Lower Margin";

//-----------------------------------------------------------------------------
MarginsSegmentationExtension::MarginsSegmentationExtension()
{
  memset(m_distances, 0, 6*sizeof(double));
  m_availableInformations << LEFT_MARGIN;
  m_availableInformations << RIGHT_MARGIN;
  m_availableInformations << TOP_MARGIN;
  m_availableInformations << BOTTOM_MARGIN;
  m_availableInformations << UPPER_MARGIN;
  m_availableInformations << LOWER_MARGIN;
}

//-----------------------------------------------------------------------------
MarginsSegmentationExtension::~MarginsSegmentationExtension()
{
}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtId MarginsSegmentationExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void MarginsSegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
}

//-----------------------------------------------------------------------------
QVariant MarginsSegmentationExtension::information(ModelItemExtension::InfoTag tag) const
{
  if (!m_init)
  {
    //qDebug() << m_seg->data().toString() << "Updating Margins";
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ModelItem::Vector channels = m_seg->relatedItems(ModelItem::IN, "Channel");
    //Q_ASSERT(!channels.isEmpty());
    if (!channels.isEmpty())
    {
      Channel *channel = dynamic_cast<Channel *>(channels.first());
      ModelItemExtension *ext = channel->extension(ID);
      Q_ASSERT(ext);
      MarginsChannelExtension *marginExt = dynamic_cast<MarginsChannelExtension *>(ext);
      marginExt->computeMarginDistance(m_seg);
      QApplication::restoreOverrideCursor();
      m_init = true;
    }
  }

  if (LEFT_MARGIN == tag)
    return m_distances[0];
  if (RIGHT_MARGIN == tag)
    return m_distances[1];
  if (TOP_MARGIN == tag)
    return m_distances[2];
  if (BOTTOM_MARGIN == tag)
    return m_distances[3];
  if (UPPER_MARGIN == tag)
    return m_distances[4];
  if (LOWER_MARGIN == tag)
    return m_distances[5];

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
SegmentationRepresentation* MarginsSegmentationExtension::representation(QString rep)
{
  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
SegmentationExtension* MarginsSegmentationExtension::clone()
{
  return new MarginsSegmentationExtension();
}

//-----------------------------------------------------------------------------
void MarginsSegmentationExtension::setMargins(double distances[6])
{
  memcpy(m_distances, distances, 6*sizeof(double));
  m_init = true;
}
