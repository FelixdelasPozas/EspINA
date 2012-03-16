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


#include "MarginsChannelExtension.h"

#include "common/processing/pqFilter.h"
#include "common/model/Channel.h"
#include "common/cache/CachedObjectBuilder.h"

#include <pqPipelineSource.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>

const QString MarginsChannelExtension::ID = "MarginsExtension";

const QString MarginsChannelExtension::LeftMargin   = "Left Margin";
const QString MarginsChannelExtension::TopMargin    = "Top Margin";
const QString MarginsChannelExtension::UpperMargin  = "Upper Margin";
const QString MarginsChannelExtension::RightMargin  = "Right Margin";
const QString MarginsChannelExtension::BottomMargin = "Bottom Margin";
const QString MarginsChannelExtension::LowerMargin  = "Lower Margin";

//-----------------------------------------------------------------------------
MarginsChannelExtension::MarginsChannelExtension()
: m_borderDetector(NULL)
{
//   m_availableInformations << LeftMargin;
//   m_availableInformations << TopMargin;
//   m_availableInformations << UpperMargin;
//   m_availableInformations << RightMargin;
//   m_availableInformations << BottomMargin;
//   m_availableInformations << LowerMargin;
}

//-----------------------------------------------------------------------------
MarginsChannelExtension::~MarginsChannelExtension()
{

}

//-----------------------------------------------------------------------------
QString MarginsChannelExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void MarginsChannelExtension::initialize(Channel *channel)
{
  m_channel = channel;

  //TODO: If has border argument, recover it
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqFilter::Arguments marginArgs;
  marginArgs << pqFilter::Argument("Input", pqFilter::Argument::INPUT, m_channel->volume().id());
  m_borderDetector = cob->createFilter("filters","ChannelBorderDetector", marginArgs);
  Q_ASSERT(m_borderDetector);
  m_borderDetector->pipelineSource()->updatePipeline();
  Q_ASSERT(m_borderDetector->getNumberOfData() == 1);
}

//-----------------------------------------------------------------------------
QVariant MarginsChannelExtension::information(QString info) const
{
//   if (LeftMargin == info)
//     return "0";

  qWarning() << ID << ":"  << info << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
ChannelExtension* MarginsChannelExtension::clone()
{
  return new MarginsChannelExtension();
}
