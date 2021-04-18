/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ChannelAdapter.h"
#include <Core/Analysis/Channel.h>
#include <Extensions/Issues/ItemIssues.h>
#include <GUI/Utils/MiscUtils.h>

// Qt
#include <QPixmap>
#include <QColor>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//------------------------------------------------------------------------
ChannelAdapter::ChannelAdapter(ChannelSPtr channel)
: ViewItemAdapter{channel}
, m_channel      {channel}
{
}

//------------------------------------------------------------------------
ChannelAdapter::~ChannelAdapter()
{
}

//------------------------------------------------------------------------
QVariant ChannelAdapter::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return m_channel->name();
    case Qt::CheckStateRole:
      return isVisible()?Qt::Checked:Qt::Unchecked;
    case Qt::DecorationRole:
    {
      QPixmap icon(0, 16);
      icon.fill(Qt::transparent);
      bool usesIcon = false;

      if(hue() != -1)
      {
        auto color = QColor::fromHsv(hue()*359,255,255,255);
        icon = QPixmap(16,16);
        icon.fill(color);
        usesIcon = true;
      }

      // We should let the extensions decorate
      if (readOnlyExtensions()->hasExtension(StackIssues::TYPE))
      {
        auto extension = readOnlyExtensions()->get<StackIssues>();

        if (extension->information(StackIssues::CRITICAL).toInt() > 0)
        {
          icon = GUI::Utils::appendImage(icon, StackIssues::severityIcon(Issue::Severity::CRITICAL, true), true);
        }
        else if (extension->information(StackIssues::WARNING).toInt() > 0)
        {
          icon = GUI::Utils::appendImage(icon, StackIssues::severityIcon(Issue::Severity::WARNING, true), true);
        }

        usesIcon = true;
      }

      if(usesIcon)
      {
        return icon;
      }
      return QVariant();
    }
    case Qt::ToolTipRole:
    {
      QString tooltip;
      tooltip = tooltip.append("<center><b>%1</b></center>").arg(data().toString());

      const QString WS  = "&nbsp;"; // White space
      const QString TAB = WS+WS+WS;
      QString boundsInfo;

      if (output()->isValid())
      {
        Bounds bounds = output()->bounds();
        boundsInfo = tr("<b>Bounds:</b><br>");
        boundsInfo = boundsInfo.append(TAB+"X: [%1 nm, %2 nm)<br>").arg(bounds[0]).arg(bounds[1]);
        boundsInfo = boundsInfo.append(TAB+"Y: [%1 nm, %2 nm)<br>").arg(bounds[2]).arg(bounds[3]);
        boundsInfo = boundsInfo.append(TAB+"Z: [%1 nm, %2 nm)<br>").arg(bounds[4]).arg(bounds[5]);
      }
      tooltip = tooltip.append(boundsInfo);

      QString extensionsInfo = tr("<b>Extensions:</b>");
      QString extensionsData;
      for(auto extension : m_channel->readOnlyExtensions())
      {
        QString extToolTip = extension->toolTipText();
        if (!extToolTip.isEmpty())
        {
          extensionsData += "<br>" + extToolTip;
        }
      }

      if(extensionsData.isEmpty())
      {
        extensionsData += tr("<br>None present.");
      }

      tooltip += extensionsInfo + extensionsData;

      return tooltip;
    }
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool ChannelAdapter::setData(const QVariant& value, int role)
{
  switch (role)
  {
    case Qt::EditRole:
      m_channel->setName(value.toString());
      return true;
    case Qt::CheckStateRole:
      setVisible(value.toBool());
      return true;
    default:
      return false;
  }
}

//------------------------------------------------------------------------
InputSPtr ChannelAdapter::asInput() const
{
  return m_channel->asInput();
}

//------------------------------------------------------------------------
void ChannelAdapter::changeOutputImplementation(InputSPtr input)
{
  m_channel->changeOutput(input);
}

//------------------------------------------------------------------------
void ChannelAdapter::setPosition(const NmVector3& point)
{
  m_channel->setPosition(point);
}

//------------------------------------------------------------------------
NmVector3 ChannelAdapter::position() const
{
  return m_channel->position();
}

//------------------------------------------------------------------------
void ChannelAdapter::setHue(double hue)
{
  m_channel->setHue(hue);
}

//------------------------------------------------------------------------
double ChannelAdapter::hue() const
{
  return m_channel->hue();
}

//------------------------------------------------------------------------
void ChannelAdapter::setOpacity(double opacity)
{
  m_channel->setOpacity(opacity);
}

//------------------------------------------------------------------------
double ChannelAdapter::opacity() const
{
  return m_channel->opacity();
}

//------------------------------------------------------------------------
void ChannelAdapter::setSaturation(double saturation)
{
  m_channel->setSaturation(saturation);
}

//------------------------------------------------------------------------
double ChannelAdapter::saturation() const
{
  return m_channel->saturation();
}

//------------------------------------------------------------------------
void ChannelAdapter::setContrast(double contrast)
{
  m_channel->setContrast(contrast);
}

//------------------------------------------------------------------------
double ChannelAdapter::contrast() const
{
  return m_channel->contrast();
}

//------------------------------------------------------------------------
double ChannelAdapter::brightness() const
{
  return m_channel->brightness();
}

//------------------------------------------------------------------------
void ChannelAdapter::setBrightness(double brightness)
{
  m_channel->setBrightness(brightness);
}

//------------------------------------------------------------------------
void ChannelAdapter::setMetadata(const QString& metadata)
{
  m_channel->setMetadata(metadata);
}

//------------------------------------------------------------------------
QString ChannelAdapter::metadata() const
{
  return m_channel->metadata();
}

//------------------------------------------------------------------------
Bounds ChannelAdapter::bounds() const
{
  return m_channel->bounds();
}

//------------------------------------------------------------------------
ChannelAdapter::ReadLockExtensions ChannelAdapter::readOnlyExtensions() const
{
  return m_channel->readOnlyExtensions();
}

//------------------------------------------------------------------------
ChannelAdapter::WriteLockExtensions ChannelAdapter::extensions()
{
  return m_channel->extensions();
}

//------------------------------------------------------------------------
bool ESPINA::operator==(ChannelAdapterSPtr lhs, ChannelSPtr rhs)
{
  return lhs->m_channel == rhs;
}

//------------------------------------------------------------------------
bool ESPINA::operator==(ChannelSPtr lhs, ChannelAdapterSPtr rhs)
{
  return lhs == rhs->m_channel;
}

//------------------------------------------------------------------------
bool ESPINA::operator!=(ChannelAdapterSPtr lhs, ChannelSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
bool ESPINA::operator!=(ChannelSPtr lhs, ChannelAdapterSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
ChannelAdapterPtr ESPINA::channelPtr(ItemAdapterPtr item)
{
  return dynamic_cast<ChannelAdapterPtr>(item);
}

//------------------------------------------------------------------------
ConstChannelAdapterPtr ESPINA::channelPtr(ConstItemAdapterPtr item)
{
  return dynamic_cast<ConstChannelAdapterPtr>(item);
}


//------------------------------------------------------------------------
bool ESPINA::isChannel(const ItemAdapterPtr item)
{
  return item && (ItemAdapter::Type::CHANNEL == item->type());
}

//------------------------------------------------------------------------
ViewItemAdapterSList ESPINA::toViewItemSList(const ChannelAdapterSPtr channel)
{
  ViewItemAdapterSList result;

  result << channel;

  return result;
}

//------------------------------------------------------------------------
ViewItemAdapterSList ESPINA::toViewItemSList(const ChannelAdapterSList channels)
{
  ViewItemAdapterSList result;

  for (auto channel : channels)
  {
    result << channel;
  }

  return result;
}
