/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "ChannelAdapter.h"

#include <Core/Analysis/Channel.h>

using namespace EspINA;

//------------------------------------------------------------------------
ChannelAdapter::ChannelAdapter(FilterAdapterSPtr filter, ChannelSPtr channel)
: ViewItemAdapter(filter, channel)
, m_channel(channel)
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
void ChannelAdapter::changeOutput(InputSPtr input)
{
  m_channel->changeOutput(input);
  m_representations.clear();
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
  return m_channel->setHue(hue);
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
void ChannelAdapter::addExtension(ChannelExtensionSPtr extension)
{
  m_channel->addExtension(extension);
}

//------------------------------------------------------------------------
Bounds ChannelAdapter::bounds() const
{
  return m_channel->bounds();
}

//------------------------------------------------------------------------
void ChannelAdapter::deleteExtension(ChannelExtensionSPtr extension)
{
  m_channel->deleteExtension(extension);
}

//------------------------------------------------------------------------
bool ChannelAdapter::hasExtension(const ChannelExtension::Type& type) const
{
  return m_channel->hasExtension(type);
}

//------------------------------------------------------------------------
ChannelExtensionSPtr ChannelAdapter::extension(const ChannelExtension::Type& type)
{
  return m_channel->extension(type);
}

//------------------------------------------------------------------------
bool EspINA::operator==(ChannelAdapterSPtr lhs, ChannelSPtr rhs)
{
  return lhs->m_channel == rhs;
}

//------------------------------------------------------------------------
bool EspINA::operator==(ChannelSPtr lhs, ChannelAdapterSPtr rhs)
{
  return lhs == rhs->m_channel;
}


//------------------------------------------------------------------------
bool EspINA::operator!=(ChannelAdapterSPtr lhs, ChannelSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
bool EspINA::operator!=(ChannelSPtr lhs, ChannelAdapterSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
ChannelAdapterPtr EspINA::channelPtr(ItemAdapterPtr item)
{
  return static_cast<ChannelAdapterPtr>(item);
}
