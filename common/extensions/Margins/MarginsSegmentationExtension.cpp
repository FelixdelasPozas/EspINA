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

const QString MarginsSegmentationExtension::ID = "MarginsExtension";

const QString MarginsSegmentationExtension::LeftMargin   = "Left Margin";
const QString MarginsSegmentationExtension::TopMargin    = "Top Margin";
const QString MarginsSegmentationExtension::UpperMargin  = "Upper Margin";
const QString MarginsSegmentationExtension::RightMargin  = "Right Margin";
const QString MarginsSegmentationExtension::BottomMargin = "Bottom Margin";
const QString MarginsSegmentationExtension::LowerMargin  = "Lower Margin";

//-----------------------------------------------------------------------------
MarginsSegmentationExtension::MarginsSegmentationExtension()
{
  memset(m_distances, 0, 6*sizeof(double));
  m_availableInformations << LeftMargin;
  m_availableInformations << TopMargin;
  m_availableInformations << UpperMargin;
  m_availableInformations << RightMargin;
  m_availableInformations << BottomMargin;
  m_availableInformations << LowerMargin;
}

//-----------------------------------------------------------------------------
MarginsSegmentationExtension::~MarginsSegmentationExtension()
{
}

//-----------------------------------------------------------------------------
QString MarginsSegmentationExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void MarginsSegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;

  ModelItem::Vector channels = m_seg->relatedItems(ModelItem::IN, "Channel");
  //Q_ASSERT(!channels.isEmpty());
  if (!channels.isEmpty())
  {
    Channel *channel = dynamic_cast<Channel *>(channels.first());
    ModelItemExtension *ext = channel->extension(ID);
    Q_ASSERT(ext);
    MarginsChannelExtension *marginExt = dynamic_cast<MarginsChannelExtension *>(ext);
    marginExt->computeMarginDistance(seg);
  }
}

//-----------------------------------------------------------------------------
SegmentationRepresentation* MarginsSegmentationExtension::representation(QString rep)
{
  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
QVariant MarginsSegmentationExtension::information(QString info) const
{
  if (!m_init)
    return QString(QObject::tr("Unknown"));

  if (LeftMargin == info)
    return m_distances[0];
  if (TopMargin == info)
    return m_distances[1];
  if (UpperMargin == info)
    return m_distances[2];
  if (RightMargin == info)
    return m_distances[3];
  if (BottomMargin == info)
    return m_distances[4];
  if (LowerMargin == info)
    return m_distances[5];

  qWarning() << ID << ":"  << info << " is not provided";
  Q_ASSERT(false);
  return QVariant();
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
