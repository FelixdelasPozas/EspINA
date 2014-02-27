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
#include "Channel.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>

using namespace EspINA;

const double MIN_BRIGHTNESS = -1.0;
const double MAX_BRIGHTNESS =  1.0;

const double MIN_CONTRAST = 0.0;
const double MAX_CONTRAST = 2.0;

const double MIN_HUE =  0.0;
const double MAX_HUE =  1.0;
const double NO_HUE  = -1.0;

const double MIN_OPACITY  =  0.0;
const double MAX_OPACITY  =  1.0;
const double AUTO_OPACITY = -1.0;

const double MIN_SATURATION = 0.0;
const double MAX_SATURATION = 1.0;

const QString Channel::STAIN_LINK  = "Stain";

//------------------------------------------------------------------------
Channel::Channel(InputSPtr input)
: ViewItem(input)
, m_brightness{0.0}
, m_contrast{1.0}
, m_hue{NO_HUE}
, m_opacity{AUTO_OPACITY}
, m_saturation{0.0}
{
}

//------------------------------------------------------------------------
Channel::~Channel()
{
  //qDebug() << "Destroying Channel" << name();
}

//------------------------------------------------------------------------
void Channel::restoreState(const State& state)
{
  for(auto element : state.split(";"))
  {
    auto tokens = element.split("=");
    if ("Brightness" == tokens[0])
    {
      setBrightness(tokens[1].toDouble());
    } else if ("Contrast" == tokens[0])
    {
      setContrast(tokens[1].toDouble());
    } else if ("Hue" == tokens[0])
    {
      setHue(tokens[1].toDouble());
    } else if ("Saturation" == tokens[0])
    {
      setSaturation(tokens[1].toDouble());
    } else if ("Opacity" == tokens[0])
    {
      setOpacity(tokens[1].toDouble());
    } else if ("Spacing" == tokens[0])
    {
      NmVector3 spacing;
      auto values = tokens[1].split(",");
      for(int i = 0; i < 3; ++i)
      {
        spacing[i] = values[i].toDouble();
      }
      output()->setSpacing(spacing);
    }
  }
}

//------------------------------------------------------------------------
State Channel::state() const
{
  State state;

  auto  spacing = output()->spacing();
  state += QString("Spacing=%1,%2,%3;").arg(spacing[0])
                                       .arg(spacing[1])
                                       .arg(spacing[2]);

  state += QString("Brightness=%1;").arg(m_brightness);

  state += QString("Contrast=%1;").arg(m_contrast);

  state += QString("Hue=%1;").arg(m_hue);

  state += QString("Saturation=%1;").arg(m_saturation);

  state += QString("Opacity=%1;").arg(m_opacity);

  return state;
}

//------------------------------------------------------------------------
void Channel::addExtension(ChannelExtensionSPtr extension)
{
  if (m_extensions.contains(extension->type()))
    throw (ChannelExtension::Existing_Extension());

  extension->setExtendedItem(this);

  m_extensions.insert(extension->type(), extension);
}

//------------------------------------------------------------------------
void Channel::deleteExtension(ChannelExtensionSPtr extension)
{
  deleteExtension(extension->type());
}

//------------------------------------------------------------------------
void Channel::deleteExtension(const QString& type)
{
  if (!m_extensions.contains(type))
    throw (ChannelExtension::Extension_Not_Found());

  m_extensions.remove(type);

  Q_ASSERT(!m_extensions.contains(type));
}


//------------------------------------------------------------------------
ChannelExtensionSPtr Channel::extension(const ChannelExtension::Type& type)
{
  if (!m_extensions.contains(type))
  {
    throw ChannelExtension::Extension_Not_Found();
  }

  return m_extensions.value(type, ChannelExtensionSPtr());
}

//------------------------------------------------------------------------
bool Channel::hasExtension(const ChannelExtension::Type& type) const
{
  foreach(ChannelExtensionSPtr extension, m_extensions) 
  {
    if (extension->type() == type) return true;
  }

  return false;
}

//------------------------------------------------------------------------
Snapshot Channel::snapshot() const
{
  Snapshot snapshot;

  if (!m_extensions.isEmpty())
  {
    QByteArray xml;

    QXmlStreamWriter stream(&xml);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Channel");
    stream.writeAttribute("Name", name());
    for(auto extension : m_extensions)
    {
      stream.writeStartElement("Extension");
      stream.writeAttribute("Type", extension->type());
      stream.writeAttribute("InvalidateOnChange", QString("%1").arg(extension->invalidateOnChange()));
      for(auto tag : extension->readyInformation())
      {
        stream.writeStartElement("Info");
        stream.writeAttribute("Name", tag);
        stream.writeCharacters(extension->information(tag).toString());
        stream.writeEndElement();
      }

      auto state = extension->state();
      if (!state.isEmpty())
      {
        stream.writeStartElement("State");
        stream.writeCharacters(state);
        stream.writeEndElement();
      }
      stream.writeEndElement();

      for(auto data: extension->snapshot())
      {
        QString file = extensionDataPath(extension, data.first);
        snapshot << SnapshotData(file, data.second);
      }
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    QString file = extensionsPath() + QString("%1.xml").arg(uuid());
    snapshot << SnapshotData(file, xml);
  }

  return snapshot;
}

//------------------------------------------------------------------------
void Channel::unload()
{

}


//------------------------------------------------------------------------
NmVector3 Channel::position() const
{
  return NmVector3{0,0,0};
}

//------------------------------------------------------------------------
void Channel::setPosition(const NmVector3& point)
{

}

//------------------------------------------------------------------------
void Channel::setHue(double hue)
{
  if (hue < MIN_HUE && hue != NO_HUE)
  {
    m_hue = MIN_HUE;
  } else if (hue > MAX_HUE) 
  {
    m_hue = MAX_HUE;
  } else 
  {
    m_hue = hue;
  }
}

//------------------------------------------------------------------------
void Channel::setOpacity(double opacity)
{
  if (opacity < MIN_OPACITY && opacity != AUTO_OPACITY)
  {
    m_opacity = MIN_OPACITY;
  } else if (opacity > MAX_OPACITY) 
  {
    m_opacity = MAX_OPACITY;
  } else 
  {
    m_opacity = opacity;    
  }
}

//------------------------------------------------------------------------
void Channel::setSaturation(double saturation)
{
  if (saturation < MIN_SATURATION)
  {
    m_saturation = MIN_SATURATION;
  } else if (saturation > MAX_SATURATION) 
  {
    m_saturation = MAX_SATURATION;
  } else 
  {
    m_saturation = saturation;    
  }
}

//------------------------------------------------------------------------
void Channel::setContrast(double contrast)
{
  if (contrast < MIN_CONTRAST)
  {
    m_contrast = MIN_CONTRAST;
  } else if (contrast > MAX_CONTRAST) 
  {
    m_contrast = MAX_CONTRAST;
  } else 
  {
    m_contrast = contrast;    
  }
}

//------------------------------------------------------------------------
void Channel::setBrightness(double brightness)
{
  if (brightness < MIN_BRIGHTNESS)
  {
    m_brightness = MIN_BRIGHTNESS;
  } else if (brightness > MAX_BRIGHTNESS) 
  {
    m_brightness = MAX_BRIGHTNESS;
  } else 
  {
    m_brightness = brightness;    
  }
}